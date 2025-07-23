/*---------------------------------------------------------------------------*\
    Copyright (C) 2017  Gerasimos Chourdakis
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
* Copyright (C) 2025 Gesellschaft fuer Anlagen- und Reaktorsicherheit         *
*                         (GRS) gGmbH                                         *
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM-preCICE adapter.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version with terms added by GRS.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License with terms by GRS for more details.

    You should have received a copy of the GNU General Public License
    with terms by GRS along with this program. If not, please
    contact your conveyor or GRS gGmbH.
    For a copy of the unmodified GNU General Public License, see
    <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "ForceBase.H"
#include "fluidThermo.H"
#include "surfaceInterpolate.H"

using namespace Foam;


preciceAdapter::FSI::ForceBase::ForceBase(
    const Foam::fvMesh& mesh,
    const std::string solverType)
: mesh_(mesh),
  solverType_(solverType)
{
    // What about type "basic"?
    if (solverType_.compare("incompressible") != 0
        && solverType_.compare("compressible") != 0
        && solverType_.compare("solid") != 0)
    {
        FatalErrorInFunction
            << "Force based calculations only support "
            << "compressible, incompressible, or solid solver types."
            << exit(FatalError);
    }

    dataType_ = vector;
}

// Calculate viscous force
Foam::tmp<Foam::surfaceVectorField> preciceAdapter::FSI::ForceBase::devTau() const
{
    //For turbulent flows
    typedef compressibleMomentumTransportModel cmpTurbModel;
    typedef incompressibleMomentumTransportModel icoTurbModel;

    if (mesh_.foundObject<cmpTurbModel>(cmpTurbModel::typeName))
    {
        const cmpTurbModel& turb =
            mesh_.lookupObject<cmpTurbModel>(cmpTurbModel::typeName);

        return turb.devTau();
    }
    else if (mesh_.foundObject<icoTurbModel>(icoTurbModel::typeName))
    {
        const icoTurbModel& turb =
            mesh_.lookupObject<icoTurbModel>(icoTurbModel::typeName);

        return fvc::interpolate(rho()) * turb.devSigma();
    }
    else
    {
        FatalErrorInFunction
            << "No valid model for viscous stress calculation"
            << exit(FatalError);

        return surfaceVectorField::null();
    }
}

// lookup correct rho
Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::ForceBase::rho() const
{
    // If volScalarField exists, read it from registry (for compressible cases)
    // interFoam is incompressible but has volScalarField rho

    if (mesh_.foundObject<volScalarField>("rho"))
    {
        return mesh_.lookupObject<volScalarField>("rho");
    }
    else if (solverType_.compare("incompressible") == 0)
    {
        const dictionary& FSIDict =
            mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");

        return tmp<volScalarField>(
            new volScalarField(
                IOobject(
                    "rho",
                    mesh_.time().name(),
                    mesh_,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE),
                mesh_,
                dimensionedScalar(static_cast<dimensionedScalar>(FSIDict.lookup("rho")))));
    }
    else
    {
        FatalErrorInFunction
            << "Did not find the correct rho."
            << exit(FatalError);

        return volScalarField::null();
    }
}

// lookup correct mu
Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::ForceBase::mu() const
{
    if (solverType_.compare("incompressible") == 0)
    {
        if (mesh_.foundObject<fluidThermo>(physicalProperties::typeName))
        {
            const fluidThermo& thermo =
                mesh_.lookupObject<fluidThermo>(physicalProperties::typeName);

            return thermo.mu();
        }
        else
        {

            const dictionary& FSIDict =
                mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");

            dimensionedScalar nu(static_cast<dimensionedScalar>(FSIDict.lookup("nu")));

            return tmp<volScalarField>(
                new volScalarField(
                    nu * rho()));
        }
    }
    else if (solverType_.compare("compressible") == 0)
    {
        return mesh_.lookupObject<volScalarField>("thermo:mu");
    }
    else
    {
        FatalErrorInFunction
            << "Did not find the correct mu."
            << exit(FatalError);

        return volScalarField::null();
    }
}

std::size_t preciceAdapter::FSI::ForceBase::writeToBuffer(double* buffer,
                                                          volVectorField& forceField,
                                                          const unsigned int dim) const
{
    // Compute forces. See the Forces function object.
    // Stress tensor boundary field
    tmp<surfaceVectorField> tdevTau = devTau();
    const surfaceVectorField::Boundary& devTaub = tdevTau().boundaryField();

    // Density boundary field
    tmp<volScalarField> trho(rho());
    const volScalarField::Boundary& rhob =
        trho().boundaryField();

    // Pressure boundary field
    const auto& pb = mesh_.lookupObject<volScalarField>("p").boundaryField();

    int bufferIndex = 0;
    // For every boundary patch of the interface
    const surfaceVectorField::Boundary& Sfb =
        mesh_.Sf().boundaryField();

    const surfaceScalarField::Boundary& magSfb =
        mesh_.magSf().boundaryField();

    for (const label patchID : patchIDs_)
    {
        // Pressure forces
        // FIXME: We need to subtract the reference pressure for incompressible calculations
        if (solverType_.compare("incompressible") == 0)
        {
            forceField.boundaryFieldRef()[patchID] =
                Sfb[patchID] * pb[patchID] * rhob[patchID];
        }
        else if (solverType_.compare("compressible") == 0)
        {
            forceField.boundaryFieldRef()[patchID] =
                Sfb[patchID] * pb[patchID];
        }
        else
        {
            FatalErrorInFunction
                << "Forces calculation does only support "
                << "compressible or incompressible solver type."
                << exit(FatalError);
        }

        // Viscous forces
        forceField.boundaryFieldRef()[patchID] +=
            magSfb[patchID] * devTaub[patchID];

        // Write the forces to the preCICE buffer
        // For every cell of the patch
        forAll(forceField.boundaryField()[patchID], i)
        {
            for (unsigned int d = 0; d < dim; ++d)
                buffer[bufferIndex++] =
                    forceField.boundaryField()[patchID][i][d];
        }
    }
    return bufferIndex;
}

void preciceAdapter::FSI::ForceBase::readFromBuffer(double* buffer) const
{
    /* TODO: Implement
     * We need two nested for-loops for each patch,
     * the outer for the locations and the inner for the dimensions.
     * See the preCICE readBlockVectorData() implementation.
     */
    FatalErrorInFunction
        << "Reading forces is not supported."
        << exit(FatalError);
}
