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

#include "Pressure.H"
#include "coupledPressureFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::Pressure::Pressure(
    const Foam::fvMesh& mesh,
    const std::string nameP)
: p_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameP)))
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::FF::Pressure::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : p_->internalField())
            {
                buffer[bufferIndex++] = cell;
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(p_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // Copy the pressure into the buffer
                    buffer[bufferIndex++] = p_->internalField()[currentCell];
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++] =
                p_->boundaryFieldRef()[patchID][i];
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::Pressure::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (auto& cell : p_->internalFieldRef())
            {
                cell = buffer[bufferIndex++];
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(p_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // Copy the pressure into the buffer
                    p_->internalFieldRef()[currentCell] = buffer[bufferIndex++];
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the pressure value boundary patch
        scalarField* valuePatchPtr = &p_->boundaryFieldRef()[patchID];
        if (isA<coupledPressureFvPatchField>(p_->boundaryFieldRef()[patchID]))
        {
            valuePatchPtr = &refCast<coupledPressureFvPatchField>(
                                 p_->boundaryFieldRef()[patchID])
                                 .refValue();
        }
        scalarField& valuePatch = *valuePatchPtr;

        // For every cell of the patch
        forAll(p_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            valuePatch[i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::Pressure::isLocationTypeSupported(const bool meshConnectivity) const
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

std::string preciceAdapter::FF::Pressure::getDataName() const
{
    return "Pressure";
}
