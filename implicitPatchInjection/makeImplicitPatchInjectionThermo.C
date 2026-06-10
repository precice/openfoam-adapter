#include "implicitPatchInjection.H"
#include "addToRunTimeSelectionTable.H"

// include basicThermoCloud
#include "basicThermoCloud.H"

namespace Foam
{
    makeInjectionModelType(implicitPatchInjection, basicThermoCloud);
}
