#include "implicitConeNozzleInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicKinematicCloud
#include "basicKinematicCloud.H"

namespace Foam
{
makeInjectionModelType(implicitConeNozzleInjection, basicKinematicCloud);
}
