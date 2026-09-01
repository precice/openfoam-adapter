/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2021 OpenCFD Ltd.
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

Class
    Foam::implicitInjectionWrapper

Group
    grpLagrangianIntermediateInjectionSubModels

Description

    Wrapper template class for overriding the prepareForNextTimeStep function
    from the InjectionModel class for every InjectionModel type.

    This creates "implicit" versions of the native OpenFOAM InjectionModel
    types (implicitPatchInjection, implicitConeNozzleInjection, implicitCellZoneInjection,
    implicitManualInjection, etc.) that support time-rewind from preCICE
    implicit coupling.

    Derived from native InjectionModels (v2412) to support Eulerian-Lagrangian 
    implicit coupling. Explicitly handles injection memory removal, 
    delayedVolume_ fractional mass caching, time0_ and timeStep0_ caching,
    and rndGen() seed restoration during preCICE sub-iteration rewinds to
    ensure strict mass conservation and eliminate Monte Carlo noise.

    Native OpenFOAM InjectionModels found here: https://gitlab.com/openfoam/core/openfoam/-/tree/OpenFOAM-v2412/src/lagrangian/intermediate/submodels/Kinematic/InjectionModel

SourceFiles
    implicitInjectionWrapper.C

\*---------------------------------------------------------------------------*/

#ifndef implicitInjectionWrapper_H
#define implicitInjectionWrapper_H

#include "InjectionModel.H"
#include <vector>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Base class for implicit injectors of various kinds. Specific instances
// will be generated using the DEFINE_IMPLICIT_INJECTOR macro defined later.
template<template<class> class BaseInjector, class CloudType>
class ImplicitInjectorBase : public BaseInjector<CloudType>
{
private:
    /*- preCICE implicit coupling timeline buffers */

    /*- Cache the history of times to determine which index corresponds to */
    /* the beginning of the time window */
    std::vector<scalar> histTime_;

    /*- Cache the time-window history of delayed volume to prevent */
    /* fractional mass leaks on rewind */
    std::vector<scalar> histDelayedVolume_;

    /*- cache the time-window history of the mass injected value */
    std::vector<scalar> histMassInjected_;

    /*- cache the time-window history of the parcels added value */
    std::vector<label> histParcelsAdded_;

    /*- cache the time-window history of the N injections value */
    std::vector<label> histNInjections_;

    /*- Cache the time-window history of the random number generator state */
    /* to eliminate Monte Carlo noise */
    std::vector<Random> histRnd_;

protected:
    /*- Determine properties for next time step/injection interval */
    /* use to override base class prepareForNextTimeStep */
    virtual bool prepareForNextTimeStep(
        const scalar time,
        label& newParcels,
        scalar& newVolumeFraction) override
    {
        /* Calculate the exact mathematical start of the current fluid */
        /* timestep */
        scalar dt = this->owner().db().time().deltaTValue();
        scalar meshTimeStart = time - dt;

        /* define relative tolerance to help us find the index for the */
        /* time-window start timestep */
        scalar tolerance = 1e-6 * dt;

        /* search the history buffer to see if we are rewinding to */
        /* previously visited timestep */
        int foundIndex = -1;
        for (int i = histTime_.size() - 1; i >= 0; --i)
        {
            /* use the relative tolerance to match to the timestep */
            if (mag(histTime_[i] - meshTimeStart) <= tolerance)
            {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex != -1)
        {
            /* rewind detected, we have been at this timestep before */
            this->timeStep0_ = meshTimeStart;
            this->time0_ = meshTimeStart;

            /* restore exact states from this moment in history */
            this->delayedVolume_ = histDelayedVolume_[foundIndex];
            this->massInjected_ = histMassInjected_[foundIndex];
            this->parcelsAddedTotal_ = histParcelsAdded_[foundIndex];
            this->nInjections_ = histNInjections_[foundIndex];

            /* restore the RNG state */
            this->owner().rndGen() = histRnd_[foundIndex];

            /* erase all history AFTER this point, as we are overwriting */
            /* the timeline */
            histTime_.resize(foundIndex + 1);
            histDelayedVolume_.resize(foundIndex + 1);
            histMassInjected_.resize(foundIndex + 1);
            histParcelsAdded_.resize(foundIndex + 1);
            histNInjections_.resize(foundIndex + 1);
            histRnd_.resize(foundIndex + 1);
        }
        else
        {
            /* forward march detected; a new timestep we haven't seen yet */
            /* save the current states BEFORE the base class changes them */
            histTime_.push_back(meshTimeStart);
            histDelayedVolume_.push_back(this->delayedVolume_);
            histMassInjected_.push_back(this->massInjected_);
            histParcelsAdded_.push_back(this->parcelsAddedTotal_);
            histNInjections_.push_back(this->nInjections_);
            histRnd_.push_back(this->owner().rndGen());
        }

        /* pass control back to native base class to run its */
        /* standard math */
        return BaseInjector<CloudType>::prepareForNextTimeStep(
            time,
            newParcels,
            newVolumeFraction);
    }

public:
    /* Constructor */
    ImplicitInjectorBase(
        const dictionary& dict,
        CloudType& owner,
        const word& modelName)
    : BaseInjector<CloudType>(dict, owner, modelName)
    {
        /* Bypass the kill-switch for time-rewinding */
        this->volumeTotal_ = Foam::GREAT;
        this->massTotal_ = Foam::GREAT;
    }

    /*- Destructor */
    virtual ~ImplicitInjectorBase() = default;
};

#define DEFINE_IMPLICIT_INJECTOR(WrapperName, BaseInjector)                          \
    template<class CloudType>                                                        \
    class WrapperName : public ImplicitInjectorBase<BaseInjector, CloudType>         \
    {                                                                                \
    public:                                                                          \
        TypeName(#WrapperName);                                                      \
        WrapperName(const dictionary& dict, CloudType& owner, const word& modelName) \
        : ImplicitInjectorBase<BaseInjector, CloudType>(dict, owner, modelName)      \
        {                                                                            \
        }                                                                            \
    };

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
