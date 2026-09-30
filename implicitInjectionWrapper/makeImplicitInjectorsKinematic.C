//- OpenFOAM cloud types
#include "basicKinematicCloud.H"

//- OpenFOAM injection models
#include "PatchInjection.H"
#include "ConeNozzleInjection.H"
#include "ManualInjection.H"
#include "CellZoneInjection.H"

//- Utilities
#include "implicitInjectionWrapper.H"

namespace Foam
{
//- use macro to create new wrapped template classes
DEFINE_IMPLICIT_INJECTOR(implicitPatchInjection, PatchInjection);
DEFINE_IMPLICIT_INJECTOR(implicitConeNozzleInjection, ConeNozzleInjection);
DEFINE_IMPLICIT_INJECTOR(implicitManualInjection, ManualInjection);
DEFINE_IMPLICIT_INJECTOR(implicitCellZoneInjection, CellZoneInjection);

//- Register for basicKinematicCloud
makeInjectionModelType(implicitPatchInjection, basicKinematicCloud);
makeInjectionModelType(implicitConeNozzleInjection, basicKinematicCloud);
makeInjectionModelType(implicitManualInjection, basicKinematicCloud);
makeInjectionModelType(implicitCellZoneInjection, basicKinematicCloud);
}
