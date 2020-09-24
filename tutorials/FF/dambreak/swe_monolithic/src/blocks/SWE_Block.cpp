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

#include "SWE_Block.hh"
#include "tools/help.hh"

#include <cmath>
#include <iostream>
#include <cassert>
#include <limits>

// gravitational acceleration
const float SWE_Block::g = 9.81f;

/**
 * Constructor: allocate variables for simulation
 *
 * unknowns h (water height), hu,hv (discharge in x- and y-direction), 
 * and b (bathymetry) are defined on grid indices [0,..,nx+1]*[0,..,ny+1]
 * -> computational domain is [1,..,nx]*[1,..,ny]
 * -> plus ghost cell layer
 *
 * The constructor is protected: no instances of SWE_Block can be 
 * generated.
 *
 */
SWE_Block::SWE_Block(int l_nx, int l_ny,
		float l_dx, float l_dy)
	: nx(l_nx), ny(l_ny),
	  dx(l_dx), dy(l_dy),
	  h(nx+2,ny+2), hu(nx+2,ny+2), hv(nx+2,ny+2), b(nx+2,ny+2),
	  // This three are only set here, so eclipse does not complain
	  maxTimestep(0), offsetX(0), offsetY(0)
{
  // set WALL as default boundary condition
  for (int i=0; i<4; i++) {
     boundary[i] = PASSIVE;
     neighbour[i] = NULL;
  };
}

/**
 * Destructor: de-allocate all variables
 */
SWE_Block::~SWE_Block() {
}

//==================================================================
// methods for external read/write to main variables h, hu, hv, and b
// Note: temporary and non-local variables depending on the main 
// variables are synchronised before/after their update or read
//==================================================================

/**
 * Initializes the unknowns and bathymetry in all grid cells according to the given SWE_Scenario.
 *
 * In the case of multiple SWE_Blocks at this point, it is not clear how the boundary conditions
 * should be set. This is because an isolated SWE_Block doesn't have any in information about the grid.
 * Therefore the calling routine, which has the information about multiple blocks, has to take care about setting
 * the right boundary conditions.
 * 
 * @param i_scenario scenario, which is used during the setup.
 * @param i_multipleBlocks are the multiple SWE_blocks?
 */
void SWE_Block::initScenario( float _offsetX, float _offsetY,
							  SWE_Scenario &i_scenario,
                              const bool i_multipleBlocks ) {
	offsetX = _offsetX;
	offsetY = _offsetY;

  // initialize water height and discharge
  for(int i=1; i<=nx; i++)
    for(int j=1; j<=ny; j++) {
      float x = offsetX + (i-0.5f)*dx;
      float y = offsetY + (j-0.5f)*dy;
      h[i][j] =  i_scenario.getWaterHeight(x,y);
      hu[i][j] = i_scenario.getVeloc_u(x,y) * h[i][j];
      hv[i][j] = i_scenario.getVeloc_v(x,y) * h[i][j]; 
    };

  // initialize bathymetry
  for(int i=0; i<=nx+1; i++) {
    for(int j=0; j<=ny+1; j++) {
      b[i][j] = i_scenario.getBathymetry( offsetX + (i-0.5f)*dx,
                                          offsetY + (j-0.5f)*dy );
    }
  }

  // in the case of multiple blocks the calling routine takes care about proper boundary conditions.
  if( i_multipleBlocks == false ) {
    // obtain boundary conditions for all four edges from scenario
    setBoundaryType(BND_LEFT, i_scenario.getBoundaryType(BND_LEFT));
    setBoundaryType(BND_RIGHT, i_scenario.getBoundaryType(BND_RIGHT));
    setBoundaryType(BND_BOTTOM, i_scenario.getBoundaryType(BND_BOTTOM));
    setBoundaryType(BND_TOP, i_scenario.getBoundaryType(BND_TOP));
  }

  // perform update after external write to variables 
  synchAfterWrite();

}

/**
 * set water height h in all interior grid cells (i.e. except ghost layer) 
 * to values specified by parameter function _h
 */
