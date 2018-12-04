#ifndef FSI_FORCE_H
#define FSI_FORCE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

// TEMPORARY
#include "pointFields.H"
#include "vectorField.H"
#include "coordinateSystem.H"
#include "volFieldsFwd.H"
#include "HashSet.H"

namespace preciceAdapter {
    namespace FSI {

//- Class that writes and reads force
        class Force : public CouplingDataUser {

        private:

            //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
            const Foam::fvMesh &mesh_;

            //- Force field
            Foam::volVectorField *Force_;

            //- Patches to integrate forces over
            labelHashSet patchSet_;

            //- Name of pressure field
            word pName_;

            //- Name of velocity field
            word UName_;

            //- Name of density field (optional)
            word rhoName_;

            //- Is the force density being supplied directly?
            Switch directForceDensity_;

            //- The name of the force density (fD) field
            word fDName_;

            //- Reference density needed for incompressible calculations
            Foam::scalar rhoRef_;

            //- Reference pressure
            Foam::scalar pRef_;

            //- Coordinate system used when evaluating forces/moments
            coordinateSystem coordSys_;

            //- Flag to indicate whether we are using a local co-ordinate sys
            bool localSystem_;

            //- Flag to include porosity effects
            bool porosity_;

            //- Initialised flag
            bool initialised_;

            //- Stress tensor (see the OpenFOAM "Forces" function object)
            Foam::tmp <Foam::volSymmTensorField> devRhoReff(dimensionedScalar rho) const;

            //- Return the effective viscous stress (laminar + turbulent).
            Foam::tmp <Foam::volSymmTensorField> devRhoReff() const;

            //- Dynamic viscosity field
            Foam::tmp <Foam::volScalarField> mu() const;

            //- Return rho if specified otherwise rhoRef
            Foam::tmp <Foam::volScalarField> rho() const;

            //- Return rhoRef if the pressure field is dynamic, i.e. p/rho
            //  otherwise return 1
            Foam::scalar rho(const volScalarField &p) const;

            //- Add patch contributions to force and moment fields
            void addToFields(
                const label patchi,
                const vectorField &fN,
                const vectorField &fT,
                const vectorField &fP);

            //- Add cell contributions to force and moment fields
            void addToFields(
                const labelList &cellIDs,
                const vectorField &fN,
                const vectorField &fT,
                const vectorField &fP);

          public:

            //- Constructor
            Force
                    (
                            const Foam::fvMesh &mesh,
                            const fileName &timeName // We create an IOobject and we need the time directory
                            /* TODO: We should add any required field names here.
                            /  They would need to be vector fields.
                            /  See CHT/Temperature.H for details.
                            /  Apply any changes also to Force.C.
                            */
                    );


            //- Calculate the forces and moments
            virtual void calcForcesMoment();

            //- Write the displacement values into the buffer
            void write(double *buffer);

            //- Read the displacement values from the buffer
            void read(double *buffer);

            //- Destructor
            ~Force();

        };

    }
}

#endif
