// copied from OpenFOAMv10 which is missing 
// on newer versions

#ifndef primitivePatchInterpolation_H
#define primitivePatchInterpolation_H

#include "PrimitivePatchInterpolation.H"
#include "PrimitivePatch.H"
#include "face.H"
#include "SubList.H"
#include "pointField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    typedef PrimitivePatchInterpolation
    <
        PrimitivePatch<SubList<face>, const pointField&>
    >
    primitivePatchInterpolation;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //