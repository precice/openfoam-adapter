#include "Force.H"
#include "fvcGrad.H"
#include "porosityModel.H"
#include "turbulentTransportModel.H"
#include "turbulentFluidThermoModel.H"
#include "addToRunTimeSelectionTable.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::FSI::Force::Force
(
    const Foam::fvMesh& mesh,
    const fileName& timeName
    /* TODO: We should add any required field names here.
    /  They would need to be vector fields.
    /  See CHT/Temperature.C for details.
    */
)
:
mesh_(mesh)
{
    dataType_ = vector;

    // TODO: Is this ok?
    Force_ = new volVectorField
    (
        IOobject
        (
            "Force",
            timeName,
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "fdim",
            dimensionSet(1,1,-2,0,0,0,0),
            Foam::vector::zero
        )
    );
}


Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::Force::rho() const
{
    if (rhoName_ == "rhoInf")
    {
        const dictionary &transportProperties = mesh_.lookupObject<dictionary>("transportProperties");
        dimensionedScalar rho = transportProperties.lookup("rho");

        return tmp<volScalarField>(
            new volScalarField(
                IOobject(
                    "rho",
                    mesh_.time().timeName(),
                    mesh_),
                mesh_,
                dimensionedScalar("rho", dimDensity, rho.value())));
    }
    else
    {
        return (mesh_.lookupObject<volScalarField>(rhoName_));
    }
}

Foam::scalar preciceAdapter::FSI::Force::rho(const volScalarField &p) const
{
    if (p.dimensions() == dimPressure)
    {
        return 1.0;
    }
    else
    {
        if (rhoName_ != "rhoInf")
        {
            FatalErrorInFunction
                << "Dynamic pressure is expected but kinematic is provided."
                << exit(FatalError);
        }

        const dictionary &transportProperties = mesh_.lookupObject<dictionary>("transportProperties");
        dimensionedScalar rho = transportProperties.lookup("rho");
        return rho.value();
    }
}

Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::Force::mu() const
{
    if (mesh_.foundObject<fluidThermo>(basicThermo::dictName))
    {
        const fluidThermo &thermo = mesh_.lookupObject<fluidThermo>(basicThermo::dictName);

        return thermo.mu();
    }
    else if (mesh_.foundObject<transportModel>("transportProperties"))
    {
        const transportModel &laminarT = mesh_.lookupObject<transportModel>("transportProperties");

        return rho() * laminarT.nu();
    }
    else if (mesh_.foundObject<dictionary>("transportProperties"))
    {
        const dictionary &transportProperties = mesh_.lookupObject<dictionary>("transportProperties");

        dimensionedScalar nu("nu", dimViscosity, transportProperties.lookup("nu"));

        return rho() * nu;
    }
    else
    {
        FatalErrorInFunction
            << "No valid model for dynamic viscosity calculation"
            << exit(FatalError);

        return volScalarField::null();
    }
}


Foam::tmp<Foam::volSymmTensorField> preciceAdapter::FSI::Force::devRhoReff()
{
    typedef compressible::turbulenceModel cmpTurbModel;
    typedef incompressible::turbulenceModel icoTurbModel;
    const volVectorField &U = mesh_.lookupObject<volVectorField>("U");

    if (mesh_.foundObject<cmpTurbModel>(cmpTurbModel::propertiesName))
    {
        DEBUG(adapterInfo("Using a compressible turbulent solver"));

        const cmpTurbModel &turb = mesh_.lookupObject<cmpTurbModel>(cmpTurbModel::propertiesName);

        return turb.devRhoReff();
    }
    else if (mesh_.foundObject<icoTurbModel>(icoTurbModel::propertiesName))
    {
        DEBUG(adapterInfo("Using a incompressible tubulent solver"));

        const incompressible::turbulenceModel &turb = mesh_.lookupObject<icoTurbModel>(icoTurbModel::propertiesName);
        rhoName_ = "rhoInf";

        return rho() * turb.devReff();
    }
    else if (mesh_.foundObject<fluidThermo>(fluidThermo::dictName))
    {
        DEBUG(adapterInfo("Using a thermophysical solver")); // TODO is this true?

        const fluidThermo &thermo = mesh_.lookupObject<fluidThermo>(fluidThermo::dictName);

        return -thermo.mu() * dev(twoSymm(fvc::grad(U)));
    }
    else if (mesh_.foundObject<transportModel>("transportProperties"))
    {
        DEBUG(adapterInfo("Using a compressible solver")); // TODO is this true?

        const transportModel &laminarT = mesh_.lookupObject<transportModel>("transportProperties");

        return -rho() * laminarT.nu() * dev(twoSymm(fvc::grad(U)));
    }
    else if (mesh_.foundObject<dictionary>("transportProperties"))
    {
        DEBUG(adapterInfo("Using a incompressible solver"));

        const dictionary &transportProperties = mesh_.lookupObject<dictionary>("transportProperties");
        rhoName_ = "rhoInf";

        dimensionedScalar nu(
            "nu",
            dimViscosity,
            transportProperties.lookup("nu"));

        return -rho() * nu * dev(twoSymm(fvc::grad(U)));
    }
    else
    {
        FatalErrorInFunction
            << "No valid model for viscous stress calculation"
            << exit(FatalError);

        return volSymmTensorField::null();
    }
}

void preciceAdapter::FSI::Force::write(double *buffer)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE writeBlockVectorData() implementation.
    */
    // Compute forces. See the Forces function object.
    // Normal vectors on the boundary, multiplied with the face areas
    const surfaceVectorField::Boundary &Sfb = mesh_.Sf().boundaryField();

    // Stress tensor
    tmp<volSymmTensorField> tdevRhoReff = devRhoReff();

    // Stress tensor boundary field
    const volSymmTensorField::Boundary &devRhoReffb = tdevRhoReff().boundaryField();

    // Pressure
    const volScalarField &p = mesh_.lookupObject<volScalarField>("p");

    // TODO Scale pRef by density for incompressible simulations
    /* Reference pressure, 0 by default
    pRef_ = mesh_.lookupOrDefault<scalar>("pRef", 0.0);

    scalar pRef = pRef_ / rho(p);
    */

    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Pressure forces
        // TODO: Extend to cover also compressible solvers
        Force_->boundaryFieldRef()[patchID] = Sfb[patchID] * p.boundaryField()[patchID] * rho(p); // -pRef

        // Viscous forces
        Force_->boundaryFieldRef()[patchID] += Sfb[patchID] & devRhoReffb[patchID];

        // Write the forces to the preCICE buffer
        // For every cell of the patch
        forAll(Force_->boundaryFieldRef()[patchID], i)
        {
            // Copy the force into the buffer
            // x-dimension
            buffer[bufferIndex++] = Force_->boundaryFieldRef()[patchID][i].x();

            // y-dimension
            buffer[bufferIndex++] = Force_->boundaryFieldRef()[patchID][i].y();

            // z-dimension
            buffer[bufferIndex++] = Force_->boundaryFieldRef()[patchID][i].z();
        }
    }
}

void preciceAdapter::FSI::Force::read(double *buffer)
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

preciceAdapter::FSI::Force::~Force()
{
    // TODO: Is this enough?
    delete Force_;
}
