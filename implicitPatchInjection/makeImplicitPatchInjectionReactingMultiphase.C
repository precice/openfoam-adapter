#include "implicitPatchInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicReactingMulitphaseCloud
#include "basicReactingMultiphaseCloud.H"

namespace Foam
{
    makeInjectionModelType(implicitPatchInjection, basicReactingMultiphaseCloud);
}
