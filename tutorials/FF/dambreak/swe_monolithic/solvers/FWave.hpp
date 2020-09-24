/**
 * FWave.hpp
 *
 ****
 **** F-Wave Riemann Solver for the Shallow Water Equation
 ****
 *
 *  Created on: Aug 25, 2011
 *  Last Update: Feb 18, 2012
 *
 ****
 *
 *  Author: Alexander Breuer
 *    Homepage: http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer
 *    E-Mail: breuera AT in.tum.de
 *
 ****
 *
 * (Main) Literature:
 *
 *   @article{bale2002wave,
 *            title={A wave propagation method for conservation laws and balance laws with spatially varying flux functions},
 *            author={Bale, D.S. and LeVeque, R.J. and Mitran, S. and Rossmanith, J.A.},
 *            journal={SIAM Journal on Scientific Computing},
 *            volume={24},
 *            number={3},
 *            pages={955--978},
 *            year={2002},
 *            publisher={Citeseer}}
 *
 *   @book{leveque2002finite,
 *         Author = {LeVeque, R. J.},
 *         Date-Added = {2011-09-13 14:09:31 +0000},
 *         Date-Modified = {2011-10-31 09:46:40 +0000},
 *         Publisher = {Cambridge University Press},
 *         Title = {Finite Volume Methods for Hyperbolic Problems},
 *         Volume = {31},
 *         Year = {2002}}
 *
 *   @webpage{levequeclawpack,
 *            Author = {LeVeque, R. J.},
 *            Lastchecked = {January, 05, 2011},
 *            Title = {Clawpack Sofware},
 *            Url = {https://github.com/clawpack/clawpack-4.x/blob/master/geoclaw/2d/lib}}
 *
 ****
 *
 * Acknowledgments:
 *   Special thanks go to R.J. LeVeque and D.L. George for publishing their code
 *   and the corresponding documentation (-> Literature).
 */

#ifndef FWAVE_HPP_
#define FWAVE_HPP_

#include <cassert>
#include <cmath>
#include <algorithm>
#include "WavePropagation.hpp"

namespace solver {
  template <typename T> class FWave;
}

/**
 * FWave Riemann Solver for the Shallow Water Equations.
 *
 * T should be double or float.
 */
