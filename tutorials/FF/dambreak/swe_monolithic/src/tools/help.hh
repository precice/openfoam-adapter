/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema
 * @author Sebastian Rettenberger
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
 *
 * TODO
 */

#ifndef __HELP_HH
#define __HELP_HH

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * class Float1D is a proxy class that can represent, for example, 
 * a column or row vector of a Float2D array, where row (sub-)arrays 
 * are stored with a respective stride. 
 * Besides constructor/deconstructor, the class provides overloading of 
 * the []-operator, such that elements can be accessed as v[i] 
 * (independent of the stride).
 * The class will never allocate separate memory for the vectors, 
 * but point to the interior data structure of Float2D (or other "host" 
 * data structures).
 */ 
class Float1D
{
  public:
	Float1D(float* _elem, int _rows, int _stride = 1) 
	: rows(_rows),stride(_stride),elem(_elem)
	{
	}

	~Float1D()
	{
	}

	inline float& operator[](int i) { 
		return elem[i*stride]; 
	}

	inline const float& operator[](int i) const {
		return elem[i*stride]; 
	}

	inline float* elemVector() {
		return elem;
	}

        inline int getSize() const { return rows; }; 

  private:
    int rows;
    int stride;
    float* elem;
};

/**
 * class Float2D is a very basic helper class to deal with 2D float arrays:
 * indices represent columns (1st index, "horizontal"/x-coordinate) and 
 * rows (2nd index, "vertical"/y-coordinate) of a 2D grid;
 * values are sequentially ordered in memory using "column major" order.
 * Besides constructor/deconstructor, the class provides overloading of 
 * the []-operator, such that elements can be accessed as a[i][j]. 
 */ 
class Float2D {
  public:
  	/**
     * Constructor:
	   * takes size of the 2D array as parameters and creates a respective Float2D object;
		 * allocates memory for the array, but does not initialise value.
     * @param _cols	number of columns (i.e., elements in horizontal direction)
     * @param _rows rumber of rows (i.e., elements in vertical directions)
     */
    Float2D(int _cols, int _rows, bool _allocateMemory = true):
      rows(_rows),
      cols(_cols),
      allocateMemory(_allocateMemory) {
      if (_allocateMemory) {
        elem = new float[rows*cols];
      }
	  }

    /**
     * Constructor:
		 * takes size of the 2D array as parameters and creates a respective Float2D object;
		 * this constructor does not allocate memory for the array, but uses the allocated memory 
		 * provided via the respective variable #_elem 
     * @param _cols	number of columns (i.e., elements in horizontal direction)
     * @param _rows rumber of rows (i.e., elements in vertical directions)
     * @param _elem pointer to a suitably allocated region of memory to be used for thew array elements
     */
    Float2D(int _cols, int _rows, float* _elem):
      rows(_rows),
      cols(_cols),
      allocateMemory(false) {
		  elem = _elem;
	  }


    /**
     * Constructor:
     * takes size of the 2D array as parameters and creates a respective Float2D object;
     * this constructor does not allocate memory for the array, but uses the allocated memory
     * provided via the respective variable #_elem
     * @param _cols number of columns (i.e., elements in horizontal direction)
     * @param _rows rumber of rows (i.e., elements in vertical directions)
     * @param _elem pointer to a suitably allocated region of memory to be used for thew array elements
     */
    Float2D(Float2D& _elem, bool shallowCopy):
      rows(_elem.rows),
      cols(_elem.cols),
      allocateMemory(!shallowCopy) {
      if (shallowCopy) {
        elem = _elem.elem;
        allocateMemory = false;
      }
      else {
        elem = new float[rows*cols];
        for (int i=0; i<rows*cols; i++) {
          elem[i] = _elem.elem[i];
        }
        allocateMemory = true;
      }
    }

	  ~Float2D() {
		  if (allocateMemory) {
		    delete[] elem;
		  }
  	}

	  inline float* operator[](int i) {
  		return (elem + (rows * i));
  	}

	  inline float const* operator[](int i) const {
  		return (elem + (rows * i));
  	}

	inline float* elemVector() {
		return elem;
	}

        inline int getRows() const { return rows; }; 
        inline int getCols() const { return cols; }; 

	inline Float1D getColProxy(int i) {
		// subarray elem[i][*]:
                // starting at elem[i][0] with rows elements and unit stride
		return Float1D(elem + (rows * i), rows);
	};
	
	inline Float1D getRowProxy(int j) {
		// subarray elem[*][j]
                // starting at elem[0][j] with cols elements and stride rows
		return Float1D(elem + j, cols, rows);
	};

  private:
    int rows;
    int cols;
    float* elem;
	bool allocateMemory;
};

//-------- Methods for Visualistion of Results --------

/**
 * generate output filenames for the single-SWE_Block version
 * (for serial and OpenMP-parallelised versions that use only a 
 *  single SWE_Block - one output file is generated per checkpoint)
 *
 *  @deprecated
 */
inline std::string generateFileName(std::string baseName, int timeStep) {

	std::ostringstream FileName;
	FileName << baseName <<timeStep<<".vtk";
	return FileName.str();
};

/**
 * Generates an output file name for a multiple SWE_Block version based on the ordering of the blocks.
 *
 * @param i_baseName base name of the output.
 * @param i_blockPositionX position of the SWE_Block in x-direction.
 * @param i_blockPositionY position of the SWE_Block in y-direction.
 * @param i_fileExtension file extension of the output file.
 * @return
 *
 * @deprecated
 */
inline std::string generateFileName( std::string i_baseName,
                                     int i_blockPositionX, int i_blockPositionY,
                                     std::string i_fileExtension=".nc" ) {

  std::ostringstream l_fileName;

  l_fileName << i_baseName << "_" << i_blockPositionX << i_blockPositionY << i_fileExtension;
  return l_fileName.str();
};

/**
 * generate output filename for the multiple-SWE_Block version
 * (for serial and parallel (OpenMP and MPI) versions that use 
 *  multiple SWE_Blocks - for each block, one output file is 
 *  generated per checkpoint)
 *
 *  @deprecated
 */
inline std::string generateFileName(std::string baseName, int timeStep, int block_X, int block_Y, std::string i_fileExtension=".vts") {

	std::ostringstream FileName;
	FileName << baseName <<"_"<< block_X<<"_"<<block_Y<<"_"<<timeStep<<i_fileExtension;
	return FileName.str();
};

/**
 * Generates an output file name for a multiple SWE_Block version based on the ordering of the blocks.
 *
 * @param i_baseName base name of the output.
 * @param i_blockPositionX position of the SWE_Block in x-direction.
 * @param i_blockPositionY position of the SWE_Block in y-direction.
 *
 * @return the output filename <b>without</b> timestep information and file extension
 */
inline
std::string generateBaseFileName(std::string &i_baseName, int i_blockPositionX , int i_blockPositionY)
{
	  std::ostringstream l_fileName;

	  l_fileName << i_baseName << "_" << i_blockPositionX << i_blockPositionY;
	  return l_fileName.str();
}

/**
 * generate output filename for the ParaView-Container-File
 * (to visualize multiple SWE_Blocks per checkpoint)
 */
inline std::string generateContainerFileName(std::string baseName, int timeStep) {

	std::ostringstream FileName;
	FileName << baseName<<"_"<<timeStep<<".pvts";
	return FileName.str();
};


#endif

