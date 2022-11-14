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

Class
    Foam::functionObjects::preciceAdapterFunctionObject

Description
    This function object prepares an OpenFOAM solver for preCICE (precice.org).
    See the README.md for details.
    Look on precice.org for the latest version of the adapter.

    NOTE: In case the functionObjects interface is different for your OpenFOAM
    version, you may create a new function object with the same name and type,
    using the template for functionObjects that you can usually find in the
    $FOAM_ETC/codeTemplates/ directory. This is only a wrapper to the
    adapter's methods (see preciceAdapterFunctionObject.C).

    Example of function object specification:
    \verbatim
    preCICE_Adapter
    {
        type preciceAdapterFunctionObject;
        libs ("libpreciceAdapterFunctionObject.so");
    }
    \endverbatim

Usage
    \table
        Property     | Description                             | Required | Default value
        type         | type name: preciceAdapterFunctionObject | yes      |
    \endtable

SourceFiles
    preciceAdapterFunctionObject.C

\*---------------------------------------------------------------------------*/

#ifndef PRECICEADAPTERFUNCTIONOBJECT_H
#define PRECICEADAPTERFUNCTIONOBJECT_H

// OpenFOAM header files
#include "fvMeshFunctionObject.H"
#include "clockValue.H"

// Main adapter header file
#include "Adapter.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                Class preciceAdapterFunctionObject Declaration
\*---------------------------------------------------------------------------*/

class preciceAdapterFunctionObject
: public fvMeshFunctionObject
{
    // Private data

#ifdef ADAPTER_ENABLE_TIMINGS
    //- Cumulative time spent in the complete adapter function object (including preCICE calls)
    clockValue timeInAll_;
    clockValue timeInSetup_;
    clockValue timeInExecute_;
#endif

    //- Adapter's main class instance
    preciceAdapter::Adapter adapter_;

    // Private Member Functions

    //- No copy construct
    preciceAdapterFunctionObject(const preciceAdapterFunctionObject&) = delete;

    //- No copy assignment
    void operator=(const preciceAdapterFunctionObject&) = delete;


public:
    //- Runtime type information
    TypeName("preciceAdapterFunctionObject");


    // Constructors

    //- Construct from Time and dictionary
    preciceAdapterFunctionObject(
        const word& name,
        const Time& runTime,
        const dictionary& dict);


    //- Destructor
    ~preciceAdapterFunctionObject() final;


    // Member Functions

    //- Read the preciceAdapterFunctionObject data
    bool read(const dictionary&) final;

    //- Execute in the beginning of each time-loop, after the first one
    bool execute() final;

    //- Execute at the final time-loop, after execute()
    bool end() final;

    //- Write the preciceAdapterFunctionObject
    bool write() final;

    //- Called at the end of Time::adjustDeltaT() if adjustTime is true
    bool adjustTimeStep() final;

    /*
        // NOTE: If you add a new module that needs to execute methods
        // whenever the mesh is updated or its points moved,
        // you may put your calls inside the following methods.

        // Update for changes of mesh
        void updateMesh(const mapPolyMesh& mpm) final;

        // Update for changes of mesh
        void movePoints(const polyMesh& mesh) final;
    */
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
