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

#ifndef CHT_TEMPERATURE_H
#define CHT_TEMPERATURE_H

#include "CouplingDataUser.H"

#include "fvMesh.H"
#include "cellSet.H"

namespace preciceAdapter
{
namespace CHT
{

//- Class that writes and reads temperature.
class Temperature : public CouplingDataUser
{

private:
    //- Temperature field
    Foam::volScalarField* T_;
    const Foam::fvMesh& mesh_;

public:
    //- Constructor
    Temperature(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Write the temperature values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read the temperature values from the buffer
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;
};

}
}

// NOTE: In order to couple another variable, you need to add another
// subclass of the CouplingDataUser. Take this file and Temperature.C
// as an example and look for more notes in the Adapter.C and Adapter.H.
// Essentially, the new class needs to implement a constructor (which also
// sets the dataType_) and implement the write() and read() methods.
// Then, you need to create objects of this class in the Adapter.C,
// whenever requested from the configuration file (see notes).

#endif
