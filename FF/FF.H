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

#ifndef FF_H
#define FF_H

#include "Interface.H"

#include "FF/Velocity.H"
#include "FF/Pressure.H"
#include "FF/Temperature.H"
#include "FF/Alpha.H"
#include "FF/PressureGradient.H"
#include "FF/VelocityGradient.H"
#include "FF/TemperatureGradient.H"
#include "FF/AlphaGradient.H"
#include "FF/Phi.H"

namespace preciceAdapter
{
namespace FF
{

class FluidFluid
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // TODO: Create a struct for all the parameter names

    //- Solver type
    std::string solverType_ = "none";

    //- Name of the velocity field
    std::string nameU_ = "U";

    //- Name of the pressure field
    std::string nameP_ = "p";

    //- Name of the temperature field
    std::string nameT_ = "T";

    //- Name of the phase variable (alpha) field
    std::string nameAlpha_ = "alpha";

    //- Name of the face flux field
    std::string namePhi_ = "phi";

    //- Flux correction of velocity
    bool fluxCorrection_ = false;

    //- Determine the solver type
    std::string determineSolverType();

    //- Read the FF-related options from the adapter's configuration file
    bool readConfig(const Foam::IOdictionary& adapterConfig);

public:
    //- Constructor
    FluidFluid(const Foam::fvMesh& mesh);

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
