/**
 * @file
 * This file is part of SWE.
 *
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 */

#include <cassert>
#include <fstream>
#include "VtkWriter.hh"

/**
 * Creates a vtk file for each time step.
 * Any existing file will be replaced.
 *
 * @param i_baseName base name of the netCDF-file to which the data will be written to.
 * @param i_nX number of cells in the horizontal direction.
 * @param i_nY number of cells in the vertical direction.
 * @param i_dX cell size in x-direction.
 * @param i_dY cell size in y-direction.
 * @param i_offsetX x-offset of the block
 * @param i_offsetY y-offset of the block
 * @param i_dynamicBathymetry
 *
 * @todo This version can only handle a boundary layer of size 1
 */
io::VtkWriter::VtkWriter( const std::string &i_baseName,
		const Float2D &i_b,
		const BoundarySize &i_boundarySize,
		int i_nX, int i_nY,
		float i_dX, float i_dY,
		int i_offsetX, int i_offsetY) :
  io::Writer(i_baseName, i_b, i_boundarySize, i_nX, i_nY),
  dX(i_dX), dY(i_dY),
  offsetX(i_offsetX), offsetY(i_offsetY)
{
}

void io::VtkWriter::writeTimeStep(
		const Float2D &i_h,
        const Float2D &i_hu,
        const Float2D &i_hv,
        float i_time)
{
	std::ofstream vtkFile(generateFileName().c_str());
	assert(vtkFile.good());

	// VTK header
	vtkFile << "<?xml version=\"1.0\"?>" << std::endl
			<< "<VTKFile type=\"StructuredGrid\">" << std::endl
			<< "<StructuredGrid WholeExtent=\"" << offsetX << " " << offsetX+nX
				<< " " << offsetY << " " << offsetY+nY << " 0 0\">" << std::endl
	        << "<Piece Extent=\"" << offsetX << " " << offsetX+nX
	        	<< " " << offsetY << " " << offsetY+nY << " 0 0\">" << std::endl;

	vtkFile << "<Points>" << std::endl
			<< "<DataArray NumberOfComponents=\"3\" type=\"Float32\" format=\"ascii\">" << std::endl;

	//Grid points
	for (int j=0; j < nY+1; j++)
	      for (int i=0; i < nX+1; i++)
	    	  vtkFile << (offsetX+i)*dX << " " << (offsetY+j)*dY <<" 0" << std::endl;

	vtkFile << "</DataArray>" << std::endl
			<< "</Points>" << std::endl;

	vtkFile << "<CellData>" << std::endl;

	// Water surface height h
	vtkFile << "<DataArray Name=\"h\" type=\"Float32\" format=\"ascii\">" << std::endl;
	for (int j=1; j < nY+1; j++)
		for (int i=1; i < nX+1; i++)
			vtkFile << i_h[i][j] << std::endl;
	vtkFile << "</DataArray>" << std::endl;

	// Momentums
	vtkFile << "<DataArray Name=\"hu\" type=\"Float32\" format=\"ascii\">" << std::endl;
	for (int j=1; j < nY+1; j++)
		for (int i=1; i < nX+1; i++)
			vtkFile << i_hu[i][j] << std::endl;
	vtkFile << "</DataArray>" << std::endl;

	vtkFile << "<DataArray Name=\"hv\" type=\"Float32\" format=\"ascii\">" << std::endl;
	for (int j=1; j < nY+1; j++)
		for (int i=1; i<nX+1; i++)
			vtkFile << i_hv[i][j] << std::endl;
	vtkFile << "</DataArray>" << std::endl;

	// Bathymetry
	vtkFile << "<DataArray Name=\"b\" type=\"Float32\" format=\"ascii\">" << std::endl;
	for (int j=1; j<nY+1; j++)
		for (int i=1; i<nX+1; i++)
			vtkFile << b[i][j] << std::endl;
	vtkFile << "</DataArray>" << std::endl;

	vtkFile << "</CellData>" << std::endl
			<< "</Piece>" << std::endl;

	vtkFile << "</StructuredGrid>" << std::endl
			<< "</VTKFile>" << std::endl;

	// Increament time step
	timeStep++;
}
