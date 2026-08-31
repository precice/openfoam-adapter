#ifndef FSI_CONTROLDEFLECTION_H
#define FSI_CONTROLDEFLECTION_H

#include "CouplingDataUser.H"

namespace preciceAdapter
{
namespace FSI
{

//- Configuration of a single (rigidly rotating) control surface
struct ControlSurfaceConfig
{
    std::string patch;          // OpenFOAM patch of the surface
    std::vector<double> hinge;  // hinge point on the rotation axis [m]
    std::vector<double> axis;   // unit rotation axis [-]
};

//- Class that reads a per-surface deflection angle (scalar) from preCICE and
//  rotates the corresponding control-surface patch about its hinge. The
//  resulting displacement is written into the pointDisplacement field on that
//  patch, from which OpenFOAM's dynamicMotionSolverFvMesh (with
//  displacementLaplacian) propagates the deformation into the fluid mesh.
//
//  This data is meant to live on an interface with location type
//  "fixedPoints" whose vertices coincide with the surface hinges (one vertex
//  per control surface, in the same order as the controlSurfaces list).
class ControlDeflection : public CouplingDataUser
{
private:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Configuration of the control surfaces (patch + hinge + axis)
    std::vector<ControlSurfaceConfig> controlSurfaces_;

    //- pointDisplacement field name
    std::string namePointDisplacement_;

    //- Reference (initial) points of each control-surface patch, stored at
    //  initialize() so the rotation is always measured from the reference
    //  (undeflected) configuration.
    std::vector<std::vector<double>> referencePoints_;

    //- Initialize the reference points of all control-surface patches
    void storeReferencePoints();

public:
    //- Constructor
    ControlDeflection(
        const Foam::fvMesh& mesh,
        std::vector<ControlSurfaceConfig> controlSurfaces);

    //- Write the deflection to the buffer (not implemented)
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read the deflection angles from the buffer and rotate the patches
    void read(double* buffer, const unsigned int dim) final;

    //- For this data set, only the fixedPoints location type is supported
    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;

    //- Initialize the reference points (called after configuration)
    void initialize() final;
};

}
}

#endif