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

#ifndef FSI_DISPLACEMENTDELTA_H
#define FSI_DISPLACEMENTDELTA_H

#include "CouplingDataUser.H"

#include "volFields.H"
#include "fixedValuePointPatchFields.H"
#include "primitivePatchInterpolation.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads displacement
class DisplacementDelta : public CouplingDataUser
{

private:
    // Displacement pointVectorField
    Foam::pointVectorField* pointDisplacement_;

    // cellDisplacement field
    Foam::volVectorField* cellDisplacement_;

    const Foam::fvMesh& mesh_;
    // NOTE: this allocation could be avoided in case we directly write to the
    // Needs to be a pointer since the class disables assignment and copy constructors
    std::vector<const Foam::primitivePatchInterpolation*> interpolationObjects_;

public:
    //- Constructor
    DisplacementDelta(
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement,
        const std::string nameCellDisplacement);

    //- Write the displacementDelta values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read the displacementDelta values from the buffer
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;

    //- We need to initialize the cell-based vector and the interpolation object
    // in case we want to use the faceCenter location for the coupling
    void initialize() final;
};

}
}

#endif
