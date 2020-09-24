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

#ifndef VTKWRITER_HH_
#define VTKWRITER_HH_

#include <sstream>
#include "writer/Writer.hh"

namespace io {
	class VtkWriter;
}

class io::VtkWriter : public io::Writer
{
private:
	//! cell size
	float dX, dY;

	float offsetX, offsetY;

public:
	VtkWriter( const std::string &i_fileName,
			   const Float2D &i_b,
			   const BoundarySize &i_boundarySize,
			   int i_nX, int i_nY,
			   float i_dX, float i_dY,
			   int i_offsetX = 0, int i_offsetY = 0);

    // writes the unknowns at a given time step to a vtk file
    void writeTimeStep( const Float2D &i_h,
                        const Float2D &i_hu,
                        const Float2D &i_hv,
                        float i_time);

private:
    std::string generateFileName()
    {
    	std::ostringstream name;

    	name << fileName << '.' << timeStep << ".vts";
    	return name.str();
    }
};

#endif // VTKWRITER_HH_
