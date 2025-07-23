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

#ifndef FSI_H
#define FSI_H

#include "Interface.H"

#include "FSI/Displacement.H"
#include "FSI/DisplacementDelta.H"
#include "FSI/Force.H"
#include "FSI/Stress.H"

#include "volFields.H"
#include "IOdictionary.H"

namespace preciceAdapter
{
namespace FSI
{

class FluidStructureInteraction
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- OpenFOAM Time object (we need to access the timestep to compute the pointMotionU)
    const Foam::Time& runTime_;

    //- Solver type
    std::string solverType_ = "none";

    //- Restart a simulation from an undeformed or a deformed interface
    bool restartFromDeformed_;

    //- Name of the pointDisplacement field
    std::string namePointDisplacement_ = "pointDisplacement";

    //- Name of the pointDisplacement field
    std::string nameCellDisplacement_ = "cellDisplacement";

    //- Name of the force field
    std::string nameForce_ = "Force";

    /* TODO: Declare here any parameters that should be read from
    /  the configuration file. See CHT/CHT.H for reference.
    /  We want to support in-house solvers with different field names,
    /  so make sure to declare these here.
    */

protected:
    //- Determine the solver type
    std::string determineSolverType();

    //- Read the FSI-related options from the adapter's configuration file
    bool readConfig(const Foam::IOdictionary& adapterConfig);

public:
    //- Constructor
    //  We need also the runTime to access the timeName and the deltaT
    FluidStructureInteraction(const Foam::fvMesh& mesh, const Foam::Time& runTime);

    //- Configure
    bool configure(const Foam::IOdictionary& adapterConfig);

    //- Add coupling data writers
    bool addWriters(std::string dataName, Interface* interface);

    //- Add coupling data readers
    bool addReaders(std::string dataName, Interface* interface);

    //- Return the nameCellDisplacement_ field name
    std::string getCellDisplacementFieldName();

    //- Return the namePointDisplacement_ field name
    std::string getPointDisplacementFieldName();

    //- Return the boolean variable stored in the class
    bool isRestartingFromDeformed();
};

}
}

#endif
