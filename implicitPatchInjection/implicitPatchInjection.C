/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2015-2021 OpenCFD Ltd.
    Copyright (C) 2026 Corvid Technologies
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "implicitPatchInjection.H"
#include "distributionModel.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

template<class CloudType>
const Foam::Enum<typename Foam::implicitPatchInjection<CloudType>::velocityType>
    Foam::implicitPatchInjection<CloudType>::velocityTypeNames_({
        {vtFixedValue, "fixedValue"},
        {vtPatchValue, "patchValue"},
        {vtZeroGradient, "zeroGradient"},
    });

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::implicitPatchInjection<CloudType>::implicitPatchInjection(
    const dictionary& dict,
    CloudType& owner,
    const word& modelName)
: InjectionModel<CloudType>(dict, owner, modelName, typeName),
  patchInjectionBase(owner.mesh(), this->coeffDict().getWord("patch")),
  duration_(this->coeffDict().getScalar("duration")),
  parcelsPerSecond_(
      this->coeffDict().getScalar("parcelsPerSecond")),
  velocityType_(
      velocityTypeNames_.getOrDefault(
          "velocityType",
          this->coeffDict(),
          vtFixedValue)),
  U0_(
      velocityType_ == vtFixedValue
          ? this->coeffDict().template get<vector>("U0")
          : Zero),
  flowRateProfile_(
      Function1<scalar>::New(
          "flowRateProfile",
          this->coeffDict(),
          &owner.mesh())),
  sizeDistribution_(
      distributionModel::New(
          this->coeffDict().subDict("sizeDistribution"),
          owner.rndGen())),
  currentParceli_(-1),
  currentFacei_(-1)
{
    // Convert from user time to reduce the number of time conversion calls
    const Time& time = owner.db().time();
    duration_ = time.userTimeToTime(duration_);
    flowRateProfile_->userTimeToTime(time);

    patchInjectionBase::updateMesh(owner.mesh());

    // Set total volume/mass to infinity to bypass the InjectionModel base class
    // kill-switch (total mass / volume accumulation). The injection will now be
    // strictly governed by the physical runTime and duration_.
    this->volumeTotal_ = Foam::GREAT;
    this->massTotal_ = Foam::GREAT;
}


template<class CloudType>
Foam::implicitPatchInjection<CloudType>::implicitPatchInjection(
    const implicitPatchInjection<CloudType>& im)
: InjectionModel<CloudType>(im),
  patchInjectionBase(im),
  duration_(im.duration_),
  parcelsPerSecond_(im.parcelsPerSecond_),
  velocityType_(im.velocityType_),
  U0_(im.U0_),
  flowRateProfile_(im.flowRateProfile_.clone()),
  sizeDistribution_(im.sizeDistribution_.clone()),
  currentParceli_(im.currentParceli_),
  currentFacei_(im.currentFacei_)
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void Foam::implicitPatchInjection<CloudType>::updateMesh()
{
    patchInjectionBase::updateMesh(this->owner().mesh());
}


template<class CloudType>
Foam::scalar Foam::implicitPatchInjection<CloudType>::timeEnd() const
{
    return this->SOI_ + duration_;
}

