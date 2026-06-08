/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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

#include "implicitConeNozzleInjection.H"
#include "Function1.H"
#include "unitConversion.H"
#include "distributionModel.H"
#include "axisAngleRotation.H"

using namespace Foam::constant;


template<class CloudType>
const Foam::Enum<
    typename Foam::implicitConeNozzleInjection<CloudType>::injectionMethod>
    Foam::implicitConeNozzleInjection<CloudType>::injectionMethodNames({
        {injectionMethod::imPoint, "point"},
        {injectionMethod::imDisc, "disc"},
        {injectionMethod::imDiscSegments, "discSegments"},
    });


template<class CloudType>
const Foam::Enum<
    typename Foam::implicitConeNozzleInjection<CloudType>::flowType>
    Foam::implicitConeNozzleInjection<CloudType>::flowTypeNames({
        {flowType::ftConstantVelocity, "constantVelocity"},
        {flowType::ftPressureDrivenVelocity, "pressureDrivenVelocity"},
        {flowType::ftFlowRateAndDischarge, "flowRateAndDischarge"},
    });


// * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void Foam::implicitConeNozzleInjection<CloudType>::setInjectionGeometry()
{
    const auto& mesh = this->owner().mesh();

    // Position
    positionVsTime_.reset(
        Function1<vector>::New("position", this->coeffDict(), &mesh));

    positionVsTime_->userTimeToTime(this->owner().time());

    if (positionVsTime_->constant())
    {
        position_ = positionVsTime_->value(0);
    }

    // Direction
    directionVsTime_.reset(
        Function1<vector>::New("direction", this->coeffDict(), &mesh));

    directionVsTime_->userTimeToTime(this->owner().time());

    if (directionVsTime_->constant())
    {
        direction_ = directionVsTime_->value(0);
        direction_.normalise();

        Random& rndGen = this->owner().rndGen();

        // Determine direction vectors tangential to direction
        vector tangent = Zero;
        scalar magTangent = 0.0;

        while (magTangent < SMALL)
        {
            vector v = rndGen.globalSample01<vector>();

            tangent = v - (v & direction_) * direction_;
            magTangent = mag(tangent);
        }

        tanVec1_ = tangent / magTangent;
        tanVec2_ = direction_ ^ tanVec1_;
    }
}


