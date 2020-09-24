/**
 * WavePropagation.hpp
 *
 ****
 **** Abstract wave propagation solver for the Shallow Water Equations.
 ****
 *
 *  Created on: Jan 03, 2012
 *  Last Update: Jan 05, 2012
 *
 ****
 *
 *  Author: Alexander Breuer
 *    Homepage: http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer
 *    E-Mail: breuera AT in.tum.de
 */

#ifndef WAVEPROPAGATIONSOLVER_HPP_
#define WAVEPROPAGATIONSOLVER_HPP_

namespace solver {
  template <typename T> class WavePropagation;
}

/**
 * Abstract wave propagation solver for the Shallow Water Equations.
 *
 * T should be double or float.
 */
template <typename T> class solver::WavePropagation {
  protected:
    //global variables
    //! numerical definition of "dry".
    T dryTol;
    //! gravity constant
    const T g;
    //! numerical definition of zero.
    const T zeroTol;

#if 0
    //parameters for computeNetUpdates
    T h[2];
    T hu[2];
    T b[2];
    T u[2];

#define hLeft  (h[0])
#define hRight (h[1])

#define huLeft  (hu[0])
#define huRight (hu[1])

#define bLeft  (b[0])
#define bRight (b[1])

#define uLeft  (u[0])
#define uRight (u[1])

#else
    //edge-local variables
    //! height on the left side of the edge (could change during execution).
    T hLeft;
    //! height on the right side of the edge (could change during execution).
    T hRight;
    //! momentum on the left side of the edge (could change during execution).
    T huLeft;
    //! momentum on the right side of the edge (could change during execution).
    T huRight;
    //! bathymetry on the left side of the edge (could change during execution).
    T bLeft;
    //! bathymetry on the right side of the edge (could change during execution).
    T bRight;

    //! velocity on the left side of the edge (computed by determineWetDryState).
    T uLeft;
    //! velocity on the right side of the edge (computed by determineWetDryState).
    T uRight;
#endif

    /**
     * The wet/dry state of the Riemann-problem.
     */
    enum WetDryState {
      DryDry,               /**< Both cells are dry. */
      WetWet,               /**< Both cells are wet. */
      WetDryInundation,     /**< 1st cell: wet, 2nd cell: dry. 1st cell lies higher than the 2nd one. */
      WetDryWall,           /**< 1st cell: wet, 2nd cell: dry. 1st cell lies lower than the 2nd one.
                             *   Momentum is not large enough to overcome the difference. */
      WetDryWallInundation, /**< 1st cell: wet, 2nd cell: dry. 1st cell lies lower than the 2nd one.
                             *   Momentum is large enough to overcome the difference. */
      DryWetInundation,     /**< 1st cell: dry, 2nd cell: wet. 1st cell lies lower than the 2nd one. */
      DryWetWall,           /**< 1st cell: dry, 2nd cell: wet. 1st cell lies higher than the 2nd one.
                             *   Momentum is not large enough to overcome the difference. */
      DryWetWallInundation  /**< 1st cell: dry, 2nd cell: wet. 1st cell lies higher than the 2nd one.
                             *   Momentum is large enough to overcome the difference. */
    };

    //! wet/dry state of our Riemann-problem (determined by determineWetDryState)
    WetDryState wetDryState;


    //! Determine the wet/dry-state and set local values if we have to.
    virtual void determineWetDryState() = 0;

    /**
     * Constructor of a wave propagation solver.
     *
     * @param gravity gravity constant.
     * @param dryTolerance numerical definition of "dry".
     * @param zeroTolerance numerical definition of zero.
     */
    WavePropagation( T i_dryTolerance,
                     T i_gravity,
                     T i_zeroTolerance ):
                       dryTol(i_dryTolerance),
                       g(i_gravity),
                       zeroTol(i_zeroTolerance) {};

    /**
     * Store parameters to member variables.
     *
     * @param i_hLeft height on the left side of the edge.
     * @param i_hRight height on the right side of the edge.
     * @param i_huLeft momentum on the left side of the edge.
     * @param i_huRight momentum on the right side of the edge.
     * @param i_bLeft bathymetry on the left side of the edge.
     * @param i_bRight bathymetry on the right side of the edge.
     */
    inline void storeParameters( const T &i_hLeft,  const T &i_hRight,
                                 const T &i_huLeft, const T &i_huRight,
                                 const T &i_bLeft,  const T &i_bRight ) {
      //store parameters to member variables
      hLeft = i_hLeft;
      hRight = i_hRight;

      huLeft = i_huLeft;
      huRight = i_huRight;

      bLeft = i_bLeft;
      bRight = i_bRight;
    }

    /**
     * Store parameters to member variables.
     *
     * @param i_hLeft height on the left side of the edge.
     * @param i_hRight height on the right side of the edge.
     * @param i_huLeft momentum on the left side of the edge.
     * @param i_huRight momentum on the right side of the edge.
     * @param i_bLeft bathymetry on the left side of the edge.
     * @param i_bRight bathymetry on the right side of the edge.
     * @param i_uLeft velocity on the left side of the edge.
     * @param i_uRight velocity on the right side of the edge.
     */
    inline void storeParameters( const T &i_hLeft,  const T &i_hRight,
                                 const T &i_huLeft, const T &i_huRight,
                                 const T &i_bLeft,  const T &i_bRight,
                                 const T &i_uLeft,  const T &i_uRight) {
      //store parameters to member variables
      storeParameters( i_hLeft,  i_hRight,
                       i_huLeft, i_huRight,
                       i_bLeft,  i_bRight );

      uLeft = i_uLeft;
      uRight = i_uRight;
    }

  public:
    /**
     * Compute net updates for the cell on the left/right side of the edge.
     * This is the default method every standalone wave propagation solver should provide.
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
    virtual void computeNetUpdates ( const T &i_hLeft,  const T &i_hRight,
                                     const T &i_huLeft, const T &i_huRight,
                                     const T &i_bLeft,  const T &i_bRight,

                                     T &o_hUpdateLeft,
                                     T &o_hUpdateRight,
                                     T &o_huUpdateLeft,
                                     T &o_huUpdateRight,
                                     T &o_maxWaveSpeed
#if CONFIG_TSUNAMI_AUGMENTED_RIEMANN_EIGEN_COEFFICIENTS
                                    ,T  o_eigenCoefficients[3]
#endif
                                   ) = 0;

    /**
     * Sets the dry tolerance of the solver.
     *
     * @param i_dryTolerance dry tolerance.
     */
    void setDryTolerance( const T i_dryTolerance ) {
      dryTol = i_dryTolerance;
    }

    virtual ~WavePropagation() {};


#undef hLeft
#undef hRight

#undef huLeft
#undef huRight

#undef bLeft
#undef bRight

#undef uLeft
#undef uRight
};

#endif /* WAVEPROPAGATIONSOLVER_HPP_ */
