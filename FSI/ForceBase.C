#include "ForceBase.H"

using namespace Foam;


preciceAdapter::FSI::ForceBase::ForceBase(
    const Foam::fvMesh& mesh,
    const std::string solverType)
: mesh_(mesh),
  solverType_(solverType)
{
    // What about type "basic"?
    if (solverType_.compare("incompressible") != 0
        && solverType_.compare("compressible") != 0
        && solverType_.compare("solid") != 0)
    {
        FatalErrorInFunction
            << "Force based calculations only support "
            << "compressible, incompressible, or solid solver types."
            << exit(FatalError);
    }

    dataType_ = vector;
}

// Calculate viscous force
Foam::tmp<Foam::volSymmTensorField> preciceAdapter::FSI::ForceBase::devRhoReff() const
{
    // For turbulent flows

    if (mesh_.foundObject<Foam::compressible::turbulenceModel>("turbulenceProperties"))
    {
        const Foam::compressible::turbulenceModel& turb(
            mesh_.db().lookupObject<Foam::compressible::turbulenceModel>("turbulenceProperties"));

        return turb.devRhoReff();
    }
    else if (mesh_.foundObject<Foam::incompressible::turbulenceModel>("turbulenceProperties"))
    {
        const Foam::incompressible::turbulenceModel& turb(
            mesh_.db().lookupObject<Foam::incompressible::turbulenceModel>("turbulenceProperties"));

        return rho() * turb.devReff();
    }
    else
    {
        // For laminar flows get the velocity
        const Foam::volVectorField& U(
            mesh_.db().lookupObject<volVectorField>("U"));

        return -mu() * dev(twoSymm(fvc::grad(U)));
    }
}

// lookup correct rho
Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::ForceBase::rho() const
{
    // If volScalarField exists, read it from registry (for compressible cases)
    // interFoam is incompressible but has volScalarField rho

    if (mesh_.foundObject<Foam::volScalarField>("rho"))
    {
        return mesh_.db().lookupObject<Foam::volScalarField>("rho");
    }
    else if (solverType_.compare("incompressible") == 0)
    {
        const dictionary& FSIDict =
            mesh_.db().lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");

        return Foam::tmp<Foam::volScalarField>(
            new Foam::volScalarField(
                IOobject(
                    "rho",
                    mesh_.time().timeName(),
                    mesh_,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE),
                mesh_,
                Foam::dimensionedScalar(FSIDict.lookup("rho"))));
    }
    else
    {
        FatalErrorInFunction
            << "Did not find the correct rho."
            << exit(FatalError);

        return Foam::volScalarField::null();
    }
}

// lookup correct mu
Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::ForceBase::mu() const
{
    if (solverType_.compare("incompressible") == 0)
    {
        typedef twoPhaseMixture iitpMixture;
        if (mesh_.foundObject<iitpMixture>("mixture"))
        {
            const iitpMixture& mixture(
                mesh_.db().lookupObject<iitpMixture>("mixture"));

            return mixture.mu();
        }
        else
        {

            const dictionary& FSIDict =
                mesh_.db().lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");

	    Foam::dimensionedScalar nu(FSIDict.lookup("nu"));

            return tmp<Foam::volScalarField>(
                new Foam::volScalarField(
                    nu * rho()));
        }
    }
    else if (solverType_.compare("compressible") == 0)
    {
        return mesh_.db().lookupObject<Foam::volScalarField>("mu");
    }
    else
    {
        FatalErrorInFunction
            << "Did not find the correct mu."
            << exit(FatalError);

        return Foam::volScalarField::null();
    }
}

void preciceAdapter::FSI::ForceBase::writeToBuffer(double* buffer,
                                                   Foam::volVectorField& forceField,
                                                   const unsigned int dim) const
{
    // Compute forces. See the Forces function object.
    // Stress tensor boundary field
    tmp<Foam::volSymmTensorField> tdevRhoReff(devRhoReff());
    const Foam::volSymmTensorField::GeometricBoundaryField& devRhoReffb(
        tdevRhoReff().boundaryField());

    // Density boundary field
    tmp<Foam::volScalarField> trho(rho());
    const Foam::volScalarField::GeometricBoundaryField& rhob =
        trho().boundaryField();

    // Pressure boundary field
    const auto& pb = mesh_.db().lookupObject<Foam::volScalarField>("p").boundaryField();

    int bufferIndex = 0;
    // For every boundary patch of the interface
    for (const label patchID : patchIDs_)
    {
        tmp<Foam::vectorField> tsurface = getFaceVectors(patchID);
        const auto& surface = tsurface();

        // Pressure forces
        // FIXME: We need to subtract the reference pressure for incompressible calculations
        if (solverType_.compare("incompressible") == 0)
        {
            forceField.boundaryField()[patchID] =
                surface * pb[patchID] * rhob[patchID];
        }
        else if (solverType_.compare("compressible") == 0)
        {
            forceField.boundaryField()[patchID] =
                surface * pb[patchID];
        }
        else
        {
            FatalErrorInFunction
                << "Forces calculation does only support "
                << "compressible or incompressible solver type."
                << exit(FatalError);
        }

        // Viscous forces
        forceField.boundaryField()[patchID] +=
            surface & devRhoReffb[patchID];

        // Write the forces to the preCICE buffer
        // For every cell of the patch
        forAll(forceField.boundaryField()[patchID], i)
        {
            for (unsigned int d = 0; d < dim; ++d)
                buffer[bufferIndex++] =
                    forceField.boundaryField()[patchID][i][d];
        }
    }
}

void preciceAdapter::FSI::ForceBase::readFromBuffer(double* buffer) const
{
    /* TODO: Implement
     * We need two nested for-loops for each patch,
     * the outer for the locations and the inner for the dimensions.
     * See the preCICE readBlockVectorData() implementation.
     */
    FatalErrorInFunction
        << "Reading forces is not supported."
        << exit(FatalError);
}
