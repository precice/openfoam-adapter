#include "implicitConeNozzleInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicKinematicCollidingCloud
#include "basicKinematicCollidingCloud.H"

namespace Foam
{
makeInjectionModelType(implicitConeNozzleInjection, basicKinematicCollidingCloud);
}
