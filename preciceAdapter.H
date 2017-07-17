/*---------------------------------------------------------------------------*\
Class
    Foam::functionObjects::preciceAdapter

Description
    This function object prepares an OpenFOAM solver for preCICE (precice.org).
    TODO: Add more information.

    Example of function object specification:
    \verbatim
    preCICE_Adapter
    {
        type preciceAdapter;
        libs ("libpreciceAdapterFunctionObject.so");
    }
    \endverbatim

Usage
    \table
        Property     | Description               | Required | Default value
        type         | type name: preciceAdapter | yes      |
    \endtable

SourceFiles
    preciceAdapter.C

\*---------------------------------------------------------------------------*/

#ifndef preciceAdapter_H
#define preciceAdapter_H

#include "fvMeshFunctionObject.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                   Class preciceAdapter Declaration
\*---------------------------------------------------------------------------*/

class preciceAdapter
:
    public fvMeshFunctionObject
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        preciceAdapter(const preciceAdapter&);

        //- Disallow default bitwise assignment
        void operator=(const preciceAdapter&);


public:

    //- Runtime type information
    TypeName("preciceAdapter");


    // Constructors

        //- Construct from Time and dictionary
        preciceAdapter
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict
        );


    //- Destructor
    virtual ~preciceAdapter();


    // Member Functions

        //- Read the preciceAdapter data
        virtual bool read(const dictionary&);

        //- Execute, currently does nothing
        virtual bool execute();

        //- Execute at the final time-loop, currently does nothing
        virtual bool end();

        //- Write the preciceAdapter
        virtual bool write();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
