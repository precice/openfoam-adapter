#include "implicitPatchInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicKinematicCollidingCloud
#include "basicKinematicCollidingCloud.H"

namespace Foam
{
    makeInjectionModelType(implicitPatchInjection, basicKinematicCollidingCloud);
}
