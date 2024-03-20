/*---------------------------------------------------------------------------*\
Class
    Foam::coupledPressureFvPatchField

Description
    This boundary condition is a pressure inlet-outlet condition.
    It acts as a fixedFluxExtrapolated pressure boundary patch for inflow faces and as a fixedValue at outflow faces.

Usage
    \table
        Property     | Description             | Required    | Default value
        refValue        | reference Pressure values       | yes         |
        refGradient        | reference Pressure gradient values       | no         | uniform 0
        phi        | name of flux field       | no         | "phi"
        U        | name of velocity field       | no         | "U"
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            coupledPressure;
        refValue           uniform 0;  // Example for Foam::scalar field usage
    }
    \endverbatim

SourceFiles
    coupledPressureFvPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef coupledPressureFvPatchField_H
#define coupledPressureFvPatchField_H

#include "fvPatchField.H"
#include "fixedFluxExtrapolatedPressureFvPatchScalarField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                   Class coupledPressureFvPatchField Declaration
\*---------------------------------------------------------------------------*/

class coupledPressureFvPatchField
: public fixedFluxExtrapolatedPressureFvPatchScalarField
{

private:
    //- Value field
    scalarField refValue_;

    //- Normal gradient field
    scalarField refGrad_;

    //- Fraction (0-1) of value used for boundary condition
    scalarField valueFraction_;

    //- Name of flux field
    word phiName_;

    //- Name of velocity field
    word uName_;

public:
    //- Runtime type information
    TypeName("coupledPressure");


    // Constructors

    //- Construct from patch and internal field
    coupledPressureFvPatchField(
        const fvPatch&,
        const DimensionedField<Foam::scalar, volMesh>&);

    //- Construct from patch, internal field and dictionary
    coupledPressureFvPatchField(
        const fvPatch&,
        const DimensionedField<Foam::scalar, volMesh>&,
        const dictionary&,
        const bool valueRequired = true);

    //- Construct by mapping the given coupledPressureFvPatchField<Type>
    //  onto a new patch
    coupledPressureFvPatchField(
        const coupledPressureFvPatchField&,
        const fvPatch&,
        const DimensionedField<Foam::scalar, volMesh>&,
        const fvPatchFieldMapper&);

    //- Construct as copy
    coupledPressureFvPatchField(
        const coupledPressureFvPatchField&);

    //- Construct and return a clone
    virtual tmp<fvPatchScalarField> clone() const
    {
        return tmp<fvPatchScalarField>(
            new coupledPressureFvPatchField(*this));
    }

    //- Construct as copy setting internal field reference
    coupledPressureFvPatchField(
        const coupledPressureFvPatchField&,
        const DimensionedField<Foam::scalar, volMesh>&);

    //- Construct and return a clone setting internal field reference
    virtual tmp<fvPatchScalarField> clone(
        const DimensionedField<Foam::scalar, volMesh>& iF) const
    {
        return tmp<fvPatchScalarField>(
            new coupledPressureFvPatchField(*this, iF));
    }


    // Member functions


    // Attributes

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
        return false;
    }

    // Evaluation functions
    void evaluate(const Pstream::commsTypes commsType =
                      Pstream::commsTypes::blocking);
    //- Return the matrix diagonal coefficients corresponding to the
    //  evaluation of the value of this patchField with given weights
    virtual tmp<Foam::scalarField> valueInternalCoeffs(
        const tmp<scalarField>&) const;

    //- Return the matrix source coefficients corresponding to the
    //  evaluation of the value of this patchField with given weights
    virtual tmp<Foam::scalarField> valueBoundaryCoeffs(
        const tmp<scalarField>&) const;

    //- Return the matrix diagonal coefficients corresponding to the
    //  evaluation of the gradient of this patchField
    virtual tmp<Foam::scalarField> gradientInternalCoeffs() const;

    //- Return the matrix source coefficients corresponding to the
    //  evaluation of the gradient of this patchField
    virtual tmp<Foam::scalarField> gradientBoundaryCoeffs() const;

    // Return defining fields

    virtual Foam::scalarField& refValue()
    {
        return refValue_;
    }

    virtual const Foam::scalarField& refValue() const
    {
        return refValue_;
    }

    virtual Foam::scalarField& refGrad()
    {
        return refGrad_;
    }

    virtual const Foam::scalarField& refGrad() const
    {
        return refGrad_;
    }

    virtual Foam::scalarField& gradient()
    {
        return refGrad_;
    }

    virtual const Foam::scalarField& gradient() const
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
    virtual tmp<Foam::scalarField> snGrad() const;


    void updateCoeffs();

    //- Write
    virtual void write(Ostream&) const;


    // Member operators

    virtual void operator=(const UList<Foam::scalar>&) {}

    virtual void operator=(const fvPatchScalarField&) {}
    virtual void operator+=(const fvPatchScalarField&) {}
    virtual void operator-=(const fvPatchScalarField&) {}
    virtual void operator*=(const fvPatchScalarField&) {}
    virtual void operator/=(const fvPatchScalarField&) {}

    virtual void operator+=(const scalarField&) {}
    virtual void operator-=(const scalarField&) {}

    virtual void operator*=(const scalarField&) {}
    virtual void operator/=(const scalarField&) {}

    virtual void operator=(const Foam::scalar&) {}
    virtual void operator+=(const Foam::scalar&) {}
    virtual void operator-=(const Foam::scalar&) {}
    virtual void operator*=(const Foam::scalar) {}
    virtual void operator/=(const Foam::scalar) {}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// #ifdef NoRepository
//     #include "coupledPressureFvPatchField.C"
// #endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
