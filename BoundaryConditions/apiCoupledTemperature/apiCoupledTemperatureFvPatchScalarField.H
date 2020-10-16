/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2019 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::apiCoupledTemperatureFvPatchScalarField

Group
    grpThermoBoundaryConditions grpWallBoundaryConditions

Description
    This boundary condition provides a base class for 'mixed' type boundary
    conditions, i.e. conditions that mix fixed value and patch-normal gradient
    conditions.

    The respective contributions from each is determined by a weight field:

        \f[
            x_p = w x_p + (1-w) \left(x_c + \frac{\nabla_\perp x}{\Delta}\right)
        \f]

    where
    \vartable
        x_p   | patch values
        x_c   | patch internal cell values
        \Delta| inverse distance from face centre to internal cell centre
        w     | weighting values (0-1)
    \endvartable


Usage
    \table
        Property         | Description                           | Required | Default
        mode             | operation mode (fixedTemperature | fixedHeatFlux | HTC ) | yes | -
        refValue         | fixed value                           | no       | value
        refGradient      | patch normal gradient                 | no       | 0.0
        valueFraction    | value weighting (0-1)                 | no       | 1.0
        value            | nothing, only for result plots        | no       | -
        relaxation       | relaxation factor                     | no       | 1.0
        qr               | Name of radiation field               | no       | -
        qrRelaxation     | relaxation factor of radiation field  | no       | 1.0
        initialHeatflux  | Initial heat flux in mode "flux"      | no       | 0.0
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            apiCoupledTemperature;
        mode            mixed;

        refValue        $internalField;
        refGradient     uniform 0;
        valueFraction   uniform 1;
        value           $internalField;

        //
        // @see temperatureCoupledBase for more informations
        //
        qr              ...;    // Name of radiation field

        kappaMethod     ...;    // Thermal conductivity method
        kappa           ...;    // Name of thermal conductivity field
        alphaAni        ...;    // Name of thermal diffusivity field
        alpha           ...;    // Name of non-isotropic alpha
    }
    \endverbatim

Note
    ...

See also
    Foam::mixedFvPatchScalarField

SourceFiles
    apiCoupledMixedFvPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef apiCoupledTemperatureFvPatchScalarField_H
#define apiCoupledTemperatureFvPatchScalarField_H

#include <iterator>

#include "fieldTypes.H"
#include "scalarField.H"
#include "mixedFvPatchFields.H"
#include "temperatureCoupledBase.H"
#include "PatchFunction1.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// forward declaration
template<class Type>
class Function1;

template<class T>
class autoPtr;

template<class EnumType>
class Enum;

// class declaration
class apiCoupledTemperatureFvPatchScalarField
:
    public mixedFvPatchScalarField,
    public temperatureCoupledBase
{
public:

    // Public Data

        //- Operation mode enumeration
        enum operationMode
        {
            fixedTemperature,
            fixedHeatFlux,
            fixedMixedTemperatureHTC,
        };

        static const Enum<operationMode> operationModeNames;


private:

    // Data

        //- Operation mode
        const operationMode mode_;

        // radiation field
        const word qrName_;

        //- Relaxation for the wall temperature (thermal inertia)
        scalar relaxation_;

        //- Cache qr for relaxation
        scalarField qrPrevious_;

        //- Relaxation for qr
        scalar qrRelaxation_;

        //- T_cell of neighbour cell
        scalarField T_neighbour_;

        //- heat transfer coefficient of neighbour cell
        scalarField h_neighbour_;

        //- heat flux
        scalarField heatflux_;


public:

    //- Runtime type information
    TypeName("apiCoupledTemperature");


    // Constructors

        //- Construct from patch and internal field
    	apiCoupledTemperatureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
    	apiCoupledTemperatureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  externalCoupledTemperatureMixedFvPatchScalarField onto a new patch
    	apiCoupledTemperatureFvPatchScalarField
        (
            const apiCoupledTemperatureFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
    	apiCoupledTemperatureFvPatchScalarField
        (
            const apiCoupledTemperatureFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchField<scalar>> clone() const
        {
            return tmp<fvPatchField<scalar>>
            (
                new apiCoupledTemperatureFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        apiCoupledTemperatureFvPatchScalarField
        (
            const apiCoupledTemperatureFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchField<scalar>> clone
        (
            const DimensionedField<scalar, volMesh>& iF
        ) const
        {
            return tmp<fvPatchField<scalar>>
            (
                new apiCoupledTemperatureFvPatchScalarField
                (
                    *this,
                    iF
                )
            );
        }


    //- Destructor
    virtual ~apiCoupledTemperatureFvPatchScalarField() = default;


    // Member Functions

        // Access

        //- Allow manipulation of the boundary values
        bool fixesValue() const override
        {
            return false;
        }


        // Mapping functions

        //- Map (and resize as needed) from self given a mapping object
        void autoMap
        (
            const fvPatchFieldMapper&
        ) override;

        //- Reverse map the given fvPatchField onto this fvPatchField
        void rmap
        (
            const fvPatchScalarField&,
            const labelList&
        ) override;


        // Evaluation functions

        //- Update the coefficients associated with the patch field
        void updateCoeffs() override;

    	//- Write
    	virtual void write(Ostream& os) const;

public:

        // calculate the wall heat flux on boundary face
        Foam::tmp<Foam::scalarField> getWallHeatFlux() const;

        // get the heat transfer coefficent on the wall side
        Foam::tmp<Foam::scalarField> getHeatTransferCoeff() const;

        //
        const scalarField & T_Wall() const;

        //
        Foam::tmp<Foam::scalarField> T_Cell() const;

        //
        scalarField& h_Neighbour();
        const scalarField& h_Neighbour() const;

        //
        scalarField& T_Neighbour();
        const scalarField& T_Neighbour() const;

        //
        scalarField& heatFlux();
        const scalarField& heatFlux() const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif
