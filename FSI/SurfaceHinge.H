#ifndef FSI_SURFACEHINGE_H
#define FSI_SURFACEHINGE_H

#include "ForceBase.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes the net aerodynamic load of a control-surface patch
//  (one value per fixed point). For each control surface it computes the
//  total aerodynamic force and the moment of that force about the hinge
//  point, and writes either the force or the moment vector to preCICE.
//
//  Sign convention: the load of the fluid ON the control surface.
//  This data lives on an interface with location type "fixedPoints" whose
//  vertices coincide with the surface hinges.
class SurfaceHinge : public ForceBase
{
private:
    //- Control surface configurations (patch + hinge + axis)
    std::vector<ControlSurfaceConfig> controlSurfaces_;

    //- Coupled quantity (true = moment about the hinge, false = force)
    const bool isMoment_;

public:
    //- Constructor
    SurfaceHinge(
        const Foam::fvMesh& mesh,
        const std::string solverType,
        std::vector<ControlSurfaceConfig> controlSurfaces,
        bool isMoment);

    //- Write the net load of each control surface to the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read from the buffer (not implemented)
    void read(double* buffer, const unsigned int dim) final;

    //- For this data set, only the fixedPoints location type is supported
    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;

    //- Returns the normal vectors multiplied by the face area
    Foam::tmp<Foam::vectorField> getFaceVectors(const unsigned int patchID) const final;
};

}
}

#endif