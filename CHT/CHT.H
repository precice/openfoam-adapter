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

#ifndef CHT_H
#define CHT_H

#include "Interface.H"

#include "CHT/Temperature.H"
#include "CHT/HeatFlux.H"
#include "CHT/SinkTemperature.H"
#include "CHT/HeatTransferCoefficient.H"

namespace preciceAdapter
{
namespace CHT
{

class ConjugateHeatTransfer
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // TODO: Create a struct for all the parameter names

    //- Solver type
    std::string solverType_ = "none";

    //- Name of the temperature field
    std::string nameT_ = "T";

    //- Name of the thermal conductivity for a basic solver
    std::string nameKappa_ = "k";

    //- Name of the density for an incompressible solver
    std::string nameRho_ = "rho";

    //- Name of the heat capacity for an incompressible solver
    std::string nameCp_ = "Cp";

    //- Name of the Prandtl number for an incompressible solver
    std::string namePr_ = "Pr";

    //- Name of the turbulent thermal diffusivity field for an incompressible solver
    std::string nameAlphat_ = "alphat";

protected:
    //- Determine the solver type
    std::string determineSolverType();

    //- Read the CHT-related options from the adapter's configuration file
    bool readConfig(const Foam::IOdictionary& adapterConfig);

public:
    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const Foam::IOdictionary& adapterConfig);

    //- Add coupling data writers
    bool addWriters(std::string dataName, Interface* interface);

    //- Add coupling data readers
    bool addReaders(std::string dataName, Interface* interface);
};

}
}

#endif