void SWE_Block::setWaterHeight(float (*_h)(float, float)) {

  for(int i=1; i<=nx; i++)
    for(int j=1; j<=ny; j++) {
      h[i][j] =  _h(offsetX + (i-0.5f)*dx, offsetY + (j-0.5f)*dy);
    };

  synchWaterHeightAfterWrite();
}

/**
 * set discharge in all interior grid cells (i.e. except ghost layer) 
 * to values specified by parameter functions
 * Note: unknowns hu and hv represent momentum, while parameters u and v are velocities! 
 */
void SWE_Block::setDischarge(float (*_u)(float, float), float (*_v)(float, float)) {

  for(int i=1; i<=nx; i++)
    for(int j=1; j<=ny; j++) {
      float x = offsetX + (i-0.5f)*dx;
      float y = offsetY + (j-0.5f)*dy;
      hu[i][j] = _u(x,y) * h[i][j];
      hv[i][j] = _v(x,y) * h[i][j]; 
    };

  synchDischargeAfterWrite();
}

/**
 * set Bathymetry b in all grid cells (incl. ghost/boundary layers)
 * to a uniform value
 * bathymetry source terms are re-computed
 */
void SWE_Block::setBathymetry(float _b) {

  for(int i=0; i<=nx+1; i++)
    for(int j=0; j<=ny+1; j++)
      b[i][j] = _b;

  synchBathymetryAfterWrite();
}

/**
 * set Bathymetry b in all grid cells (incl. ghost/boundary layers)
 * using the specified bathymetry function;
 * bathymetry source terms are re-computed
 */
void SWE_Block::setBathymetry(float (*_b)(float, float)) {

  for(int i=0; i<=nx+1; i++)
    for(int j=0; j<=ny+1; j++)
      b[i][j] = _b(offsetX + (i-0.5f)*dx, offsetY + (j-0.5f)*dy);

  synchBathymetryAfterWrite();
}

// /** 
// 	Restores values for h, v, and u from file data
// 	@param _b		array holding b-values in sequence
// */
// void SWE_Block::setBathymetry(float* _b) {
// 	// Set all inner cells to the value available
// 	int i, j;	
// 	for(int k=0; k<nx*ny; k++) {
// 		i = (k % ny) + 1;
// 		j = (k / ny) + 1;
// 		b[i][j] = _b[k];
// 	};
// 
// 	// Set ghost cells values such that normals = 0
// 	// Boundaries
// 	for(int i=1; i<=nx; i++) {
// 		b[0][i] = b[1][i];
// 		b[nx+1][i] = b[nx][i];
// 		b[i][0] = b[i][1];
// 		b[i][nx+1] = b[i][nx];
// 	}
// 	// Corners
// 	b[0][0] = b[1][1];
// 	b[0][ny+1] = b[1][ny];
// 	b[nx+1][0] = b[nx][1];
// 	b[nx+1][ny+1] = b[nx][ny];
// 
// 	synchBathymetryAfterWrite();
// }

/**
 * return reference to water height unknown h
 */
const Float2D& SWE_Block::getWaterHeight() { 
  synchWaterHeightBeforeRead();
  return h; 
};

/**
 * return reference to discharge unknown hu
 */
const Float2D& SWE_Block::getDischarge_hu() { 
  synchDischargeBeforeRead();
  return hu; 
};

/**
 * return reference to discharge unknown hv
 */
const Float2D& SWE_Block::getDischarge_hv() { 
  synchDischargeBeforeRead();
  return hv;
};

/**
 * return reference to bathymetry unknown b
 */
const Float2D& SWE_Block::getBathymetry() { 
  synchBathymetryBeforeRead();
  return b; 
};

//==================================================================
// methods for simulation
//==================================================================

/**
 * Executes a single timestep with fixed time step size
 *  * compute net updates for every edge
 *  * update cell values with the net updates
 *
 * @param dt	time step width of the update
 */
void
SWE_Block::simulateTimestep (float dt)
{
	computeNumericalFluxes ();
	updateUnknowns (dt);
}

/**
 * simulate implements the main simulation loop between two checkpoints;
 * Note: this implementation can only be used, if you only use a single SWE_Block
 *       and only apply simple boundary conditions! 
 *       In particular, SWE_Block::simulate can not trigger calls to exchange values 
 *       of copy and ghost layers between blocks!
 * @param	tStart	time where the simulation is started
 * @param	tEnd	time of the next checkpoint 
 * @return	actual	end time reached
 */
