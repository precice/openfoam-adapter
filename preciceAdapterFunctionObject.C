/*---------------------------------------------------------------------------*\
preCICE-adapter for OpenFOAM

Copyright (c) 2017 Gerasimos Chourdakis
-------------------------------------------------------------------------------

License
    This adapter is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This adapter is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with the adapter.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "preciceAdapterFunctionObject.H"

// OpenFOAM header files
#include "Time.H"
#include "fvMesh.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(preciceAdapterFunctionObject, 0);
    addToRunTimeSelectionTable(functionObject, preciceAdapterFunctionObject, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapterFunctionObject::preciceAdapterFunctionObject
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict),
    adapter_(runTime)
{
    Info << "---[preciceAdapter] functionObject: CONSTRUCTOR --------" << nl;
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapterFunctionObject::~preciceAdapterFunctionObject()
{
    Info << "---[preciceAdapter] functionObject: DESTRUCTOR ---------" << nl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::preciceAdapterFunctionObject::read(const dictionary& dict)
{
    Info << "---[preciceAdapter] functionObject: READ ---------------" << nl;

    // Configure the adapter
    return adapter_.configure();
}


bool Foam::functionObjects::preciceAdapterFunctionObject::execute()
{
    Info << "---[preciceAdapter] functionObject: EXECUTE (i > 1)------" << nl;

    adapter_.execute();

    return true;
}


bool Foam::functionObjects::preciceAdapterFunctionObject::end()
{
    Info << "---[preciceAdapter] functionObject: END ----------------" << nl;
    return true;
}


bool Foam::functionObjects::preciceAdapterFunctionObject::write()
{
    Info << "---[preciceAdapter] functionObject: WRITE --------------" << nl;
    return true;
}

bool Foam::functionObjects::preciceAdapterFunctionObject::adjustTimeStep()
{
    Info << "---[preciceAdapter] functionObject: ADJUSTTIMESTEP -----" << nl;

    adapter_.adjustTimeStep();

    return true;
}

// ************************************************************************* //
