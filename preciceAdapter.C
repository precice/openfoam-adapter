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

#include "preciceAdapter.H"
#include "Time.H"
#include "fvMesh.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(preciceAdapter, 0);
    addToRunTimeSelectionTable(functionObject, preciceAdapter, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapter::preciceAdapter
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict)
{
    Info << "---[preciceAdapter] CONSTRUCTOR --------" << nl;
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapter::~preciceAdapter()
{
    Info << "---[preciceAdapter] DESTRUCTOR ---------" << nl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::preciceAdapter::read(const dictionary& dict)
{
    Info << "---[preciceAdapter] READ ---------------" << nl;
    return true;
}


bool Foam::functionObjects::preciceAdapter::execute()
{
    Info << "---[preciceAdapter] EXECUTE ------------" << nl;
    return true;
}


bool Foam::functionObjects::preciceAdapter::end()
{
    Info << "---[preciceAdapter] END ----------------" << nl;
    return true;
}


bool Foam::functionObjects::preciceAdapter::write()
{
    Info << "---[preciceAdapter] WRITE --------------" << nl;
    return true;
}

bool Foam::functionObjects::preciceAdapter::adjustTimeStep()
{
    Info << "---[preciceAdapter] ADJUSTTIMESTEP -----" << nl;
    return true;
}

// ************************************************************************* //