float
SWE_Block::simulate (float i_tStart, float i_tEnd)
{
	float t = i_tStart;
	do {
		//set values in ghost cells
		setGhostLayer ();

		// compute numerical fluxes for every edge
		// -> computeNumericalFluxes might update maxTimestep
		computeNumericalFluxes ();
		// update unknowns accordingly
		updateUnknowns (maxTimestep);
		t += maxTimestep;

		std::cout << "Simulation at time " << t << std::endl << std::flush;
	} while (t < i_tEnd);

	return t;
}

/**
 * Set the boundary type for specific block boundary.
 *
 * @param i_edge location of the edge relative to the SWE_block.
 * @param i_boundaryType type of the boundary condition.
 * @param i_inflow pointer to an SWE_Block1D, which specifies the inflow (should be NULL for WALL or OUTFLOW boundary)
 */
void SWE_Block::setBoundaryType( const BoundaryEdge i_edge,
                                 const BoundaryType i_boundaryType,
                                 const SWE_Block1D* i_inflow) {
	boundary[i_edge] = i_boundaryType;
	neighbour[i_edge] = i_inflow;

	if (i_boundaryType == OUTFLOW || i_boundaryType == WALL)
		// One of the boundary was changed to OUTFLOW or WALL
		// -> Update the bathymetry for this boundary
		setBoundaryBathymetry();
}

/**
 * Sets the bathymetry on OUTFLOW or WALL boundaries.
 * Should be called very time a boundary is changed to a OUTFLOW or
 * WALL boundary <b>or</b> the bathymetry changes.
 */
void SWE_Block::setBoundaryBathymetry()
{
	// set bathymetry values in the ghost layer, if necessary
	if( boundary[BND_LEFT] == OUTFLOW || boundary[BND_LEFT] == WALL ) {
		memcpy(b[0], b[1], sizeof(float)*(ny+2));
	}
	if( boundary[BND_RIGHT] == OUTFLOW || boundary[BND_RIGHT] == WALL ) {
		memcpy(b[nx+1], b[nx], sizeof(float)*(ny+2));
	}
	if( boundary[BND_BOTTOM] == OUTFLOW || boundary[BND_BOTTOM] == WALL ) {
		for(int i=0; i<=nx+1; i++) {
			b[i][0] = b[i][1];
		}
	}
	if( boundary[BND_TOP] == OUTFLOW || boundary[BND_TOP] == WALL ) {
		for(int i=0; i<=nx+1; i++) {
			b[i][ny+1] = b[i][ny];
		}
	}


	// set corner values
        b[0][0]       = b[1][1];
        b[0][ny+1]    = b[1][ny];
        b[nx+1][0]    = b[nx][1];
        b[nx+1][ny+1] = b[nx][ny];

	// synchronize after an external update of the bathymetry
	synchBathymetryAfterWrite();
}

/**
 * register the row or column layer next to a boundary as a "copy layer",
 * from which values will be copied into the ghost layer or a neighbour;
 * @return	a SWE_Block1D object that contains row variables h, hu, and hv
 */
SWE_Block1D* SWE_Block::registerCopyLayer(BoundaryEdge edge){

  switch (edge) {
    case BND_LEFT:
      return new SWE_Block1D( h.getColProxy(1), hu.getColProxy(1), hv.getColProxy(1) );
    case BND_RIGHT:
      return new SWE_Block1D( h.getColProxy(nx), hu.getColProxy(nx), hv.getColProxy(nx) );
    case BND_BOTTOM:
      return new SWE_Block1D( h.getRowProxy(1), hu.getRowProxy(1), hv.getRowProxy(1));
    case BND_TOP:
      return new SWE_Block1D( h.getRowProxy(ny), hu.getRowProxy(ny), hv.getRowProxy(ny));
  };
  return NULL;
}

