#include "ForceBase.H"

using namespace Foam;


preciceAdapter::FSI::ForceBase::ForceBase(
    const Foam::fvMesh& mesh,
    const std::string solverType)
: mesh_(mesh),
  solverType_(solverType)
{
    //What about type "basic"?
    if (solverType_.compare("incompressible") != 0 && solverType_.compare("compressible") != 0)
    {
        FatalErrorInFunction
            << "Force based calculations only support "
            << "compressible or incompressible solver types."
            << exit(FatalError);
    }

    dataType_ = vector;
}

//Calculate viscous force
Foam::tmp<Foam::volSymmTensorField> preciceAdapter::FSI::ForceBase::devRhoReff() const
{
    //For turbulent flows
    typedef compressible::turbulenceModel cmpTurbModel;
    typedef incompressible::turbulenceModel icoTurbModel;

    if (mesh_.foundObject<cmpTurbModel>(cmpTurbModel::propertiesName))
    {
        const cmpTurbModel& turb(
            mesh_.lookupObject<cmpTurbModel>(cmpTurbModel::propertiesName));

        return turb.devRhoReff();
    }
    else if (mesh_.foundObject<icoTurbModel>(icoTurbModel::propertiesName))
    {
        const incompressible::turbulenceModel& turb(
            mesh_.lookupObject<icoTurbModel>(icoTurbModel::propertiesName));

        return rho() * turb.devReff();
    }
    else
    {
        // For laminar flows get the velocity
        const volVectorField& U(
            mesh_.lookupObject<volVectorField>("U"));

        return -mu() * dev(twoSymm(fvc::grad(U)));
    }
}

//lookup correct rho
Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::ForceBase::rho() const
{
    // If volScalarField exists, read it from registry (for compressible cases)
    // interFoam is incompressible but has volScalarField rho

    if (mesh_.foundObject<volScalarField>("rho"))
    {
        return mesh_.lookupObject<volScalarField>("rho");
    }
    else if (solverType_.compare("incompressible") == 0)
    {
        const dictionary& FSIDict =
            mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");

        return tmp<volScalarField>(
            new volScalarField(
                IOobject(
                    "rho",
                    mesh_.time().timeName(),
                    mesh_,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE),
                mesh_,
                dimensionedScalar(FSIDict.get<dimensionedScalar>("rho"))));
    }
    else
    {
        FatalErrorInFunction
            << "Did not find the correct rho."
            << exit(FatalError);

        return volScalarField::null();
    }
}

//lookup correct mu
Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::ForceBase::mu() const
{

    if (solverType_.compare("incompressible") == 0)
    {
        typedef immiscibleIncompressibleTwoPhaseMixture iitpMixture;
        if (mesh_.foundObject<iitpMixture>("mixture"))
        {
            const iitpMixture& mixture(
                mesh_.lookupObject<iitpMixture>("mixture"));

            return mixture.mu();
        }
        else
        {

            const dictionary& FSIDict =
                mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");

            dimensionedScalar nu(FSIDict.get<dimensionedScalar>("nu"));

            return tmp<volScalarField>(
                new volScalarField(
                    nu * rho()));
        }
    }
    else if (solverType_.compare("compressible") == 0)
    {
        return mesh_.lookupObject<volScalarField>("thermo:mu");
    }
    else
    {
        FatalErrorInFunction
            << "Did not find the correct mu."
            << exit(FatalError);

        return volScalarField::null();
    }
}

void preciceAdapter::FSI::ForceBase::writeToBuffer(double* buffer,
                                                   volVectorField& forceField,
                                                   const unsigned int dim) const
{
    // Compute forces. See the Forces function object.
    // Stress tensor boundary field
    tmp<volSymmTensorField> tdevRhoReff(devRhoReff());
    const volSymmTensorField::Boundary& devRhoReffb(
        tdevRhoReff().boundaryField());

    // Density boundary field
    tmp<volScalarField> trho(rho());
    const volScalarField::Boundary& rhob =
        trho().boundaryField();

    // Pressure boundary field
    tmp<volScalarField> tp = mesh_.lookupObject<volScalarField>("p");
    const volScalarField::Boundary& pb(
        tp().boundaryField());

    // For every boundary patch of the interface
    for (const label patchID : patchIDs_)
    {

        const auto& surface = getFaceVectors(patchID);

        // Pressure forces
        // FIXME: We need to substract the reference pressure for incompressible calculations
        if (solverType_.compare("incompressible") == 0)
        {
            forceField.boundaryFieldRef()[patchID] =
                surface * pb[patchID] * rhob[patchID];
        }
        else if (solverType_.compare("compressible") == 0)
        {
            forceField.boundaryFieldRef()[patchID] =
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
        forceField.boundaryFieldRef()[patchID] +=
            surface & devRhoReffb[patchID];

        // Write the forces to the preCICE buffer
        // For every cell of the patch
        forAll(forceField.boundaryField()[patchID], i)
        {
            for (unsigned int d = 0; d < dim; ++d)
                buffer[i * dim + d] =
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
