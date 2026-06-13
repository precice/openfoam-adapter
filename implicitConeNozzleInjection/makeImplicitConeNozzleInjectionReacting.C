#include "implicitConeNozzleInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicReactingCloud
#include "basicReactingCloud.H"

namespace Foam
{
makeInjectionModelType(implicitConeNozzleInjection, basicReactingCloud);
}
