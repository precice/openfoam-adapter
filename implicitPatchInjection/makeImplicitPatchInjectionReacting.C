#include "implicitPatchInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicReactingCloud
#include "basicReactingCloud.H"

namespace Foam
{
makeInjectionModelType(implicitPatchInjection, basicReactingCloud);
}
