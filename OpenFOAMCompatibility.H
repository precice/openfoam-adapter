#ifndef OPENFOAMCOMPATIBILITY_H
#define OPENFOAMCOMPATIBILITY_H

// OpenFOAM 14 removed the legacy fvCFD.H and primitivePatchInterpolation.H headers.
// Keep compatibility with both older and newer OpenFOAM releases by falling back to
// the modern include names when the legacy headers are not available.
//
// On newer OpenFOAM releases, the legacy umbrella headers are not present and the
// required mesh/field types must be included explicitly. `fv.H` alone is not enough
// to declare fvMesh, volVectorField, and pointVectorField in the finite-volume API.
#if __has_include("fvCFD.H")
    #include "fvCFD.H"
#else
    #include "Time.H"
    #include "fvMesh.H"
    #include "volFields.H"
    #include "pointFields.H"
#endif

#if __has_include("primitivePatchInterpolation.H")
    #include "primitivePatchInterpolation.H"
#else
    #include "PrimitivePatchInterpolation.H"
#endif

#include "fvc.H"
#include "IOdictionary.H"
#include "surfaceFields.H"
#include "fixedGradientFvPatchFields.H"
#include "mixedFvPatchFields.H"

namespace Foam
{
using primitivePatchInterpolation = PrimitivePatchInterpolation<polyPatch>;
}

using namespace Foam;

#endif
