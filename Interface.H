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

#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include "surfaceFields.H"
#include "argList.H"
#include "timeSelector.H"
#include "findRefCell.H"
#include "CouplingDataUser.H"
#include <precice/precice.hpp>

#include "pointPatchField.H"

namespace preciceAdapter
{

class Interface
{
protected:
    //- preCICE solver interface
    precice::Participant& precice_;

    //- Mesh name used in the preCICE configuration
    std::string meshName_;

    //- Point locations type (faceCenters vs faceNodes)
    LocationType locationType_ {LocationType::none};

    //- Mesh ID assigned by preCICE to the interface
    int meshID_;

    //- Names of the OpenFOAM patches that form the interface
    std::vector<std::string> patchNames_;

    //- OpenFOAM patches that form the interface
    std::vector<int> patchIDs_;

    //- Names of the OpenFOAM cell sets to be coupled (for volume coupling)
    std::vector<std::string> cellSetNames_;

    //- Number of data points (cell centers) on the interface
    int numDataLocations_ = 0;

    //- Vertex IDs assigned by preCICE
    std::vector<int> vertexIDs_;

    //- Buffer for the coupling data
    std::vector<double> dataBuffer_;

    //- Vector of CouplingDataReaders
    std::vector<CouplingDataUser*> couplingDataReaders_;

    //- Vector of CouplingDataWriters
    std::vector<CouplingDataUser*> couplingDataWriters_;

    //Switch for faceTriangulation (nearest projection)
    bool meshConnectivity_;

    //- Reset the displacement during interface definition
    bool restartFromDeformed_;

    // Simulation dimension
    unsigned int dim_;

    //- Extracts the locations of the face centers or face nodes
    //  and exposes them to preCICE with setMeshVertices
    void configureMesh(const Foam::fvMesh& mesh,
                       const std::string& namePointDisplacement,
                       const std::string& nameCellDisplacement);

public:
    //- Constructor
    Interface(
        precice::Participant& precice,
        const Foam::fvMesh& mesh,
        std::string meshName,
        std::string locationsType,
        std::vector<std::string> patchNames,
        std::vector<std::string> cellSetNames,
        bool meshConnectivity,
        bool restartFromDeformed,
        const std::string& namePointDisplacement,
        const std::string& nameCellDisplacement);

    //- Add a CouplingDataUser to read data from the interface
    void addCouplingDataReader(
        std::string dataName,
        CouplingDataUser* couplingDataReader);


    //- Add a CouplingDataUser to write data on the interface
    void addCouplingDataWriter(
        std::string dataName,
        CouplingDataUser* couplingDataWriter);

    //- Allocate an appropriate buffer for scalar or vector data.
    //  If at least one couplingDataUser has vector data, then
    //  define a buffer for 3D data. Otherwise, for 1D data.
    void createBuffer();

    //- Call read() on each registered couplingDataReader to read the coupling
    //  data from the buffer and apply the boundary conditions
    void readCouplingData(double relativeReadTime);

    //- Call write() on each registered couplingDataWriter to extract the boundary
    //  data and write them into the buffer
    void writeCouplingData();

    //- Destructor
    ~Interface();
};

}

#endif
