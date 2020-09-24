/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema, Tobias Schnabel
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

#ifndef __SWE_SCENARIO_H
#define __SWE_SCENARIO_H

/**
 * enum type: available types of boundary conditions
 */
typedef enum BoundaryType {
   OUTFLOW, WALL, INFLOW, CONNECT, PASSIVE
} BoundaryType;

/**
 * enum type: numbering of the boundary edges
 */
typedef enum BoundaryEdge {
   BND_LEFT, BND_RIGHT, BND_BOTTOM, BND_TOP
} BoundaryEdge;

/**
 * SWE_Scenario defines an interface to initialise the unknowns of a 
 * shallow water simulation - i.e. to initialise water height, velocities,
 * and bathymatry according to certain scenarios.
 * SWE_Scenario can act as stand-alone scenario class, providing a very
 * basic scenario (all functions are constant); however, the idea is 
 * to provide derived classes that implement the SWE_Scenario interface
 * for more interesting scenarios.
 */
class SWE_Scenario {

 public:

    virtual float getWaterHeight(float x, float y) { return 10.0f; };
    virtual float getVeloc_u(float x, float y) { return 0.0f; };
    virtual float getVeloc_v(float x, float y) { return 0.0f; };
    virtual float getBathymetry(float x, float y) { return 0.0f; };
    
    virtual float waterHeightAtRest() { return 10.0f; };

    virtual float endSimulation() { return 0.1f; };
    
    virtual BoundaryType getBoundaryType(BoundaryEdge edge) { return WALL; };
    virtual float getBoundaryPos(BoundaryEdge edge) {
       if (edge==BND_LEFT || edge==BND_BOTTOM)
          return 0.0f;
       else
          return 1.0f; 
    };
    
    virtual ~SWE_Scenario() {};

};


#endif
