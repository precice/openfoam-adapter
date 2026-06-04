#include "basicThermoCloud.H"
#include "implicitPatchInjection.H"

namespace Foam
{
   makeInjectionModelType(implicitPatchInjection, basicThermoCloud);
}
