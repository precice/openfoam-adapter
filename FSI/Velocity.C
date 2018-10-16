#include "Velocity.H"

using namespace Foam;


preciceAdapter::FSI::Velocity::Velocity
(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime,
    const std::string nameVelocity
    /* TODO: We should add any required field names here.
    /  They would need to be vector fields.
    /  See CHT/Temperature.C for details.
    */
)
:
mesh_(mesh),
runTime_(runTime),
time_(0.0),
timeOld_(0.0),
velocity_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameVelocity)
    )
)
{
    dataType_ = vector;

    // Initialize the current and old face displacement as two IOOBJECTS.
    // Maybe this is not required, and can be solved in another way.
    faceDisplacement_ = new volVectorField
    (
        IOobject
        (
            "faceDisplacement",
            runTime_.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "lengthdim",
            dimensionSet(0,1,0,0,0,0,0),
            Foam::vector::zero
        )
    );

    faceDisplacementOld_ =  new volVectorField
    (
        IOobject
        (
            "faceDisplacement_old",
            runTime_.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "lengthdim",
            dimensionSet(0,1,0,0,0,0,0),
            Foam::vector::zero
        )
    );
}

void preciceAdapter::FSI::Velocity::write(double * buffer)
{
    /* TODO: Implement
    * FOR NOW ONLY WORKS IF THE DISPLACEMENT FIELD IS ALREADY UPDATED.
    * Make this function not dependent on the buffer, but rather on the faceDisplacement
    * This can be a function in the displacement.C
    * Create velocity interpolation
    */
    FatalErrorInFunction
        << "Writing velocities is not supported."
        << exit(FatalError);
}

void preciceAdapter::FSI::Velocity::read(double * buffer)
{
    /* TODO: Implement
    * FOR NOW ONLY WORKS IF THE DISPLACEMENT FIELD IS ALREADY UPDATED.
    * check $FOAM_SRC/finiteVolume/fields/fvPatchFields/derived/movingWallVelocity
    * Or myMovingWallVelocity from David Blom for a second order interpolation.
    */
    if (time_!=runTime_.value())
    {
        // check if the function needs to be called.
        timeOld_ = time_;
        time_ = runTime_.value();

        // save the old displaement at the faceCentres.
        *faceDisplacementOld_ = *faceDisplacement_;
    }
    if ( time_ == 0 )
    {
        timeOld_ = -1.;
    }

    // Get the pointdisplacement.
    // TODO check if only the boundary field can be loaded?
    pointVectorField& pointDisplacement_ =
        const_cast<pointVectorField&>
        (
            mesh_.lookupObject<pointVectorField>("pointDisplacement")
        );

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the patchvelocity (fixedValueFvPatchVectorField)
        vectorField& velocityPatch =
           refCast<vectorField>
            (
                velocity_->boundaryFieldRef()[patchID]
            );

        // Get the pointPatchDisplacement (fixedValuePointPatchVectorField)
        vectorField& pointPatchDisplacement_ =
            refCast<vectorField>
            (
                pointDisplacement_.boundaryFieldRef()[patchID]
            );

        // Get the facePatchDisplacement (fixedValueFvPatchVectorField)
        vectorField& facePatchDisplacement_ =
            refCast<vectorField>
            (
                faceDisplacement_->boundaryFieldRef()[patchID]
            );

        // Get the old facePatchDisplacement (fixedValueFvPatchVectorField)
        vectorField& facePatchDisplacementOld_ =
            refCast<vectorField>
            (
                faceDisplacementOld_->boundaryFieldRef()[patchID]
            );

        // Define interpolator: patchInterpolator
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        // Interpolate from the points to the faces.
        facePatchDisplacement_
            =
            patchInterpolator.pointToFaceInterpolate(pointPatchDisplacement_);

        // TODO: This displacement only works without subcycling
        // For subcycling this function must be called every timestep with some approximation.
        // Loop over the cells of the patch
        forAll(velocity_->boundaryFieldRef()[patchID], i)
        {
            velocityPatch[i][0] = (facePatchDisplacement_[i][0] - facePatchDisplacementOld_[i][0]) / (time_ - timeOld_);
            velocityPatch[i][1] = (facePatchDisplacement_[i][1] - facePatchDisplacementOld_[i][1]) / (time_ - timeOld_);
            velocityPatch[i][2] = (facePatchDisplacement_[i][2] - facePatchDisplacementOld_[i][2]) / (time_ - timeOld_);
        }
        Info << "HALLO HIER BEN IK" << endl;
    }
}

//- Destructor
preciceAdapter::FSI::Velocity::~Velocity()
{
    // TODO: Is this enough?
    delete faceDisplacement_;
    delete faceDisplacementOld_;
}
