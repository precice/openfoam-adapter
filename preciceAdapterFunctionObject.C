/*---------------------------------------------------------------------------*\
preCICE-adapter for OpenFOAM

Copyright (c) 2017 Gerasimos Chourdakis

Based on previous work by Lucia Cheung Yau. See also the README.md.
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
#include "foamTime.H"
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

Foam::functionObjects::preciceAdapterFunctionObject::preciceAdapterFunctionObject(
    const word& name,
    const Time& runTime,
    const dictionary& dict)
: functionObject(name),
  adapter_(runTime, runTime.lookupObject<fvMesh>(polyMesh::defaultRegion))
{

#if (defined OPENFOAM_PLUS && (OPENFOAM_PLUS >= 1712)) || (defined OPENFOAM && (OPENFOAM >= 1806))
    // Patch for issue #27: warning "MPI was already finalized" while
    // running in serial. This only affects openfoam.com, while initNull()
    // does not exist in openfoam.org.
    UPstream::initNull();
#endif

    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapterFunctionObject::~preciceAdapterFunctionObject()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::preciceAdapterFunctionObject::read(const dictionary& dict)
{

    adapter_.configure();

    return true;
}

bool Foam::functionObjects::preciceAdapterFunctionObject::start()
{
    return true;
}



bool Foam::functionObjects::preciceAdapterFunctionObject::execute()
{
    adapter_.execute();

    return true;
}

bool Foam::functionObjects::preciceAdapterFunctionObject::execute(const bool forceWrite)
{
    adapter_.execute();

    return true;
}

bool Foam::functionObjects::preciceAdapterFunctionObject::end()
{

    adapter_.end();


    return true;
}


bool Foam::functionObjects::preciceAdapterFunctionObject::write()
{
    return true;
}

bool Foam::functionObjects::preciceAdapterFunctionObject::adjustTimeStep()
{

    adapter_.adjustTimeStep();

    return true;
}

void Foam::functionObjects::preciceAdapterFunctionObject::updateMesh(const mapPolyMesh& mpm)
{

}

void Foam::functionObjects::preciceAdapterFunctionObject::movePoints(const pointField& mesh)
{

}

// ************************************************************************* //
