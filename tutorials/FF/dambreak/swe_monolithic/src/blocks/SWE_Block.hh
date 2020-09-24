/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema, Tobias Schnabel
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
 *
 * TODO
 */

#ifndef __SWE_BLOCK_HH
#define __SWE_BLOCK_HH

#include "tools/help.hh"
#include "scenarios/SWE_Scenario.hh"

#include <iostream>
#include <fstream>

using namespace std;

// forward declaration
class SWE_Block1D;

/**
 * SWE_Block is the main data structure to compute our shallow water model 
 * on a single Cartesian grid block:
 * SWE_Block is an abstract class (and interface) that should be extended 
 * by respective implementation classes.
 *
 * <h3>Cartesian Grid for Discretization:</h3>
 * 
 * SWE_Blocks uses a regular Cartesian grid of size #nx by #ny, where each 
 * grid cell carries three unknowns:
 * - the water level #h 
 * - the momentum components #hu and #hv (in x- and y- direction, resp.)
 * - the bathymetry #b
 * 
 * Each of the components is stored as a 2D array, implemented as a Float2D object, 
 * and are defined on grid indices [0,..,#nx+1]*[0,..,#ny+1]. 
 * The computational domain is indexed with [1,..,#nx]*[1,..,#ny].
 * 
 * The mesh sizes of the grid in x- and y-direction are stored in static variables 
 * #dx and #dy. The position of the Cartesian grid in space is stored via the 
 * coordinates of the left-bottom corner of the grid, in the variables 
 * #offsetX and #offsetY.
 * 
 * <h3>Ghost layers:</h3>
 * 
 * To implement the behaviour of the fluid at boundaries and for using 
 * multiple block in serial and parallel settings, SWE_Block adds an 
 * additional layer of so-called ghost cells to the Cartesian grid, 
 * as illustrated in the following figure.
 * Cells in the ghost layer have indices 0 or #nx+1 / #ny+1.
 *
 * \image html ghost_cells.gif
 * 
 * <h3>Memory Model:</h3>
 * 
 * The variables #h, #hu, #hv for water height and momentum will typically be  
 * updated by classes derived from SWE_Block. However, it is not assumed that 
 * such and updated will be performed in every time step. 
 * Instead, subclasses are welcome to update #h, #hu, and #hv in a lazy fashion, 
 * and keep data in faster memory (incl. local memory of acceleration hardware, 
 * such as GPGPUs), instead. 
 *
 * It is assumed that the bathymetry data #b is not changed during the algorithm
 * (up to the exceptions mentioned in the following). 
 * 
 * To force a synchronization of the respective data structures, the following 
 * methods are provided as part of SWE_Block:
 * - synchAfterWrite() to synchronize #h, #hu, #hv, and #b after an external update 
 *   (reading a file, e.g.);
 * - synchWaterHeightAfterWrite(), synchDischargeAfterWrite(), synchBathymetryAfterWrite():
 *   to synchronize only #h or momentum (#hu and #hv) or bathymetry #b;
 * - synchGhostLayerAfterWrite() to synchronize only the ghost layers
 * - synchBeforeRead() to synchronize #h, #hu, #hv, and #b before an output of the 
 *   variables (writing a visualization file, e.g.)
 * - synchWaterHeightBeforeRead(), synchDischargeBeforeRead(), synchBathymetryBeforeRead():
 *   as synchBeforeRead(), but only for the specified variables
 * - synchCopyLayerBeforeRead(): synchronizes the copy layer only (i.e., a layer that 
 *   is to be replicated in a neighbouring SWE_Block.
 *
 * <h3>Derived Classes</h3>
 *
 * As SWE_Block just provides an abstract base class together with the most 
 * important data structures, the implementation of concrete models is the 
 * job of respective derived classes (see the class diagram at the top of this 
 * page). Similar, parallel implementations that are based on a specific 
 * parallel programming model (such as OpenMP) or parallel architecture 
 * (such as GPU/CUDA) should form subclasses of their own. 
 * Please refer to the documentation of these classes for more details on the 
 * model and on the parallelisation approach.
 */
class SWE_Block {

  public:
  // object methods
    /// initialise unknowns to a specific scenario:
    void initScenario(float _offsetX, float _offsetY,
    		SWE_Scenario &i_scenario, const bool i_multipleBlocks = false );
    // set unknowns
    /// set the water height according to a given function
    void setWaterHeight(float (*_h)(float, float));
    /// set the momentum/discharge according to the provided functions
    void setDischarge(float (*_u)(float, float), float (*_v)(float, float));
    /// set the bathymetry to a uniform value
    void setBathymetry(float _b);
    /// set the bathymetry according to a given function
    void setBathymetry(float (*_b)(float, float));
    
    // read access to arrays of unknowns
    /// provides read access to the water height array 
    const Float2D& getWaterHeight();
    /// provides read access to the momentum/discharge array (x-component) 
    const Float2D& getDischarge_hu();
    /// provides read access to the momentum/discharge array (y-component) 
    const Float2D& getDischarge_hv();
    /// provides read access to the bathymetry data 
    const Float2D& getBathymetry();

    // defining boundary conditions
    /// set type of boundary condition for the specified boundary
    void setBoundaryType(BoundaryEdge edge, BoundaryType boundtype, 
                         const SWE_Block1D* inflow = NULL);
//     void connectBoundaries(BoundaryEdge edge, SWE_Block &neighbour, BoundaryEdge neighEdge);

