#include "implicitConeNozzleInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicReactingMultiphaseCloud
#include "basicReactingMultiphaseCloud.H"

namespace Foam
{
makeInjectionModelType(implicitConeNozzleInjection, basicReactingMultiphaseCloud);
}
