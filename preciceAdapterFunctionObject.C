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

Foam::functionObjects::preciceAdapterFunctionObject::preciceAdapterFunctionObject(
    const word& name,
    const Time& runTime,
    const dictionary& dict)
: fvMeshFunctionObject(name, runTime, dict),
  adapter_(runTime, mesh_)
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
    Info << "-------------------- preCICE adapter timers (primary rank) --------------------" << nl;
    Info << "Total time spent in the adapter and preCICE: " << timeInAll_.str() << " (format: day-hh:mm:ss.ms)" << nl;
    Info << "  Time spent in the adapter and preCICE only for setting up: " << timeInSetup_.str() << " (read() function)" << nl;
    Info << "  Time spent in the adapter and preCICE only for iterations: " << timeInExecute_.str() << " (execute() and adjustTimeStep() functions)" << nl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::preciceAdapterFunctionObject::read(const dictionary& dict)
{
    // Save the current wall clock time stamp to the clock
    clockGlobal_.update();
    adapter_.configure();
    // Accumulate the time spent in this section into a global timer.
    // Same in all function object methods.
    timeInAll_ += clockGlobal_.elapsed();
    timeInSetup_ = clockGlobal_.elapsed();

    return true;
}


bool Foam::functionObjects::preciceAdapterFunctionObject::execute()
{
    clockGlobal_.update();
    adapter_.execute();
    timeInAll_ += clockGlobal_.elapsed();
    timeInExecute_ += clockGlobal_.elapsed();

    return true;
}


bool Foam::functionObjects::preciceAdapterFunctionObject::end()
{
    clockGlobal_.update();
    adapter_.end();
    timeInAll_ += clockGlobal_.elapsed();
    timeInExecute_ += clockGlobal_.elapsed();

    return true;
}


bool Foam::functionObjects::preciceAdapterFunctionObject::write()
{
    return true;
}

bool Foam::functionObjects::preciceAdapterFunctionObject::adjustTimeStep()
{
    clockGlobal_.update();
    adapter_.adjustTimeStep();
    timeInAll_ += clockGlobal_.elapsed();
    timeInExecute_ += clockGlobal_.elapsed();

    return true;
}

// ************************************************************************* //
