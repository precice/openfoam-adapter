/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2018-2021 OpenCFD Ltd.
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
    Foam::implicitConeNozzleInjection

Group
    grpLagrangianIntermediateInjectionSubModels

Description
    Implicit cone injection with preCICE time-rewind support

    Derived from native ConeNozzleInjection (v2412) to support
    Eulerian-Lagrangian implicit coupling. Explicitly handles memory
    lobotomization, delayedVolume_ fractional mass caching, and rndGen() seed
    restoration during preCICE sub-iteration rewinds to ensure strict mass
    conservation and eliminate Monte Carlo noise.

    User specifies:
      - time of start of injection
      - injector position
      - direction (along injection axis)
      - parcel flow rate
      - inner and outer half-cone angles

    Properties:
      - Parcel diameters obtained by size distribution model.

      - Parcel velocity is calculated as:
        - Constant velocity:
          \verbatim
          U = \<specified by user\>
          \endverbatim

        - Pressure driven velocity:
          \verbatim
          U = sqrt(2*(Pinj - Pamb)/rho)
          \endverbatim

        - Flow rate and discharge:
          \verbatim
          U = V_dot/(A*Cd)
          \endverbatim

SourceFiles
    implicitConeNozzleInjection.C

\*---------------------------------------------------------------------------*/

#ifndef implicitConeNozzleInjection_H
#define implicitConeNozzleInjection_H

#include "InjectionModel.H"
#include "Enum.H"
#include "Random.H"
#include <vector>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes

template<class Type>
class Function1;

class distributionModel;

/*---------------------------------------------------------------------------*\
                     Class implicitConeNozzleInjection Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class implicitConeNozzleInjection
: public InjectionModel<CloudType>
{
public:
    //- Injection method enumeration
    enum class injectionMethod
    {
        imPoint,
        imDisc,
        imDiscSegments //<! Disc with angular segments
    };

    static const Enum<injectionMethod> injectionMethodNames;

    //- Flow type enumeration
    enum class flowType
    {
        ftConstantVelocity,
        ftPressureDrivenVelocity,
        ftFlowRateAndDischarge
    };

    static const Enum<flowType> flowTypeNames;


private:
    // Private data

    //- Point/disc injection method
    injectionMethod injectionMethod_;

    //- Flow type
    flowType flowType_;

    //- Outer nozzle diameter [m]
    const scalar outerDiameter_;

    //- Inner nozzle diameter [m]
    const scalar innerDiameter_;

    //- Injection duration [s]
    scalar duration_;

    //- Position relative to SOI []
    autoPtr<Function1<vector>> positionVsTime_;

    //- Static injector - position [m]
    vector position_;

    //- Static injector - cell containing injector position []
    label injectorCell_;

    //- Static injector - index of tet face for injector cell
    label tetFacei_;

    //- Static injector - index of tet point for injector cell
    label tetPti_;

    //- Injector direction []
    autoPtr<Function1<vector>> directionVsTime_;

    //- Cached direction vector
    vector direction_;

    //- Swirl velocity (optional)
    autoPtr<Function1<scalar>> omegaPtr_;

    //- Number of parcels to introduce per second []
    const label parcelsPerSecond_;

    //- Flow rate profile relative to SOI []
    autoPtr<Function1<scalar>> flowRateProfile_;

    //- Inner half-cone angle relative to SOI [deg]
    autoPtr<Function1<scalar>> thetaInner_;

    //- Outer half-cone angle relative to SOI [deg]
    autoPtr<Function1<scalar>> thetaOuter_;

    //- Parcel size PDF model
    autoPtr<distributionModel> sizeDistribution_;

    //- Previous-time value
    scalar t0_;

    // preCICE implicit coupling variables

    //- Track the Eulerian mesh clock to detect preCICE sub-iterations
    scalar lastMeshTimeStart_;

    //- Cache the history of times to determine which index corresponds to
    // the beginning of the time window
    std::vector<scalar> histTime_;

    //- Cache the time-window history of delayed volume to prevent
    // fractional mass leaks on rewind
    std::vector<scalar> histDelayedVolume_;

    //- cache the time-window history of the mass injected value
    std::vector<scalar> histMassInjected_;

    //- cache the time-window history of the parcels added value
    std::vector<label> histParcelsAdded_;

    //- cache the time-window history of the N injections value
    std::vector<label> histNInjections_;

    //- Cache the time-window history of the previous timestep to protect
    // moving-injector velocity math
    std::vector<scalar> histT0_;

    //- Cache the time-window history of the random number generator state
    // to eliminate Monte Carlo noise
    std::vector<Random> histRnd_;

    //- Number of injectors in angular-segmented disc
    label nInjectors_;

    //- Estimated speed of the moving injector
    vector Uinjector_;

    //- Initial injector direction
    vector initialInjectorDir_;


    // Tangential vectors to the direction vector

    //- First tangential vector
    vector tanVec1_;

    //- Second tangential vector
    vector tanVec2_;

    //- Injection vector orthogonal to direction
    vector normal_;


    // Velocity model coefficients

    //- Constant velocity [m/s]
    scalar UMag_;

    //- Discharge coefficient, relative to SOI [m/s]
    autoPtr<Function1<scalar>> Cd_;

    //- Injection pressure [Pa]
    autoPtr<Function1<scalar>> Pinj_;


    // Private Member Functions

    //- Set the injection position and direction
    void setInjectionGeometry();

    //- Set the injection flow type
    void setFlowType();


protected:
    //- Determine properties for next time step/injection interval
    // use to override base class prepareForNextTimeStep; used for
    // preCICE implicit coupling
    virtual bool prepareForNextTimeStep(
        const scalar time,
        label& newParcels,
        scalar& newVolumeFraction);


public:
    //- Runtime type information
    TypeName("implicitConeNozzleInjection");


    // Constructors

    //- Construct from dictionary
    implicitConeNozzleInjection(
        const dictionary& dict,
        CloudType& owner,
        const word& modelName);

    //- Construct copy
    implicitConeNozzleInjection(const implicitConeNozzleInjection<CloudType>& im);

    //- Construct and return a clone
    virtual autoPtr<InjectionModel<CloudType>> clone() const
    {
        return autoPtr<InjectionModel<CloudType>>(
            new implicitConeNozzleInjection<CloudType>(*this));
    }


    //- Destructor
    virtual ~implicitConeNozzleInjection() = default;


    // Member Functions

    //- Set injector locations when mesh is updated
    virtual void updateMesh();

    //- Return the end-of-injection time
    scalar timeEnd() const;

    //- Number of parcels to introduce relative to SOI
    virtual label parcelsToInject(const scalar time0, const scalar time1);

    //- Volume of parcels to introduce relative to SOI
    virtual scalar volumeToInject(const scalar time0, const scalar time1);


    // Injection geometry

    //- Set the injection position and owner cell
    virtual void setPositionAndCell(
        const label parcelI,
        const label nParcels,
        const scalar time,
        vector& position,
        label& cellOwner,
        label& tetFacei,
        label& tetPti);

    //- Set the parcel properties
    virtual void setProperties(
        const label parcelI,
        const label nParcels,
        const scalar time,
        typename CloudType::parcelType& parcel);

    //- Flag to identify whether model fully describes the parcel
    virtual bool fullyDescribed() const;

    //- Return flag to identify whether or not injection of parcelI is
    //  permitted
    virtual bool validInjection(const label parcelI);

    // I-O

    //- Write injection info
    virtual void info();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#include "implicitConeNozzleInjection.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
