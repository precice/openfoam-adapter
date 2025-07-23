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

#include "VelocityGradient.H"
#include "coupledVelocityFvPatchField.H"
#include "fixedGradientFvPatchFields.H"

using namespace Foam;

preciceAdapter::FF::VelocityGradient::VelocityGradient(
    const Foam::fvMesh& mesh,
    const std::string nameU)
: U_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(nameU)))
{
    dataType_ = vector;
}

std::size_t preciceAdapter::FF::VelocityGradient::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity gradient boundary patch
        vectorField gradientPatch((U_->boundaryFieldRef()[patchID])
                                      .snGrad());

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++] =
                -gradientPatch[i].x();

            // y-dimension
            buffer[bufferIndex++] =
                -gradientPatch[i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    -gradientPatch[i].z();
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::VelocityGradient::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity gradient boundary patch
        vectorField* gradientPatchPtr;
        if (isA<coupledVelocityFvPatchField>(U_->boundaryFieldRef()[patchID]))
        {
            gradientPatchPtr = &refCast<coupledVelocityFvPatchField>(
                                    U_->boundaryFieldRef()[patchID])
                                    .refGrad();
        }
        else
        {
            gradientPatchPtr = &refCast<fixedGradientFvPatchVectorField>(
                                    U_->boundaryFieldRef()[patchID])
                                    .gradient();
        }
        vectorField& gradientPatch = *gradientPatchPtr;


        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            gradientPatch[i].x() =
                buffer[bufferIndex++];

            // y-dimension
            gradientPatch[i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
                // z-dimension
                gradientPatch[i].z() =
                    buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::VelocityGradient::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::VelocityGradient::getDataName() const
{
    return "VelocityGradient";
}
