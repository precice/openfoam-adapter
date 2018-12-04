#include "Force.H"
#include "fvcGrad.H"
#include "porosityModel.H"
#include "turbulentTransportModel.H"
#include "turbulentFluidThermoModel.H"
#include "addToRunTimeSelectionTable.H"

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
    Force_ = new volVectorField(
        IOobject(
            "Force",
            timeName,
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE),
        mesh,
        dimensionedVector("fdim", dimForce, Zero));

}

Foam::tmp<Foam::volSymmTensorField> preciceAdapter::FSI::Force::devRhoReff() const
{
        typedef compressible::turbulenceModel cmpTurbModel;
        typedef incompressible::turbulenceModel icoTurbModel;

        if (mesh_.foundObject<cmpTurbModel>(cmpTurbModel::propertiesName))
        {
                const cmpTurbModel &turb =
                    mesh_.lookupObject<cmpTurbModel>(cmpTurbModel::propertiesName);

                return turb.devRhoReff();
        }
        else if (mesh_.foundObject<icoTurbModel>(icoTurbModel::propertiesName))
        {
                const incompressible::turbulenceModel &turb =
                    mesh_.lookupObject<icoTurbModel>(icoTurbModel::propertiesName);

                return rho() * turb.devReff();
        }
        else if (mesh_.foundObject<fluidThermo>(fluidThermo::dictName))
        {
                const fluidThermo &thermo =
                    mesh_.lookupObject<fluidThermo>(fluidThermo::dictName);

                const volVectorField &U = mesh_.lookupObject<volVectorField>(UName_);

                return -thermo.mu() * dev(twoSymm(fvc::grad(U)));
        }
        else if (
            mesh_.foundObject<transportModel>("transportProperties"))
        {
                const transportModel &laminarT =
                    mesh_.lookupObject<transportModel>("transportProperties");

                const volVectorField &U = mesh_.lookupObject<volVectorField>(UName_);

                return -rho() * laminarT.nu() * dev(twoSymm(fvc::grad(U)));
        }
        else if (mesh_.foundObject<dictionary>("transportProperties"))
        {
                const dictionary &transportProperties =
                    mesh_.lookupObject<dictionary>("transportProperties");

                dimensionedScalar nu(
                    "nu",
                    dimViscosity,
                    transportProperties.lookup("nu"));

                const volVectorField &U = mesh_.lookupObject<volVectorField>(UName_);

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

Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::Force::mu() const
{
        if (mesh_.foundObject<fluidThermo>(basicThermo::dictName))
        {
                const fluidThermo &thermo =
                    mesh_.lookupObject<fluidThermo>(basicThermo::dictName);

                return thermo.mu();
        }
        else if (
            mesh_.foundObject<transportModel>("transportProperties"))
        {
                const transportModel &laminarT =
                    mesh_.lookupObject<transportModel>("transportProperties");

                return rho() * laminarT.nu();
        }
        else if (mesh_.foundObject<dictionary>("transportProperties"))
        {
                const dictionary &transportProperties =
                    mesh_.lookupObject<dictionary>("transportProperties");

                dimensionedScalar nu(
                    "nu",
                    dimViscosity,
                    transportProperties.lookup("nu"));

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

Foam::tmp<Foam::volScalarField> preciceAdapter::FSI::Force::rho() const
{
        if (rhoName_ == "rhoInf")
        {
                return tmp<volScalarField>(
                    new volScalarField(
                        IOobject(
                            "rho",
                            mesh_.time().timeName(),
                            mesh_),
                        mesh_,
                        dimensionedScalar("rho", dimDensity, rhoRef_)));
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

                return rhoRef_;
        }
}

void preciceAdapter::FSI::Force::addToFields(
    const label patchi,
    const vectorField &fN,
    const vectorField &fT,
    const vectorField &fP)
{
        volVectorField &force =
            const_cast<volVectorField &>(
                mesh_.lookupObject<volVectorField>("Force"));

        vectorField &pf = force.boundaryFieldRef()[patchi];
        pf += fN + fT + fP;

}

void preciceAdapter::FSI::Force::addToFields(
    const labelList &cellIDs,
    const vectorField &fN,
    const vectorField &fT,
    const vectorField &fP)
{
    volVectorField &force =
        const_cast<volVectorField &>(
            mesh_.lookupObject<volVectorField>("Force"));


    forAll(cellIDs, i)
    {
        label celli = cellIDs[i];
        force[celli] += fN[i] + fT[i] + fP[i];
        }
}

void preciceAdapter::FSI::Force::calcForcesMoment()
{
        Force_[0] = Foam::vector::zero;
        Force_[1] = Foam::vector::zero;
        Force_[2] = Foam::vector::zero;

        if (directForceDensity_)
        {
                const volVectorField &fD = mesh_.lookupObject<volVectorField>(fDName_);

                const surfaceVectorField::Boundary &Sfb =
                    mesh_.Sf().boundaryField();

                forAllConstIter(labelHashSet, patchSet_, iter)
                {
                        label patchi = iter.key();


                        scalarField sA(mag(Sfb[patchi]));

                        // Normal force = surfaceUnitNormal*(surfaceNormal & forceDensity)
                        vectorField fN(
                            Sfb[patchi] / sA * (Sfb[patchi] & fD.boundaryField()[patchi]));

                        // Tangential force (total force minus normal fN)
                        vectorField fT(sA * fD.boundaryField()[patchi] - fN);

                        //- Porous force
                        vectorField fP(fT.size(), Zero);

                        addToFields(patchi, fN, fT, fP);
                }
        }
        else
        {
                const volScalarField &p = mesh_.lookupObject<volScalarField>(pName_);

                const surfaceVectorField::Boundary &Sfb =
                    mesh_.Sf().boundaryField();

                tmp<volSymmTensorField> tdevRhoReff = devRhoReff();
                const volSymmTensorField::Boundary &devRhoReffb = tdevRhoReff().boundaryField();

                // Scale pRef by density for incompressible simulations
                Foam::scalar pRef = pRef_ / rho(p);

                forAllConstIter(labelHashSet, patchSet_, iter)
                {
                        label patchi = iter.key();

                        vectorField fN(
                            rho(p) * Sfb[patchi] * (p.boundaryField()[patchi] - pRef));

                        vectorField fT(Sfb[patchi] & devRhoReffb[patchi]);

                        vectorField fP(fT.size(), Zero);

                        addToFields(patchi, fN, fT, fP);
                }
        }

        if (porosity_)
        {
                const volVectorField &U = mesh_.lookupObject<volVectorField>(UName_);
                const volScalarField rho(this->rho());
                const volScalarField mu(this->mu());

                const HashTable<const porosityModel *> models =
                    mesh_.lookupClass<porosityModel>();

                if (models.empty())
                {
                        WarningInFunction
                            << "Porosity effects requested, but no porosity models found "
                            << "in the database"
                            << endl;
                }

                forAllConstIter(HashTable<const porosityModel *>, models, iter)
                {
                        // non-const access required if mesh is changing
                        porosityModel &pm = const_cast<porosityModel &>(*iter());

                        vectorField fPTot(pm.force(U, rho, mu));

                        const labelList &cellZoneIDs = pm.cellZoneIDs();

                        forAll(cellZoneIDs, i)
                        {
                                label zoneI = cellZoneIDs[i];
                                const cellZone &cZone = mesh_.cellZones()[zoneI];

                                const vectorField d(mesh_.C(), cZone);
                                const vectorField fP(fPTot, cZone);

                                const vectorField fDummy(fP.size(), Zero);

                                addToFields(cZone, fDummy, fDummy, fP);
                        }
                }
        }

}

void preciceAdapter::FSI::Force::write(double *buffer)
{
        calcForcesMoment();

        mesh_.lookupObject<volVectorField>("Force").write();

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
