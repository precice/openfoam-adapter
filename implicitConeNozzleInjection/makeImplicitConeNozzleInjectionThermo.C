#include "implicitConeNozzleInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicThermoCloud
#include "basicThermoCloud.H"

namespace Foam
{
    makeInjectionModelType(implicitConeNozzleInjection, basicThermoCloud);
}
