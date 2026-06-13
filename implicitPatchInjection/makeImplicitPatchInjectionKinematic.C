#include "implicitPatchInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicKinematicCloud
#include "basicKinematicCloud.H"

namespace Foam
{
makeInjectionModelType(implicitPatchInjection, basicKinematicCloud);
}
