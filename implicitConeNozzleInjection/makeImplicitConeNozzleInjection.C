#include "implicitConeNozzleInjection.H"

#include "basicThermoCloud.H"

namespace Foam
{
// register implicitConeNozzleInjection for use with basicThermoCloud
makeInjectionModelType(implicitConeNozzleInjection, basicThermoCloud);
}
