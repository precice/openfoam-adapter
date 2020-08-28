#include "Stress.H"

using namespace Foam;

preciceAdapter::FSI::Stress::Stress
(
    const Foam::fvMesh& mesh,
    const fileName& timeName,
    const std::string solverType
    /* TODO: We should add any required field names here.
    /  They would need to be vector fields.
    /  See FSI/Temperature.C for details.
    */
)
:
Force(mesh,solverType),
mesh_(mesh),
solverType_(solverType)
{
    if (solverType_.compare("incompressible") != 0 && solverType_.compare("compressible") != 0)
    {
        FatalErrorInFunction
            << "Stresses calculation only supports "
            << "compressible or incompressible solver type."
            << exit(FatalError);
    }
    
    dataType_ = vector;

    Stress_ = new volVectorField
    (
        IOobject
        (
            "Stress",
            timeName,
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "pdim",
            dimensionSet(1,-1,-2,0,0,0,0),
            Foam::vector::zero
        )
    );
}

void preciceAdapter::FSI::Stress::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    // Compute stress. See the Forces function object.

    // Stress tensor boundary field
    tmp<volSymmTensorField> tdevRhoReff(this->devRhoReff());
    const volSymmTensorField::Boundary& devRhoReffb
    (
        tdevRhoReff().boundaryField()
    );

    // Density boundary field
    tmp<volScalarField> trho(this->rho());
    const volScalarField::Boundary& rhob
    (
        trho().boundaryField()
    );

    // Pressure boundary field
    tmp<volScalarField> tp(mesh_.lookupObject<volScalarField>("p"));
    const volScalarField::Boundary& pb
    (
        tp().boundaryField()
    );

    int bufferIndex = 0;
    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {

        int patchID = patchIDs_.at(j);

        // Compute normal vectors on each patch
        const vectorField nV = mesh_.boundary()[patchID].nf();

        // Pressure constribution
        if (solverType_.compare("incompressible") == 0)
        {
            Stress_->boundaryFieldRef()[patchID] =
                nV * pb[patchID] * rhob[patchID];
        }
        else if (solverType_.compare("compressible") == 0)
        {
            Stress_->boundaryFieldRef()[patchID] =
                nV * pb[patchID];
        }
        else
        {
            FatalErrorInFunction
                << "Stress calculation does only support "
                << "compressible or incompressible solver type."
                << exit(FatalError);
        }

        // Viscous contribution
        Stress_->boundaryFieldRef()[patchID] +=
            nV & devRhoReffb[patchID];

        // Write the stress to the preCICE buffer
        // For every cell of the patch
        forAll(Stress_->boundaryFieldRef()[patchID], i)
        {
            // Copy the stress into the buffer
            // x-dimension
            buffer[bufferIndex++]
            = 
            Stress_->boundaryFieldRef()[patchID][i].x();

            // y-dimension
            buffer[bufferIndex++]
            =
            Stress_->boundaryFieldRef()[patchID][i].y();

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                        =
                        Stress_->boundaryFieldRef()[patchID][i].z();
        }
    }
}

void preciceAdapter::FSI::Stress::read(double * buffer, const unsigned int dim)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE readBlockVectorData() implementation.
    */
    FatalErrorInFunction
        << "Reading stresses is not supported."
        << exit(FatalError);
}

preciceAdapter::FSI::Stress::~Stress()
{
    delete Stress_;
}