template<class CloudType>
void Foam::implicitConeNozzleInjection<CloudType>::setFlowType()
{
    switch (flowType_)
    {
    case flowType::ftConstantVelocity:
    {
        this->coeffDict().readEntry("UMag", UMag_);
        break;
    }
    case flowType::ftPressureDrivenVelocity:
    {
        Pinj_.reset(
            Function1<scalar>::New(
                "Pinj",
                this->coeffDict(),
                &this->owner().mesh()));
        Pinj_->userTimeToTime(this->owner().time());
        break;
    }
    case flowType::ftFlowRateAndDischarge:
    {
        Cd_.reset(
            Function1<scalar>::New(
                "Cd",
                this->coeffDict(),
                &this->owner().mesh()));
        Cd_->userTimeToTime(this->owner().time());
        break;
    }
    default:
    {
        FatalErrorInFunction
            << "Unhandled flow type "
            << flowTypeNames[flowType_]
            << exit(FatalError);
    }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::implicitConeNozzleInjection<CloudType>::implicitConeNozzleInjection(
    const dictionary& dict,
    CloudType& owner,
    const word& modelName)
: InjectionModel<CloudType>(dict, owner, modelName, typeName),
  injectionMethod_(
      injectionMethodNames.get("injectionMethod", this->coeffDict())),
  flowType_(flowTypeNames.get("flowType", this->coeffDict())),
  outerDiameter_(this->coeffDict().getScalar("outerDiameter")),
  innerDiameter_(this->coeffDict().getScalar("innerDiameter")),
  duration_(this->coeffDict().getScalar("duration")),
  positionVsTime_(nullptr),
  position_(Zero),
  injectorCell_(-1),
  tetFacei_(-1),
  tetPti_(-1),
  directionVsTime_(nullptr),
  direction_(Zero),
  omegaPtr_(
      Function1<scalar>::NewIfPresent(
          "omega",
          this->coeffDict(),
          &owner.mesh())),
  parcelsPerSecond_(this->coeffDict().getScalar("parcelsPerSecond")),
  flowRateProfile_(
      Function1<scalar>::New(
          "flowRateProfile",
          this->coeffDict(),
          &owner.mesh())),
  thetaInner_(
      Function1<scalar>::New(
          "thetaInner",
          this->coeffDict(),
          &owner.mesh())),
  thetaOuter_(
      Function1<scalar>::New(
          "thetaOuter",
          this->coeffDict(),
          &owner.mesh())),
  sizeDistribution_(
      distributionModel::New(
          this->coeffDict().subDict("sizeDistribution"),
          owner.rndGen())),
  t0_(this->template getModelProperty<scalar>("t0")),
  nInjectors_(
      this->coeffDict().template getOrDefault<scalar>("nInjectors", 1)),
  Uinjector_(Zero),
  initialInjectorDir_(
      this->coeffDict().template getOrDefault<vector>(
          "initialInjectorDir",
          Zero)),
  tanVec1_(Zero),
  tanVec2_(Zero),
  normal_(Zero),

  UMag_(0.0),
  Cd_(nullptr),
  Pinj_(nullptr)
{
    if (innerDiameter_ >= outerDiameter_)
    {
        FatalErrorInFunction
            << "Inner diameter must be less than the outer diameter:" << nl
            << "    innerDiameter: " << innerDiameter_ << nl
            << "    outerDiameter: " << outerDiameter_
            << exit(FatalError);
    }

    if (nInjectors_ < SMALL)
    {
        FatalIOErrorInFunction(this->coeffDict())
            << "Number of injectors in angular-segmented disc "
            << "must be positive" << nl
            << "    nInjectors: " << nInjectors_ << nl
            << exit(FatalIOError);
    }

    // Convert from user time to reduce the number of time conversion calls
    const Time& time = owner.db().time();
    duration_ = time.userTimeToTime(duration_);
    flowRateProfile_->userTimeToTime(time);
    thetaInner_->userTimeToTime(time);
    thetaOuter_->userTimeToTime(time);

    if (omegaPtr_)
    {
        omegaPtr_->userTimeToTime(time);
    }

    setInjectionGeometry();

    setFlowType();

    // Set total volume/mass to infinity to bypass the InjectionModel
    // base class kill-switch (total mass / volume accumulation). The
    // injection will now be strictly governed by the physical runTime
    // and duration_.
    this->volumeTotal_ = Foam::GREAT;
    this->massTotal_ = Foam::GREAT;

    updateMesh();
}


template<class CloudType>
Foam::implicitConeNozzleInjection<CloudType>::implicitConeNozzleInjection(
    const implicitConeNozzleInjection<CloudType>& im)
: InjectionModel<CloudType>(im),
  injectionMethod_(im.injectionMethod_),
  flowType_(im.flowType_),
  outerDiameter_(im.outerDiameter_),
  innerDiameter_(im.innerDiameter_),
  duration_(im.duration_),
  positionVsTime_(im.positionVsTime_.clone()),
  position_(im.position_),
  injectorCell_(im.injectorCell_),
  tetFacei_(im.tetFacei_),
  tetPti_(im.tetPti_),
  directionVsTime_(im.directionVsTime_.clone()),
  direction_(im.direction_),
  omegaPtr_(im.omegaPtr_.clone()),
  parcelsPerSecond_(im.parcelsPerSecond_),
  flowRateProfile_(im.flowRateProfile_.clone()),
  thetaInner_(im.thetaInner_.clone()),
  thetaOuter_(im.thetaOuter_.clone()),
  sizeDistribution_(im.sizeDistribution_.clone()),
  t0_(im.t0_),
  nInjectors_(im.nInjectors_),
  Uinjector_(im.Uinjector_),
  initialInjectorDir_(im.initialInjectorDir_),
  tanVec1_(im.tanVec1_),
  tanVec2_(im.tanVec2_),
  normal_(im.normal_),
  UMag_(im.UMag_),
  Cd_(im.Cd_.clone()),
  Pinj_(im.Pinj_.clone())
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void Foam::implicitConeNozzleInjection<CloudType>::updateMesh()
{
    // Set/cache the injector cell info for static methods
    if (positionVsTime_->constant())
    {
        position_ = positionVsTime_->value(0);

        this->findCellAtPosition(
            injectorCell_,
            tetFacei_,
            tetPti_,
            position_);
    }
}

template<class CloudType>
Foam::scalar Foam::implicitConeNozzleInjection<CloudType>::timeEnd() const
{
    return this->SOI_ + duration_;
}

// override base class prepareForNextTimeStep
template<class CloudType>
bool Foam::implicitConeNozzleInjection<CloudType>::prepareForNextTimeStep(
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
        t0_ = histT0_[foundIndex];

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
        histT0_.resize(foundIndex + 1);
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
        histT0_.push_back(t0_);
        histRnd_.push_back(this->owner().rndGen());
    }

    // pass control back to native base class to run its standard math
    return InjectionModel<CloudType>::prepareForNextTimeStep(
        time,
        newParcels,
        newVolumeFraction);
}

template<class CloudType>
Foam::label Foam::implicitConeNozzleInjection<CloudType>::parcelsToInject(
    const scalar time0,
    const scalar time1)
{
    // use -SMALL to account for floating point trunctatoin during time rewinds
    if ((time0 >= -SMALL) && (time0 < duration_))
    {
        return floor((time1 - time0) * parcelsPerSecond_);
    }

    return 0;
}


template<class CloudType>
Foam::scalar Foam::implicitConeNozzleInjection<CloudType>::volumeToInject(
    const scalar time0,
    const scalar time1)
{
    // use -SMALL to account for floating point trunctatoin during time rewinds
    if ((time0 >= -SMALL) && (time0 < duration_))
    {
        return flowRateProfile_->integrate(time0, time1);
    }

    return 0.0;
}


template<class CloudType>
void Foam::implicitConeNozzleInjection<CloudType>::setPositionAndCell(
    const label,
    const label,
    const scalar time,
    vector& position,
    label& cellOwner,
    label& tetFacei,
    label& tetPti)
{
    Random& rndGen = this->owner().rndGen();
    const scalar t = time - this->SOI_;

    if (!directionVsTime_->constant())
    {
        direction_ = directionVsTime_->value(t);
        direction_.normalise();

        // Determine direction vectors tangential to direction
        vector tangent = Zero;
        scalar magTangent = 0.0;

        while (magTangent < SMALL)
        {
            vector v = rndGen.globalSample01<vector>();

            tangent = v - (v & direction_) * direction_;
            magTangent = mag(tangent);
        }

        tanVec1_ = tangent / magTangent;
        tanVec2_ = direction_ ^ tanVec1_;
    }

    scalar beta = mathematical::twoPi * rndGen.globalSample01<scalar>();
    normal_ = tanVec1_ * cos(beta) + tanVec2_ * sin(beta);

    switch (injectionMethod_)
    {
    case injectionMethod::imPoint:
    {
        if (positionVsTime_->constant())
        {
            position = position_;
            cellOwner = injectorCell_;
            tetFacei = tetFacei_;
            tetPti = tetPti_;
        }
        else
        {
            position = positionVsTime_->value(t);

            // Estimate the moving injector velocity
            const vector position0(positionVsTime_->value(t0_));
            const scalar dt = t - t0_;
            if (dt > 0)
            {
                Uinjector_ = (position - position0) / dt;
            }

            this->findCellAtPosition(
                cellOwner,
                tetFacei,
                tetPti,
                position);
        }
        break;
    }
    case injectionMethod::imDisc:
    {
        scalar frac = rndGen.globalSample01<scalar>();
        scalar dr = outerDiameter_ - innerDiameter_;
        scalar r = 0.5 * (innerDiameter_ + frac * dr);

        position = positionVsTime_->value(t) + r * normal_;

        // Estimate the moving injector velocity
        const vector position0(positionVsTime_->value(t0_) + r * normal_);
        const scalar dt = t - t0_;
        if (dt > 0)
        {
            Uinjector_ = (position - position0) / dt;
        }

        this->findCellAtPosition(
            cellOwner,
            tetFacei,
            tetPti,
            position);
        break;
    }
    case injectionMethod::imDiscSegments:
    {
        // Calculate the uniform angular increment in radians
        const scalar angleIncrement = mathematical::twoPi / nInjectors_;

        // Randomly set the index of injector angles
        const label injectorIndex =
            rndGen.globalPosition<label>(0, nInjectors_ - 1);

        // Calculate the angle for the current injection
        const scalar angle = injectorIndex * angleIncrement;

        // Calculate the rotation tensor around an axis by an angle
        const tensor R(
            coordinateRotations::axisAngle::rotation(
                direction_,
                angle,
                false // radians
                ));

        // Compute a random radius between innerDiameter_ and outerDiameter_
        const scalar fraction = rndGen.globalSample01<scalar>();
        const scalar dr = outerDiameter_ - innerDiameter_;
        const scalar radius = 0.5 * (innerDiameter_ + fraction * dr);

        // Rotate the initial direction to get the normal direction
        // for this injector
        normal_ = R & initialInjectorDir_;

        // Compute the particle's injection position
        const vector radialOffset(radius * normal_);
        position = positionVsTime_->value(t) + radialOffset;

        // Estimate the injector velocity if time has advanced
        const scalar dt = t - t0_;
        if (dt > 0)
        {
            const vector position0(
                positionVsTime_->value(t0_) + radialOffset);
            Uinjector_ = (position - position0) / dt;
        }

        this->findCellAtPosition(
            cellOwner,
            tetFacei,
            tetPti,
            position);
        break;
    }
    default:
    {
        FatalErrorInFunction
            << "Unhandled injection method "
            << injectionMethodNames[injectionMethod_]
            << exit(FatalError);
    }
    }

    // Update the previous time step
    t0_ = t;
}


template<class CloudType>
void Foam::implicitConeNozzleInjection<CloudType>::setProperties(
    const label parcelI,
    const label,
    const scalar time,
    typename CloudType::parcelType& parcel)
{
    Random& rndGen = this->owner().rndGen();

    // Set particle velocity
    scalar t = time - this->SOI_;
    scalar ti = thetaInner_->value(t);
    scalar to = thetaOuter_->value(t);
    scalar coneAngle = degToRad(rndGen.sample01<scalar>() * (to - ti) + ti);

    scalar alpha = sin(coneAngle);
    scalar dcorr = cos(coneAngle);

    vector normal = alpha * normal_;
    vector dirVec = dcorr * direction_;
    dirVec += normal;
    dirVec.normalise();

    switch (flowType_)
    {
    case flowType::ftConstantVelocity:
    {
        parcel.U() = UMag_ * dirVec + Uinjector_;
        break;
    }
    case flowType::ftPressureDrivenVelocity:
    {
        scalar pAmbient = this->owner().pAmbient();
        scalar rho = parcel.rho();
        scalar UMag = ::sqrt(2.0 * (Pinj_->value(t) - pAmbient) / rho);
        parcel.U() = UMag * dirVec + Uinjector_;
        break;
    }
    case flowType::ftFlowRateAndDischarge:
    {
        scalar Ao = 0.25 * mathematical::pi * outerDiameter_ * outerDiameter_;
        scalar Ai = 0.25 * mathematical::pi * innerDiameter_ * innerDiameter_;
        scalar massFlowRate =
            this->massTotal()
            * flowRateProfile_->value(t)
            / this->volumeTotal();

        scalar Umag = massFlowRate / (parcel.rho() * Cd_->value(t) * (Ao - Ai));
        parcel.U() = Umag * dirVec + Uinjector_;
        break;
    }
    default:
    {
        FatalErrorInFunction
            << "Unhandled injection method "
            << flowTypeNames[flowType_]
            << exit(FatalError);
    }
    }


    if (omegaPtr_)
    {
        const scalar omega = omegaPtr_->value(t);

        const vector p0(parcel.position() - positionVsTime_->value(t));
        const vector r(p0 - (p0 & direction_) * direction_);
        const scalar rMag = mag(r);

        const vector d = normalised(normal_ ^ dirVec);

        parcel.U() += omega * rMag * d;
    }

    // Set particle diameter
    parcel.d() = sizeDistribution_->sample();
}


template<class CloudType>
bool Foam::implicitConeNozzleInjection<CloudType>::fullyDescribed() const
{
    return false;
}


template<class CloudType>
bool Foam::implicitConeNozzleInjection<CloudType>::validInjection(const label)
{
    return true;
}


template<class CloudType>
void Foam::implicitConeNozzleInjection<CloudType>::info()
{
    InjectionModel<CloudType>::info();

    if (this->writeTime())
    {
        this->setModelProperty("t0", t0_);
    }
}

// ************************************************************************* //
