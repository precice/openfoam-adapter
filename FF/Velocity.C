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

#include "Velocity.H"
#include "coupledVelocityFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::Velocity::Velocity(
    const Foam::fvMesh& mesh,
    const std::string nameU,
    const std::string namePhi,
    bool fluxCorrection)
: phi_(const_cast<surfaceScalarField*>(
    &mesh.lookupObject<surfaceScalarField>(namePhi))),
  fluxCorrection_(fluxCorrection)
{
    if (mesh.foundObject<volVectorField>(nameU))
    {
        adapterInfo("Using existing velocity object " + nameU, "debug");
        U_ = const_cast<volVectorField*>(
            &mesh.lookupObject<volVectorField>(nameU));
    }
    else
    {
        adapterInfo("Creating a new velocity object " + nameU, "debug");
        U_ = new volVectorField(
            IOobject(
                nameU,
                mesh.time().name(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE),
            mesh);
    }
    dataType_ = vector;
}

std::size_t preciceAdapter::FF::Velocity::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : U_->internalField())
            {
                // x-dimension
                buffer[bufferIndex++] = cell.x();

                // y-dimension
                buffer[bufferIndex++] = cell.y();

                if (dim == 3)
                {
                    // z-dimension
                    buffer[bufferIndex++] = cell.z();
                }
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(U_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    buffer[bufferIndex++] = U_->internalField()[currentCell].x();

                    // y-dimension
                    buffer[bufferIndex++] = U_->internalField()[currentCell].y();

                    if (dim == 3)
                    {
                        // z-dimension
                        buffer[bufferIndex++] = U_->internalField()[currentCell].z();
                    }
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        vectorField UPatch = U_->boundaryField()[patchID];

        // Correct the velocity by the boundary face flux
        if (fluxCorrection_)
        {
            scalarField phip = phi_->boundaryFieldRef()[patchID];
            const vectorField n(U_->boundaryField()[patchID].patch().nf());
            const scalarField& magS = U_->boundaryFieldRef()[patchID].patch().magSf();
            UPatch = UPatch - n * (n & U_->boundaryField()[patchID]) + n * phip / magS;
        }

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++] =
                UPatch[i].x();

            // y-dimension
            buffer[bufferIndex++] =
                UPatch[i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    UPatch[i].z();
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::Velocity::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (auto& cell : U_->internalFieldRef())
            {
                // x-dimension
                cell.x() = buffer[bufferIndex++];

                // y-dimension
                cell.y() = buffer[bufferIndex++];

                if (dim == 3)
                {
                    // z-dimension
                    cell.z() = buffer[bufferIndex++];
                }
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(U_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    U_->internalFieldRef()[currentCell].x() = buffer[bufferIndex++];

                    // y-dimension
                    U_->internalFieldRef()[currentCell].y() = buffer[bufferIndex++];

                    if (dim == 3)
                    {
                        // z-dimension
                        U_->internalFieldRef()[currentCell].z() = buffer[bufferIndex++];
                    }
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity value boundary patch
        vectorField* valuePatchPtr = &U_->boundaryFieldRef()[patchID];
        if (isA<coupledVelocityFvPatchField>(U_->boundaryFieldRef()[patchID]))
        {
            valuePatchPtr = &refCast<coupledVelocityFvPatchField>(
                                 U_->boundaryFieldRef()[patchID])
                                 .refValue();
        }
        vectorField& valuePatch = *valuePatchPtr;

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            valuePatch[i].x() =
                buffer[bufferIndex++];

            // y-dimension
            valuePatch[i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                valuePatch[i].z() =
                    buffer[bufferIndex++];
            }
        }
    }
}

bool preciceAdapter::FF::Velocity::isLocationTypeSupported(const bool meshConnectivity) const
{
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceCenters);
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::FF::Velocity::getDataName() const
{
    return "Velocity";
}