/**
 * "grab" the ghost layer at the specific boundary in order to set boundary values 
 * in this ghost layer externally. 
 * The boundary conditions at the respective ghost layer is set to PASSIVE, 
 * such that the grabbing program component is responsible to provide correct 
 * values in the ghost layer, for example by receiving data from a remote 
 * copy layer via MPI communication. 
 * @param	specified edge
 * @return	a SWE_Block1D object that contains row variables h, hu, and hv
 */
SWE_Block1D* SWE_Block::grabGhostLayer(BoundaryEdge edge){

  boundary[edge] = PASSIVE;
  switch (edge) {
    case BND_LEFT:
      return new SWE_Block1D( h.getColProxy(0), hu.getColProxy(0), hv.getColProxy(0) );
    case BND_RIGHT:
      return new SWE_Block1D( h.getColProxy(nx+1), hu.getColProxy(nx+1), hv.getColProxy(nx+1) );
    case BND_BOTTOM:
      return new SWE_Block1D( h.getRowProxy(0), hu.getRowProxy(0), hv.getRowProxy(0));
    case BND_TOP:
      return new SWE_Block1D( h.getRowProxy(ny+1), hu.getRowProxy(ny+1), hv.getRowProxy(ny+1));
  };
  return NULL;
}


/**
 * set the values of all ghost cells depending on the specifed 
 * boundary conditions;
 * if the ghost layer replicates the variables of a remote SWE_Block, 
 * the values are copied
 */
void SWE_Block::setGhostLayer() {

#ifdef DBG
  cout << "Set simple boundary conditions " << endl << flush;
#endif
  // call to virtual function to set ghost layer values 
  setBoundaryConditions();

  // for a CONNECT boundary, data will be copied from a neighbouring
  // SWE_Block (via a SWE_Block1D proxy object)
  // -> these copy operations cannot be executed in GPU/accelerator memory, e.g.
  //    setBoundaryConditions then has to take care that values are copied.
  
#ifdef DBG
  cout << "Set CONNECT boundary conditions in main memory " << endl << flush;
#endif
  // left boundary
  if (boundary[BND_LEFT] == CONNECT) {
     for(int j=0; j<=ny+1; j++) {
       h[0][j] = neighbour[BND_LEFT]->h[j];
       hu[0][j] = neighbour[BND_LEFT]->hu[j];
       hv[0][j] = neighbour[BND_LEFT]->hv[j];
      };
  };
  
  // right boundary
  if(boundary[BND_RIGHT] == CONNECT) {
     for(int j=0; j<=ny+1; j++) {
       h[nx+1][j] = neighbour[BND_RIGHT]->h[j];
       hu[nx+1][j] = neighbour[BND_RIGHT]->hu[j];
       hv[nx+1][j] = neighbour[BND_RIGHT]->hv[j];
      };
  };

  // bottom boundary
  if(boundary[BND_BOTTOM] == CONNECT) {
     for(int i=0; i<=nx+1; i++) {
       h[i][0] = neighbour[BND_BOTTOM]->h[i];
       hu[i][0] = neighbour[BND_BOTTOM]->hu[i];
       hv[i][0] = neighbour[BND_BOTTOM]->hv[i];
      };
  };

  // top boundary
  if(boundary[BND_TOP] == CONNECT) {
     for(int i=0; i<=nx+1; i++) {
       h[i][ny+1] = neighbour[BND_TOP]->h[i];
       hu[i][ny+1] = neighbour[BND_TOP]->hu[i];
       hv[i][ny+1] = neighbour[BND_TOP]->hv[i];
     }
  };

#ifdef DBG
  cout << "Synchronize ghost layers (for heterogeneous memory) " << endl << flush;
#endif
  // synchronize the ghost layers (for PASSIVE and CONNECT conditions)
  // with accelerator memory
  synchGhostLayerAfterWrite();
}

/**
 * Compute the largest allowed time step for the current grid block
 * (reference implementation) depending on the current values of 
 * variables h, hu, and hv, and store this time step size in member 
 * variable maxTimestep.
 *
 * @param i_dryTol dry tolerance (dry cells do not affect the time step).
 * @param i_cflNumber CFL number of the used method.
 */
