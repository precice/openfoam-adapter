/*---------------------------------------------------------------------------*\
Class
    Foam::coupledVelocityFvPatchField

Description
    This boundary condition is a velocity inlet-outlet.
    It acts as a fixedValue boundary patch for inflow faces and as a fixedGradient at outflow faces.
    It is essentially a type-specific version of the inletOutlet condition of OpenFOAM.

Usage
    \table
        Property     | Description             | Required    | Default value
        refValue        | reference Velocity values       | yes         |
        refGradient        | reference Velocity gradient values       | no         | uniform (0 0 0)
        phi        | name of flux field       | no         | "phi"
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            coupledVelocity;
        refValue        uniform (0 0 0);
    }
    \endverbatim

SourceFiles
    coupledVelocityFvPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef coupledVelocityFvPatchField_H
#define coupledVelocityFvPatchField_H

#include "fvPatchFields.H"
#include "fvCFD.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                 Class coupledVelocityFvPatchField Declaration
\*---------------------------------------------------------------------------*/

class coupledVelocityFvPatchField
: public fvPatchVectorField
{
private:
    // Private data

    //- Value field
    vectorField refValue_;

    //- Normal gradient field
    vectorField refGrad_;

    //- Fraction (0-1) of value used for boundary condition
    scalarField valueFraction_;

    //- Name of flux field
    word phiName_;

public:
    //- Runtime type information
    TypeName("coupledVelocity");


    // Constructors

    //- Construct from patch and internal field
    coupledVelocityFvPatchField(
        const fvPatch&,
        const DimensionedField<vector, volMesh>&);

    //- Construct from patch, internal field and dictionary
    coupledVelocityFvPatchField(
        const fvPatch&,
        const DimensionedField<vector, volMesh>&,
        const dictionary&);

    //- Construct by mapping the given coupledVelocityFvPatchField
    //  onto a new patch
    coupledVelocityFvPatchField(
        const coupledVelocityFvPatchField&,
        const fvPatch&,
        const DimensionedField<vector, volMesh>&,
        const fvPatchFieldMapper&);

    //- Construct as copy
    coupledVelocityFvPatchField(
        const coupledVelocityFvPatchField&);

    //- Construct and return a clone
    virtual tmp<fvPatchVectorField> clone() const
    {
        return tmp<fvPatchVectorField>(
            new coupledVelocityFvPatchField(*this));
    }

    //- Construct as copy setting internal field reference
    coupledVelocityFvPatchField(
        const coupledVelocityFvPatchField&,
        const DimensionedField<vector, volMesh>&);

    //- Construct and return a clone setting internal field reference
    virtual tmp<fvPatchField<vector>> clone(
        const DimensionedField<vector, volMesh>& iF) const
    {
        return tmp<fvPatchField<vector>>(
            new coupledVelocityFvPatchField(*this, iF));
    }


    // Member functions
    //- Return true if this patch field fixes a value.
    //  Needed to check if a level has to be specified while solving
    //  Poissons equations.
    virtual bool fixesValue() const
    {
        return true;
    }

    //- Return false: this patch field is not altered by assignment
    virtual bool assignable() const
    {
        return true;
    }

    // Evaluation functions

    //- Return neighbour coupled internal cell data
    virtual vectorField& refValue()
    {
        return refValue_;
    }

    virtual const vectorField& refValue() const
    {
        return refValue_;
    }

    virtual vectorField& refGrad()
    {
        return refGrad_;
    }

    virtual const vectorField& refGrad() const
    {
        return refGrad_;
    }

    virtual vectorField& gradient()
    {
        return refGrad_;
    }

    virtual const vectorField& gradient() const
    {
        return refGrad_;
    }

    virtual scalarField& valueFraction()
    {
        return valueFraction_;
    }

    virtual const scalarField& valueFraction() const
    {
        return valueFraction_;
    }

    //- Return gradient at boundary
    virtual tmp<vectorField> snGrad() const;

    void updateCoeffs();

    //- Evaluate the patch field
    virtual void evaluate(
        const Pstream::commsTypes commsType =
            Pstream::commsTypes::blocking);

    //- Return the matrix diagonal coefficients corresponding to the
    //  evaluation of the value of this patchField with given weights
    virtual tmp<Field<vector>> valueInternalCoeffs(
        const tmp<scalarField>&) const;

    //- Return the matrix source coefficients corresponding to the
    //  evaluation of the value of this patchField with given weights
    virtual tmp<Field<vector>> valueBoundaryCoeffs(
        const tmp<scalarField>&) const;

    //- Return the matrix diagonal coefficients corresponding to the
    //  evaluation of the gradient of this patchField
    virtual tmp<Field<vector>> gradientInternalCoeffs() const;

    //- Return the matrix source coefficients corresponding to the
    //  evaluation of the gradient of this patchField
    virtual tmp<Field<vector>> gradientBoundaryCoeffs() const;

    //- Write
    virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
