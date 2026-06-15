//- OpenFOAM cloud types
#include "basicKinematicCollidingCloud.H"

//- OpenFOAM injection models
#include "PatchInjection.H"
#include "ConeNozzleInjection.H"
#include "ManualInjection.H"

//- Utilities
#include "implicitInjectionWrapper.H"

namespace Foam
{
//- use macro to create new wrapped template classes
DEFINE_IMPLICIT_INJECTOR(implicitPatchInjection, PatchInjection);
DEFINE_IMPLICIT_INJECTOR(implicitConeNozzleInjection, ConeNozzleInjection);
DEFINE_IMPLICIT_INJECTOR(implicitManualInjection, ManualInjection);

//- Register for basicKinematicCollidingCloud
makeInjectionModelType(implicitPatchInjection, basicKinematicCollidingCloud);
makeInjectionModelType(implicitConeNozzleInjection, basicKinematicCollidingCloud);
makeInjectionModelType(implicitManualInjection, basicKinematicCollidingCloud);
}
