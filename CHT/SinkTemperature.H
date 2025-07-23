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

#ifndef CHT_SINKTEMPERATURE_H
#define CHT_SINKTEMPERATURE_H

#include "CouplingDataUser.H"
#include "mixedFvPatchFields.H"
#include "volFields.H"

namespace preciceAdapter
{
namespace CHT
{

//- Class that writes and reads sink temperature.
class SinkTemperature : public CouplingDataUser
{

private:
    //- Temperature field
    Foam::volScalarField* T_;
    const Foam::fvMesh& mesh_;

public:
    //- Constructor
    SinkTemperature(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Write the sink temperature values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read the sink temperature values from the buffer
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;
};

}
}

#endif