void SWE_Block::computeMaxTimestep( const float i_dryTol,
                                    const float i_cflNumber ) {
  
  // initialize the maximum wave speed
  float l_maximumWaveSpeed = (float) 0;

  // compute the maximum wave speed within the grid
  for(int i=1; i <= nx; i++) {
    for(int j=1; j <= ny; j++) {
      if( h[i][j] > i_dryTol ) {
        float l_momentum = std::max( std::abs( hu[i][j] ),
                                     std::abs( hv[i][j] ) );

        float l_particleVelocity = l_momentum / h[i][j];
        
        // approximate the wave speed
        float l_waveSpeed = l_particleVelocity + std::sqrt( g * h[i][j] );
        
        l_maximumWaveSpeed = std::max( l_maximumWaveSpeed, l_waveSpeed );
      }
    }
  }
  
  float l_minimumCellLength = std::min( dx, dy );

  // set the maximum time step variable
  maxTimestep = l_minimumCellLength / l_maximumWaveSpeed;

  // apply the CFL condition
  maxTimestep *= i_cflNumber;
}


//==================================================================
// protected member functions for simulation
// (to provide a reference implementation)
//==================================================================

/**
 * set the values of all ghost cells depending on the specifed 
 * boundary conditions
 * - set boundary conditions for typs WALL and OUTFLOW
 * - derived classes need to transfer ghost layers
 */
void SWE_Block::setBoundaryConditions() {

  // CONNECT boundary conditions are set in the calling function setGhostLayer
  // PASSIVE boundary conditions need to be set by the component using SWE_Block

  // left boundary
  switch(boundary[BND_LEFT]) {
    case WALL:
    {
      for(int j=1; j<=ny; j++) {
        h[0][j] = h[1][j];
        hu[0][j] = -hu[1][j];
        hv[0][j] = hv[1][j];
      };
      break;
    }
    case OUTFLOW:
    {
      for(int j=1; j<=ny; j++) {
        h[0][j] = h[1][j];
        hu[0][j] = hu[1][j];
        hv[0][j] = hv[1][j];
      };
      break;
    }
    case CONNECT:
    case PASSIVE:
      break;
    default:
      assert(false);
      break;
  };

  // right boundary
  switch(boundary[BND_RIGHT]) {
    case WALL:
    {
      for(int j=1; j<=ny; j++) {
        h[nx+1][j] = h[nx][j];
        hu[nx+1][j] = -hu[nx][j];
        hv[nx+1][j] = hv[nx][j];
      };
      break;
    }
    case OUTFLOW:
    {
      for(int j=1; j<=ny; j++) {
        h[nx+1][j] = h[nx][j];
        hu[nx+1][j] = hu[nx][j];
        hv[nx+1][j] = hv[nx][j];
      };
      break;
    }
    case CONNECT:
    case PASSIVE:
      break;
    default:
      assert(false);
      break;
  };

  // bottom boundary
  switch(boundary[BND_BOTTOM]) {
    case WALL:
    {
      for(int i=1; i<=nx; i++) {
        h[i][0] = h[i][1];
        hu[i][0] = hu[i][1];
        hv[i][0] = -hv[i][1];
      };
      break;
    }
    case OUTFLOW:
    {
      for(int i=1; i<=nx; i++) {
        h[i][0] = h[i][1];
        hu[i][0] = hu[i][1];
        hv[i][0] = hv[i][1];
      };
      break;
    }
    case CONNECT:
    case PASSIVE:
      break;
    default:
      assert(false);
      break;
  };

  // top boundary
  switch(boundary[BND_TOP]) {
    case WALL:
    {
      for(int i=1; i<=nx; i++) {
        h[i][ny+1] = h[i][ny];
        hu[i][ny+1] = hu[i][ny];
        hv[i][ny+1] = -hv[i][ny];
      };
      break;
    }
    case OUTFLOW:
    {
      for(int i=1; i<=nx; i++) {
        h[i][ny+1] = h[i][ny];
        hu[i][ny+1] = hu[i][ny];
        hv[i][ny+1] = hv[i][ny];
      };
      break;
    }
    case CONNECT:
    case PASSIVE:
      break;
    default:
      assert(false);
      break;
  };

  /*
   * Set values in corner ghost cells. Required for dimensional splitting and visualizuation.
   *   The quantities in the corner ghost cells are chosen to generate a zero Riemann solutions
   *   (steady state) with the neighboring cells. For the lower left corner (0,0) using
   *   the values of (1,1) generates a steady state (zero) Riemann problem for (0,0) - (0,1) and
   *   (0,0) - (1,0) for both outflow and reflecting boundary conditions.
   * 
   *   Remark: Unsplit methods don't need corner values.
   *
   * Sketch (reflecting boundary conditions, lower left corner):
   * <pre>
   *                  **************************
   *                  *  _    _    *  _    _   *   
   *  Ghost           * |  h   |   * |  h   |  *   
   *  cell    ------> * | -hu  |   * |  hu  |  * <------ Cell (1,1) inside the domain
   *  (0,1)           * |_ hv _|   * |_ hv _|  *  
   *                  *            *           *
   *                  **************************
   *                  *  _    _    *  _    _   *
   *   Corner Ghost   * |  h   |   * |  h   |  *  
   *   cell   ------> * |  hu  |   * |  hu  |  * <----- Ghost cell (1,0)
   *   (0,0)          * |_ hv _|   * |_-hv _|  * 
   *                  *            *           *
   *                  **************************
   * </pre>
   */
  h [0][0] = h [1][1];
  hu[0][0] = hu[1][1];
  hv[0][0] = hv[1][1];

  h [0][ny+1] = h [1][ny];
  hu[0][ny+1] = hu[1][ny];
  hv[0][ny+1] = hv[1][ny];
  
  h [nx+1][0] = h [nx][1];
  hu[nx+1][0] = hu[nx][1];
  hv[nx+1][0] = hv[nx][1];

  h [nx+1][ny+1] = h [nx][ny];
  hu[nx+1][ny+1] = hu[nx][ny];
  hv[nx+1][ny+1] = hv[nx][ny];
}