// override base class prepareForNextTimeStep
template<class CloudType>
bool Foam::implicitPatchInjection<CloudType>::prepareForNextTimeStep(
    const scalar time,
    label& newParcels,
    scalar& newVolumeFraction)
{
    // Calculate the exact mathematical start of the current fluid timestep
    scalar dt = this->owner().db().time().deltaTValue();
    scalar meshTimeStart = time - dt;

    // define relative tolerance to help us find the index for the time-window
    // start timestep
    scalar tolerance = 1e-6 * dt;

    // search the history buffer to see if we are rewinding to previously
    // visited timestep
    int foundIndex = -1;
    for (int i = histTime_.size() - 1; i >= 0; --i)
    {
        // use the relative tolerance to match to the timestep
        if (mag(histTime_[i] - meshTimeStart) <= tolerance)
        {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1)
    {
        // rewind detected, we have been at this timestep before
        this->timeStep0_ = meshTimeStart;
        this->time0_ = meshTimeStart;

        // restore exact states from this moment in history
        this->delayedVolume_ = histDelayedVolume_[foundIndex];
        this->massInjected_ = histMassInjected_[foundIndex];
        this->parcelsAddedTotal_ = histParcelsAdded_[foundIndex];
        this->nInjections_ = histNInjections_[foundIndex];

        // clear the table's stateful cache
        flowRateProfile_ = Function1<scalar>::New(
            "flowRateProfile",
            this->coeffDict(),
            &this->owner().mesh());

        // restore the RNG state
        this->owner().rndGen() = histRnd_[foundIndex];

        // erase all history AFTER this point, as we are overwritin the
        // timeline
        histTime_.resize(foundIndex + 1);
        histDelayedVolume_.resize(foundIndex + 1);
        histMassInjected_.resize(foundIndex + 1);
        histParcelsAdded_.resize(foundIndex + 1);
        histNInjections_.resize(foundIndex + 1);
        histRnd_.resize(foundIndex + 1);
    }
    else
    {
        // forward march detected; a new timestep we haven't seen yet
        // save the current states BEFORE the base class changes them
        histTime_.push_back(meshTimeStart);
        histDelayedVolume_.push_back(this->delayedVolume_);
        histMassInjected_.push_back(this->massInjected_);
        histParcelsAdded_.push_back(this->parcelsAddedTotal_);
        histNInjections_.push_back(this->nInjections_);
        histRnd_.push_back(this->owner().rndGen());
    }

    // pass control back to native base class to run its standard math
    return InjectionModel<CloudType>::prepareForNextTimeStep(
        time,
        newParcels,
        newVolumeFraction);
}


template<class CloudType>
Foam::label Foam::implicitPatchInjection<CloudType>::parcelsToInject(
    const scalar time0,
    const scalar time1)
{
    // use -SMALL to account for floating point truncation during time rewinds
    if ((time0 > -SMALL) && (time0 < duration_))
    {
        // force floating point noise back to perfect zero for calculation
        scalar t0 = max(0.0, time0);
        scalar nParcels = (time1 - t0) * parcelsPerSecond_;
        Random& rnd = this->owner().rndGen();
        scalar rndPos = rnd.globalPosition(scalar(0), scalar(1));
        label nParcelsToInject = floor(nParcels);

        // Inject an additional parcel with a probability based on the
        // remainder after the floor function
        if (
            nParcelsToInject > 0
            && (nParcels - scalar(nParcelsToInject) > rndPos))
        {
            ++nParcelsToInject;
        }

        return nParcelsToInject;
    }

    return 0;
}


template<class CloudType>
Foam::scalar Foam::implicitPatchInjection<CloudType>::volumeToInject(
    const scalar time0,
    const scalar time1)
{
    // use -SMALL to account for floating point truncation during time rewinds
    if ((time0 > -SMALL) && (time0 < duration_))
    {
        // Safely use OpenFOAM's native, mathematically exact integral
        return flowRateProfile_->integrate(time0, time1);
    }

    return 0.0;
}


template<class CloudType>
void Foam::implicitPatchInjection<CloudType>::setPositionAndCell(
    const label parcelI,
    const label nParcels,
    const scalar time,
    vector& position,
    label& cellOwner,
    label& tetFacei,
    label& tetPti)
{
    currentParceli_ = parcelI;

    currentFacei_ = patchInjectionBase::setPositionAndCell(
        this->owner().mesh(),
        this->owner().rndGen(),
        position,
        cellOwner,
        tetFacei,
        tetPti);
}


template<class CloudType>
void Foam::implicitPatchInjection<CloudType>::setProperties(
    const label parcelI,
    const label nParcels,
    const scalar time,
    typename CloudType::parcelType& parcel)
{
    // Set particle velocity
    switch (velocityType_)
    {
    case vtFixedValue:
    {
        parcel.U() = U0_;
        break;
    }
    case vtPatchValue:
    {
        if (parcelI != currentParceli_)
        {
            WarningInFunction
                << "Synchronisation problem: "
                << "attempting to set injected parcel " << parcelI
                << " properties using cached parcel " << currentParceli_
                << " properties" << endl;
        }

        const label patchFacei = currentFacei_;

        if (patchFacei < 0)
        {
            FatalErrorInFunction
                << "Unable to set parcel velocity using patch value "
                << "due to missing face index: patchFacei=" << patchFacei
                << abort(FatalError);
        }

        const volVectorField& U = this->owner().U();
        const label patchi = this->patchId_;
        parcel.U() = U.boundaryField()[patchi][patchFacei];
        break;
    }
    case vtZeroGradient:
    {
        const label celli = parcel.cell();

        if (celli < 0)
        {
            FatalErrorInFunction
                << "Unable to set parcel velocity using zeroGradient "
                << "due to missing cell index"
                << abort(FatalError);
        }

        const volVectorField& U = this->owner().U();
        parcel.U() = U[celli];
        break;
    }
    default:
    {
        FatalErrorInFunction
            << "Unhandled velocityType "
            << velocityTypeNames_[velocityType_]
            << ". Available options are:"
            << velocityTypeNames_.sortedToc()
            << abort(FatalError);
    }
    }

    // Set particle diameter
    parcel.d() = sizeDistribution_->sample();
}


template<class CloudType>
bool Foam::implicitPatchInjection<CloudType>::fullyDescribed() const
{
    return false;
}


template<class CloudType>
bool Foam::implicitPatchInjection<CloudType>::validInjection(const label)
{
    return true;
}


// ************************************************************************* //
