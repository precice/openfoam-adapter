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

Class
    Foam::functionObjects::preciceAdapterFunctionObject

Description
    This function object prepares an OpenFOAM solver for preCICE (precice.org).
    TODO: Add more information.

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

// Other header files
// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"
// MPI - Used to extract the MPI configuration for the preCICE initialization.
#include "mpi.h"
// preCICE Solver Interface
#include "precice/SolverInterface.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                Class preciceAdapterFunctionObject Declaration
\*---------------------------------------------------------------------------*/

class preciceAdapterFunctionObject
:
    public fvMeshFunctionObject
{
    // Private data

        //- runTime object
        const Time& runTime_;

        //- YAML Node object for the configuration file
        YAML::Node adapterConfig_;

        //- preCICE participant name
        std::string participantName_;

        //- preCICE configuration file name
        std::string preciceConfigFilename_;

        //- Type of timestep (fixed, adjustable)
        bool adjustableTimestep_;

        //- Switch for checkpointing
        bool subcyclingAllowed_;

        //- Switch for checkpointing
        bool checkpointingEnabled_;

        //- preCICE solver interface
        precice::SolverInterface * precice_;

    // Private Member Functions

        //- Produce an error and exit
        void errorAndExit(const std::string message, const std::string function);

        //- Read the adapter's configuration file
        void configFileRead();

        //- Check the adapter's configuration file
        void configFileCheck(const std::string adapterConfigFileName);

        //- Disallow default bitwise copy construct
        preciceAdapterFunctionObject(const preciceAdapterFunctionObject&);

        //- Disallow default bitwise assignment
        void operator=(const preciceAdapterFunctionObject&);


public:

    //- Runtime type information
    TypeName("preciceAdapterFunctionObject");


    // Constructors

        //- Construct from Time and dictionary
        preciceAdapterFunctionObject
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict
        );


    //- Destructor
    virtual ~preciceAdapterFunctionObject();


    // Member Functions

        //- Read the preciceAdapterFunctionObject data
        virtual bool read(const dictionary&);

        //- Execute, currently does nothing
        virtual bool execute();

        //- Execute at the final time-loop, currently does nothing
        virtual bool end();

        //- Write the preciceAdapterFunctionObject
        virtual bool write();

        //- Called at the end of Time::adjustDeltaT() if adjustTime is true
        virtual bool adjustTimeStep();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