//==================================================================
// protected member functions for memory model: 
// in case of temporary variables (especial in non-local memory, for 
// example on accelerators), the main variables h, hu, hv, and b 
// are not necessarily updated after each time step.
// The following methods are called to synchronise before or after 
// external read or write to the variables.
//==================================================================

/**
 * Update all temporary and non-local (for heterogeneous computing) variables
 * after an external update of the main variables h, hu, hv, and b.
 */
void SWE_Block::synchAfterWrite() {
   synchWaterHeightAfterWrite();
   synchDischargeAfterWrite();
   synchBathymetryAfterWrite();
}

/**
 * Update temporary and non-local (for heterogeneous computing) variables
 * after an external update of the water height h
 */
void SWE_Block::synchWaterHeightAfterWrite() {}

/**
 * Update temporary and non-local (for heterogeneous computing) variables
 * after an external update of the discharge variables hu and hv
 */
void SWE_Block::synchDischargeAfterWrite() {}

/**
 * Update temporary and non-local (for heterogeneous computing) variables
 * after an external update of the bathymetry b
 */
void SWE_Block::synchBathymetryAfterWrite() {}

/**
 * Update the ghost layers (only for CONNECT and PASSIVE boundary conditions)
 * after an external update of the main variables h, hu, hv, and b in the 
 * ghost layer.
 */
void SWE_Block::synchGhostLayerAfterWrite() {}

/**
 * Update all temporary and non-local (for heterogeneous computing) variables
 * before an external access to the main variables h, hu, hv, and b.
 */
void SWE_Block::synchBeforeRead() {
   synchWaterHeightBeforeRead();
   synchDischargeBeforeRead();
   synchBathymetryBeforeRead();
}

/**
 * Update temporary and non-local (for heterogeneous computing) variables
 * before an external access to the water height h
 */
void SWE_Block::synchWaterHeightBeforeRead() {}

/**
 * Update temporary and non-local (for heterogeneous computing) variables
 * before an external access to the discharge variables hu and hv
 */
void SWE_Block::synchDischargeBeforeRead() {}

/**
 * Update temporary and non-local (for heterogeneous computing) variables
 * before an external access to the bathymetry b
 */
void SWE_Block::synchBathymetryBeforeRead() {}

/**
 * Update (for heterogeneous computing) variables in copy layers
 * before an external access to the unknowns
 */
void SWE_Block::synchCopyLayerBeforeRead() {}

