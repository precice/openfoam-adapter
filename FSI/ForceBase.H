/*---------------------------------------------------------------------------*\
    Copyright (C) 2017  Gerasimos Chourdakis
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
* Copyright (C) 2025 Gesellschaft fuer Anlagen- und Reaktorsicherheit         *
*                         (GRS) gGmbH                                         *
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM-preCICE adapter.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version with terms added by GRS.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License with terms by GRS for more details.

    You should have received a copy of the GNU General Public License
    with terms by GRS along with this program. If not, please
    contact your conveyor or GRS gGmbH.
    For a copy of the unmodified GNU General Public License, see
    <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef FORCEBASE_H
#define FORCEBASE_H


#include "CouplingDataUser.H"

#include "volFields.H"

#include "pointFields.H"
#include "vectorField.H"
#include "compressibleMomentumTransportModel.H"
#include "incompressibleMomentumTransportModel.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads force
class ForceBase : public CouplingDataUser
{
protected:
    //- Stress tensor (see the OpenFOAM "Forces" function object)
    Foam::tmp<Foam::surfaceVectorField> devTau() const;

    Foam::tmp<Foam::volScalarField> rho() const;

    Foam::tmp<Foam::volScalarField> mu() const;

    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;

    const std::string solverType_;

public:
    //- Constructor
    ForceBase(
        const Foam::fvMesh& mesh,
        const std::string solverType);

    std::size_t writeToBuffer(double* buffer,
                              Foam::volVectorField& forceField,
                              const unsigned int dim) const;

    void readFromBuffer(double* buffer) const;
};
}
}

#endif // FORCEBASE_H
