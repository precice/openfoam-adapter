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

#include "Stress.H"

using namespace Foam;

preciceAdapter::FSI::Stress::Stress(
    const Foam::fvMesh& mesh,
    const std::string solverType)
: ForceBase(mesh, solverType)
{
    Stress_ = new volVectorField(
        IOobject(
            "Stress",
            mesh_.time().name(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE),
        mesh,
        dimensionedVector(
            "pdim",
            dimensionSet(1, -1, -2, 0, 0, 0, 0),
            Foam::vector::zero));
}

std::size_t preciceAdapter::FSI::Stress::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    return this->writeToBuffer(buffer, *Stress_, dim);
}

void preciceAdapter::FSI::Stress::read(double* buffer, const unsigned int dim)
{
    this->readFromBuffer(buffer);
}

bool preciceAdapter::FSI::Stress::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FSI::Stress::getDataName() const
{
    return "Stress";
}

preciceAdapter::FSI::Stress::~Stress()
{
    delete Stress_;
}