    /// return a pointer to proxy class to access the copy layer
    virtual SWE_Block1D* registerCopyLayer(BoundaryEdge edge);
    /// "grab" the ghost layer in order to set these values externally
    virtual SWE_Block1D* grabGhostLayer(BoundaryEdge edge);
    
    /// set values in ghost layers
    void setGhostLayer();

   /// return maximum size of the time step to ensure stability of the method 
   /**
    * @return	current value of the member variable #maxTimestep 
    */
    float getMaxTimestep() { return maxTimestep; };
  
    // compute the largest allowed time step for the current grid block
    void computeMaxTimestep( const float i_dryTol = 0.1, const float i_cflNumber = 0.4 );

    /// execute a single time step (with fixed time step size) of the simulation
    virtual void simulateTimestep(float dt);

    /// perform the simulation starting with simulation time tStart,
    /// until simulation time tEnd is reached
    virtual float simulate(float tStart, float tEnd);
    
    /// compute the numerical fluxes for each edge of the Cartesian grid
    /**
     * The computation of fluxes strongly depends on the chosen numerical 
     * method. Hence, this purely virtual function has to be implemented 
     * in the respective derived classes.
     */
    virtual void computeNumericalFluxes() = 0;
    
    /// compute the new values of the unknowns h, hu, and hv in all grid cells
    /**
     * based on the numerical fluxes (computed by computeNumericalFluxes)
     * and the specified time step size dt, an Euler time step is executed.
     * As the computational fluxes will depend on the numerical method,
     * this purely virtual function has to be implemented separately for 
     * each specific numerical model (and parallelisation approach).  
     * @param dt	size of the time step
     */
    virtual void updateUnknowns(float dt) = 0;

    // access methods to grid sizes
    /// returns #nx, i.e. the grid size in x-direction 
    int getNx() { return nx; }
    /// returns #ny, i.e. the grid size in y-direction 
    int getNy() { return ny; }

  // Konstanten:
    /// static variable that holds the gravity constant (g = 9.81 m/s^2):
    static const float g;
	
  protected:
    // Constructor und Destructor
    SWE_Block(int l_nx, int l_ny,
    		float l_dx, float l_dy);
    virtual ~SWE_Block();

    // Sets the bathymetry on outflow and wall boundaries
    void setBoundaryBathymetry();

    // synchronization Methods
    virtual void synchAfterWrite();
    virtual void synchWaterHeightAfterWrite();
    virtual void synchDischargeAfterWrite();
    virtual void synchBathymetryAfterWrite();
    virtual void synchGhostLayerAfterWrite();

    virtual void synchBeforeRead();
    virtual void synchWaterHeightBeforeRead();
    virtual void synchDischargeBeforeRead();
    virtual void synchBathymetryBeforeRead();
    virtual void synchCopyLayerBeforeRead();
    
    /// set boundary conditions in ghost layers (set boundary conditions)
    virtual void setBoundaryConditions();

    // grid size: number of cells (incl. ghost layer in x and y direction:
    int nx;	///< size of Cartesian arrays in x-direction
    int ny;	///< size of Cartesian arrays in y-direction
    // mesh size dx and dy:
    float dx;	///<  mesh size of the Cartesian grid in x-direction
    float dy;	///<  mesh size of the Cartesian grid in y-direction

    // define arrays for unknowns: 
    // h (water level) and u,v (velocity in x and y direction)
    // hd, ud, and vd are respective CUDA arrays on GPU
    Float2D h;	///< array that holds the water height for each element
    Float2D hu; ///< array that holds the x-component of the momentum for each element (water height h multiplied by velocity in x-direction)
    Float2D hv; ///< array that holds the y-component of the momentum for each element (water height h multiplied by velocity in y-direction)
    Float2D b;  ///< array that holds the bathymetry data (sea floor elevation) for each element
    
    /// type of boundary conditions at LEFT, RIGHT, TOP, and BOTTOM boundary
    BoundaryType boundary[4];
    /// for CONNECT boundaries: pointer to connected neighbour block
    const SWE_Block1D* neighbour[4];

    /// maximum time step allowed to ensure stability of the method
    /**
     * maxTimestep can be updated as part of the methods computeNumericalFluxes
     * and updateUnknowns (depending on the numerical method)
     */
    float maxTimestep;

    // offset of current block
    float offsetX;	///< x-coordinate of the origin (left-bottom corner) of the Cartesian grid
    float offsetY;	///< y-coordinate of the origin (left-bottom corner) of the Cartesian grid
};

/**
 * SWE_Block1D is a simple struct that can represent a single line or row of 
 * SWE_Block unknowns (using the Float1D proxy class).
 * It is intended to unify the implementation of inflow and periodic boundary 
 * conditions, as well as the ghost/copy-layer connection between several SWE_Block
 * grids. 
 */ 
struct SWE_Block1D {
    SWE_Block1D(const Float1D& _h, const Float1D& _hu, const Float1D& _hv)
    : h(_h), hu(_hu), hv(_hv) {};
    SWE_Block1D(float* _h, float* _hu, float* _hv, int _size, int _stride=1)
    : h(_h,_size,_stride), hu(_hu,_size,_stride), hv(_hv,_size,_stride) {};
   
    Float1D h;
    Float1D hu;
    Float1D hv;
};


#endif