template <typename T> class solver::FWave: public WavePropagation<T> {
  private: //explicit for unit tests
    //use nondependent names (template base class)
    using solver::WavePropagation<T>::zeroTol;
    using solver::WavePropagation<T>::g;
    using solver::WavePropagation<T>::dryTol;

    using solver::WavePropagation<T>::hLeft;
    using solver::WavePropagation<T>::hRight;
    using solver::WavePropagation<T>::huLeft;
    using solver::WavePropagation<T>::huRight;
    using solver::WavePropagation<T>::bLeft;
    using solver::WavePropagation<T>::bRight;
    using solver::WavePropagation<T>::uLeft;
    using solver::WavePropagation<T>::uRight;

    using solver::WavePropagation<T>::wetDryState;
    using solver::WavePropagation<T>::DryDry;
    using solver::WavePropagation<T>::WetWet;
    using solver::WavePropagation<T>::WetDryWall;
    using solver::WavePropagation<T>::DryWetWall;

    using solver::WavePropagation<T>::storeParameters;

  public:
    /**
     * Constructor of the f-wave solver with optional parameters.
     *
     * @param i_dryTolerance numerical definition of "dry".
     * @param i_gravity gravity constant.
     * @param i_zeroTolerance numerical definition of zero.
     */
    FWave( T i_dryTolerance =  (T) 0.01,
           T i_gravity =       (T) 9.81,
           T i_zeroTolerance = (T) 0.000000001 ):
             WavePropagation<T>( i_dryTolerance, i_gravity, i_zeroTolerance ) {

#ifndef NDEBUG
#ifndef SUPPRESS_SOLVER_DEBUG_OUTPUT
      //print some information about the used solver
      std::cout << "  *** solver::FWave created" << std::endl
                << "    zeroTolerance=" << zeroTol << std::endl
                << "    gravity=" << g << std::endl
                << "    dryTolerance=" << dryTol << std::endl
                << "  ***\n\n";
#endif
#endif
    }

    /**
     * Compute net updates for the cell on the left/right side of the edge.
     * This is the default method of a standalone f-Wave solver.
     *
     * Please note:
     *   In the case of a Dry/Wet- or Wet/Dry-boundary, wall boundary conditions will be set.
     *   The f-Wave solver is not positivity preserving.
     *   -> You as the programmer have to take care about "negative water heights"!
     *
     * @param i_hLeft height on the left side of the edge.
     * @param i_hRight height on the right side of the edge.
     * @param i_huLeft momentum on the left side of the edge.
     * @param i_huRight momentum on the right side of the edge.
     * @param i_bLeft bathymetry on the left side of the edge.
     * @param i_bRight bathymetry on the right side of the edge.
     *
     * @param o_hUpdateLeft will be set to: Net-update for the height of the cell on the left side of the edge.
     * @param o_hUpdateRight will be set to: Net-update for the height of the cell on the right side of the edge.
     * @param o_huUpdateLeft will be set to: Net-update for the momentum of the cell on the left side of the edge.
     * @param o_huUpdateRight will be set to: Net-update for the momentum of the cell on the right side of the edge.
     * @param o_maxWaveSpeed will be set to: Maximum (linearized) wave speed -> Should be used in the CFL-condition.
     */
    void computeNetUpdates ( const T &i_hLeft,  const T &i_hRight,
                             const T &i_huLeft, const T &i_huRight,
                             const T &i_bLeft,  const T &i_bRight,

                             T &o_hUpdateLeft,
                             T &o_hUpdateRight,
                             T &o_huUpdateLeft,
                             T &o_huUpdateRight,
                             T &o_maxWaveSpeed ) {

      //set speeds to zero (will be determined later)
      uLeft = uRight = 0;

      //reset the maximum wave speed
      o_maxWaveSpeed = 0;

      //! wave speeds of the f-waves
      T waveSpeeds[2];

      //store parameters to member variables
      storeParameters( i_hLeft, i_hRight,
                       i_huLeft, i_huRight,
                       i_bLeft, i_bRight );

      //determine the wet/dry state and compute local variables correspondingly
      determineWetDryState();

      //zero updates and return in the case of dry cells
      if(wetDryState == DryDry) {
        o_hUpdateLeft = o_hUpdateRight = o_huUpdateLeft = o_huUpdateRight  = (T)0.;
        return;
      }

      //compute the wave speeds
      computeWaveSpeeds(waveSpeeds);

      //use the wave speeds to compute the net-updates
      computeNetUpdates_WithWaveSpeeds( waveSpeeds,
                                        o_hUpdateLeft, o_hUpdateRight,
                                        o_huUpdateLeft, o_huUpdateRight,
                                        o_maxWaveSpeed );

      //zero ghost updates (wall boundary)
      if(wetDryState == WetDryWall) {
        o_hUpdateRight = 0;
        o_huUpdateRight = 0;
      }
      else if(wetDryState == DryWetWall) {
        o_hUpdateLeft = 0;
        o_huUpdateLeft = 0;
      }
    }

    /**
     * Compute net updates for the cell on the left/right side of the edge.
     * This is an expert method, because a lot of (numerical-)knowledge about the problem is assumed/has to be provided.
     * It is the f-Wave entry point for the hybrid solver,  which combines the "simple" F-Wave approach with the more complex Augmented Riemann Solver.
     *
     * wetDryState is assumed to be WetWet.
     *
     *
     * @param i_hLeft height on the left side of the edge.
     * @param i_hRight height on the right side of the edge.
     * @param i_huLeft momentum on the left side of the edge.
     * @param i_huRight momentum on the right side of the edge.
     * @param i_bLeft bathymetry on the left side of the edge.
     * @param i_bRight bathymetry on the right side of the edge.
     * @param i_uLeft velocity on the left side of the edge.
     * @param i_uRight velocity on the right side of the edge.
     * @param waveSpeeds speeds of the linearized waves (eigenvalues).
     *                   A hybrid solver will typically provide its own values.
     *
     * @param o_hUpdateLeft will be set to: Net-update for the height of the cell on the left side of the edge.
     * @param o_hUpdateRight will be set to: Net-update for the height of the cell on the right side of the edge.
     * @param o_huUpdateLeft will be set to: Net-update for the momentum of the cell on the left side of the edge.
     * @param o_huUpdateRight will be set to: Net-update for the momentum of the cell on the right side of the edge.
     * @param o_maxWaveSpeed will be set to: Maximum (linearized) wave speed -> Should be used in the CFL-condition.
     */
    void computeNetUpdatesHybrid ( const T &i_hLeft,  const T &i_hRight,
                                   const T &i_huLeft, const T &i_huRight,
                                   const T &i_bLeft,  const T &i_bRight,
                                   const T &i_uLeft,  const T &i_uRight,

                                   const T i_waveSpeeds[2],

                                   T &o_hUpdateLeft,
                                   T &o_hUpdateRight,
                                   T &o_huUpdateLeft,
                                   T &o_huUpdateRight,
                                   T &o_maxWaveSpeed ) {
      //store parameters to member variables
      storeParameters( i_hLeft,  i_hRight,
                       i_huLeft, i_huRight,
                       i_bLeft,  i_bRight,
                       i_uLeft,  i_uRight );

      computeNetUpdates_WithWaveSpeeds( i_waveSpeeds,
                 							          o_hUpdateLeft, o_hUpdateRight,
                      							    o_huUpdateLeft, o_huUpdateRight,
                        							  o_maxWaveSpeed );
    }

  private:
    /**
     * Compute net updates for the cell on the left/right side of the edge.
     * Its assumed that the member variables are set already.
     *
     * @param i_waveSpeeds speeds of the linearized waves (eigenvalues).
     *
     * @param o_hUpdateLeft will be set to: Net-update for the height of the cell on the left side of the edge.
     * @param o_hUpdateRight will be set to: Net-update for the height of the cell on the right side of the edge.
     * @param o_huUpdateLeft will be set to: Net-update for the momentum of the cell on the left side of the edge.
     * @param o_huUpdateRight will be set to: Net-update for the momentum of the cell on the right side of the edge.
     * @param o_maxWaveSpeed will be set to: Maximum (linearized) wave speed -> Should be used in the CFL-condition.
     */
    void computeNetUpdates_WithWaveSpeeds ( const T i_waveSpeeds[2],

                            							  T &o_hUpdateLeft,
                             								T &o_hUpdateRight,
                             					      T &o_huUpdateLeft,
                             								T &o_huUpdateRight,
                            							  T &o_maxWaveSpeed ) {
      //reset net updates
      o_hUpdateLeft = o_hUpdateRight = o_huUpdateLeft = o_huUpdateRight  = (T)0.;

      //! where to store the two f-waves
      T fWaves[2][2];

      //compute the decomposition into f-waves
      computeWaveDecomposition( i_waveSpeeds, fWaves );

      //compute the net-updates
      //1st wave family
      if(i_waveSpeeds[0] < -zeroTol) { //left going
        o_hUpdateLeft +=  fWaves[0][0];
        o_huUpdateLeft += fWaves[0][1];
      }
      else if(i_waveSpeeds[0] > zeroTol) { //right going
        o_hUpdateRight +=  fWaves[0][0];
        o_huUpdateRight += fWaves[0][1];
      }
      else { //split waves
        o_hUpdateLeft +=   (T).5*fWaves[0][0];
        o_huUpdateLeft +=  (T).5*fWaves[0][1];
        o_hUpdateRight +=  (T).5*fWaves[0][0];
        o_huUpdateRight += (T).5*fWaves[0][1];
      }

      //2nd wave family
      if(i_waveSpeeds[1] < -zeroTol) { //left going
          o_hUpdateLeft +=  fWaves[1][0];
          o_huUpdateLeft += fWaves[1][1];
      }
      else if(i_waveSpeeds[1] > zeroTol) { //right going
          o_hUpdateRight += fWaves[1][0];
          o_huUpdateRight += fWaves[1][1];
      }
      else { //split waves
        o_hUpdateLeft +=   (T).5*fWaves[1][0];
        o_huUpdateLeft +=  (T).5*fWaves[1][1];
        o_hUpdateRight +=  (T).5*fWaves[1][0];
        o_huUpdateRight += (T).5*fWaves[1][1];
      }

      //compute maximum wave speed (-> CFL-condition)
      o_maxWaveSpeed = std::max( std::fabs(i_waveSpeeds[0]) , std::fabs(i_waveSpeeds[1]) );
    }

    /**
     * Determine the wet/dry state and set member variables accordingly.
     */
    void determineWetDryState() {
      //determine the wet/dry state
      if(hLeft < dryTol && hRight < dryTol) { //both cells are dry
        wetDryState = DryDry;
      }
      else if(hLeft < dryTol) { // left cell dry, right cell wet
        uRight = huRight/hRight;

        //Set wall boundary conditions.
        //This is not correct in the case of inundation problems.
        hLeft = hRight;
        bLeft = bRight;
        huLeft = -huRight;
        uLeft = -uRight;
        wetDryState = DryWetWall;
      }
      else if(hRight < dryTol) { //left cell wet, right cell dry
        uLeft = huLeft/hLeft;

        //Set wall boundary conditions.
        //This is not correct in the case of inundation problems.
          hRight = hLeft;
          bRight = bLeft;
          huRight = -huLeft;
          uLeft = -uRight;
          wetDryState = WetDryWall;
      }
      else { //both cells wet
        uLeft = huLeft/hLeft;
        uRight = huRight/hRight;

        wetDryState = WetWet;
      }
    }

    /**
     * Compute the decomposition into f-Waves.
     *
     * @param i_waveSpeeds speeds of the linearized waves (eigenvalues).
     * @param o_fWaves  will be set to: Decomposition into f-Waves.
     */
    void computeWaveDecomposition( const T i_waveSpeeds[2],
                                         T o_fWaves[2][2]
                                 ) const {
      //TODO: Update documentation.
      //Eigenvalues***********************************************************************************************
      //Computed somewhere before.
      //An option would be to use the char. speeds:
      //
      //lambda^1 = u_{i-1} - sqrt(g*h_{i-1})
      //lambda^2 = u_i     + sqrt(g*h_i)
      //Matrix of right eigenvectors******************************************************************************
      //     1                              1
      // R =
      //     u_{i-1} - sqrt(g * h_{i-1})    u_i + sqrt(g * h_i)
      //**********************************************************************************************************
      //                                                                      u_i + sqrt(g * h_i)              -1
      // R^{-1} = 1 / (u_i - sqrt(g * h_i) - u_{i-1} + sqrt(g * h_{i-1}) *
      //                                                                   -( u_{i-1} - sqrt(g * h_{i-1}) )     1
      //**********************************************************************************************************
      //                hu
      // f(q) =
      //         hu^2 + 1/2 g * h^2
      //
      //**********************************************************************************************************
      //                                    0
      //  \delta x \Psi =
      //                  -g * 1/2 * (h_i + h_{i-1}) * (b_i - b_{i+1})
      //**********************************************************************************************************
      // beta = R^{-1} * (f(Q_i) - f(Q_{i-1}) - \delta x \Psi)
      //**********************************************************************************************************

      //assert: wave speed of the 1st wave family should be less than the speed of the 2nd wave family.
      assert( i_waveSpeeds[0] < i_waveSpeeds[1] );

      T lambdaDif = i_waveSpeeds[1] - i_waveSpeeds[0];

      //assert: no division by zero
      assert(std::abs(lambdaDif) > zeroTol);

      //compute the inverse matrix R^{-1}
      T Rinv[2][2];

      T oneDivLambdaDif = (T)1. /  lambdaDif;
      Rinv[0][0] = oneDivLambdaDif *  i_waveSpeeds[1];
      Rinv[0][1] = -oneDivLambdaDif;

      Rinv[1][0] = oneDivLambdaDif * -i_waveSpeeds[0];
      Rinv[1][1] = oneDivLambdaDif;

      //right hand side
      T fDif[2];

      //calculate modified (bathymetry!) flux difference
      // f(Q_i) - f(Q_{i-1})
      fDif[0] = huRight - huLeft;
      fDif[1] = huRight * uRight + (T).5 * g * hRight * hRight
              -(huLeft  * uLeft  + (T).5 * g * hLeft  * hLeft);

      // \delta x \Psi[2]
      T psi = -g * (T).5 * (hRight + hLeft)*(bRight - bLeft);
      fDif[1] -= psi;

      //solve linear equations
      T beta[2];
      beta[0] = Rinv[0][0] * fDif[0] + Rinv[0][1] * fDif[1];
      beta[1] = Rinv[1][0] * fDif[0] + Rinv[1][1] * fDif[1];

      //return f-waves
      o_fWaves[0][0] = beta[0];
      o_fWaves[0][1] = beta[0] * i_waveSpeeds[0];

      o_fWaves[1][0] = beta[1];
      o_fWaves[1][1] = beta[1] * i_waveSpeeds[1];
    }

    /**
     * Compute the edge local eigenvalues.
     *
     * @param o_waveSpeeds will be set to: speeds of the linearized waves (eigenvalues).
     */
    void computeWaveSpeeds( T o_waveSpeeds[2]) const {
      //compute eigenvalues of the jacobian matrices in states Q_{i-1} and Q_{i}
      T characteristicSpeeds[2];
      characteristicSpeeds[0] = uLeft - std::sqrt(g*hLeft);
      characteristicSpeeds[1] = uRight + std::sqrt(g*hRight);

      //compute "Roe speeds"
      T hRoe = (T).5 * (hRight + hLeft);
      T uRoe = uLeft * std::sqrt(hLeft) + uRight * std::sqrt(hRight);
      uRoe /= std::sqrt(hLeft) + std::sqrt(hRight);

      T roeSpeeds[2];
      roeSpeeds[0] = uRoe - std::sqrt(g*hRoe);
      roeSpeeds[1] = uRoe + std::sqrt(g*hRoe);

      //computer eindfeldt speeds
      T einfeldtSpeeds[2];
      einfeldtSpeeds[0] = std::min(characteristicSpeeds[0], roeSpeeds[0]);
      einfeldtSpeeds[1] = std::max(characteristicSpeeds[1], roeSpeeds[1]);

      //set wave speeds
      o_waveSpeeds[0] = einfeldtSpeeds[0];
      o_waveSpeeds[1] = einfeldtSpeeds[1];
    }
};




#endif /* FWAVE_HPP_ */
