#ifndef fvMesh_H
#define fvMesh_H

#include "timeMock.h"
#include "meshMock.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

    class fvMeshLduAddressing;
    class volMesh{};

/*---------------------------------------------------------------------------*\
                           Class fvMesh Declaration
\*---------------------------------------------------------------------------*/

    class fvMesh
    {
        classMock classMock_;

    public:
        fvMesh()=default;
        ~fvMesh()=default;

        // Public typedefs

        typedef fvMesh Mesh;
//        typedef fvBoundaryMesh BoundaryMesh;

        // Member Functions
        MOCK_METHOD(int, nCells, (), (const));
        MOCK_METHOD(int, nInternalFaces, (), (const));
        MOCK_METHOD(Foam::word, faceOwner, (), (const));
        MOCK_METHOD(surfaceScalarField&, phi, (), (const));
        MOCK_METHOD(void, movePoints, (const pointField&));
        MOCK_METHOD(volScalarField::Internal&, V0, (), (const));
        MOCK_METHOD(volScalarField::Internal&, V00, (), (const));
        MOCK_METHOD(void, clearOut, ());
        MOCK_METHOD(void, removeFvBoundary, ());
        MOCK_METHOD(Foam::pointField, points, (), (const));
        MOCK_METHOD(Foam::pointField, oldPoints, (), (const));
        MOCK_METHOD(bool, moving, (), (const));
        // Helpers
        MOCK_METHOD(bool, foundObject, (const word&), (const));
        MOCK_METHOD(const volSymmTensorField&, lookupObjectvolSymmTensorField, (), (const));

        MOCK_METHOD(classMock&, functionObjects, ());
        MOCK_METHOD(classMock&, lookupClass, ());

        template<class Type>
        const Type& lookupObject(const word& name) const{
            return lookup(name, Type());
        };

        template<class Type>
        const classMock& lookupClass(const bool strict = false) const{
            return classMock_;
        };

        template<class Type>
        bool foundObject(const word& obj) const{
            return true;
        }


        template<class Type>
        typename pTraits<Type>::labelType validComponents() const;


        // Member Operators

        bool operator!=(const fvMesh&) const;
        bool operator==(const fvMesh&) const;
    private:
        MOCK_METHOD(const classMock&, lookup, (const word&, const classMock&), (const));
        MOCK_METHOD(const volScalarField&, lookup, (const word&, const volScalarField&), (const));
        MOCK_METHOD(const volVectorField&, lookup, (const word&, const volVectorField&), (const));

        MOCK_METHOD(const volTensorField&, lookup, (const word&, const volTensorField&), (const));
        MOCK_METHOD(const surfaceScalarField&, lookup, (const word&, const surfaceScalarField&), (const));
        MOCK_METHOD(const surfaceVectorField&, lookup, (const word&, const surfaceVectorField&), (const));
        MOCK_METHOD(const surfaceTensorField&, lookup, (const word&, const surfaceTensorField&), (const));
        MOCK_METHOD(const pointScalarField&, lookup, (const word&, const pointScalarField&), (const));
        MOCK_METHOD(const pointVectorField&, lookup, (const word&, const pointVectorField&), (const));
        MOCK_METHOD(const pointTensorField&, lookup, (const word&, const pointTensorField&), (const));
        MOCK_METHOD(const volSymmTensorField&, lookup, (const word&, const volSymmTensorField&), (const));
    };



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

typedef Foam::surfaceScalarField surfaceScalarField;
typedef Foam::surfaceVectorField surfaceVectorField;
typedef Foam::surfaceTensorField surfaceTensorField;

typedef Foam::pointScalarField pointScalarField;
typedef Foam::pointVectorField pointVectorField;
typedef Foam::pointTensorField pointTensorField;

typedef Foam::volScalarField volScalarField;
typedef Foam::volVectorField volVectorField;
typedef Foam::volTensorField volTensorField;
typedef Foam::volSymmTensorField volSymmTensorField;

#endif

