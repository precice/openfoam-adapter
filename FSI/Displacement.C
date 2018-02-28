#include "Displacement.H"

using namespace Foam;

preciceAdapter::FSI::Displacement::Displacement
(
    const Foam::fvMesh& mesh
    /* TODO: We should add any required field names here.
    /  They would need to be vector fields.
    /  See CHT/Temperature.C for details.
    */
)
/* TODO: We probably need to initialize some fields here,
/  see CHT/Temperature.C.
*/
{
    dataType_ = vector;
}

void preciceAdapter::FSI::Displacement::write(double * buffer)
{
    /* TODO: Implement
    */
}

void preciceAdapter::FSI::Displacement::read(double * buffer)
{
    /* TODO: Implement
    */
}
