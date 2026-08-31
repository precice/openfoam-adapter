# Local extensions: propeller and control-surface coupling

This fork of the OpenFOAM-preCICE adapter (OpenFOAM-14 port) adds optional
coupling data for **actuator-disk propellers** and **hinged control surfaces**
on top of the standard FSI (`Force` / `AirVelocity` / `Stress` /
`Displacement`) exchange. They are used by the `multiphysics` flying-sled
example (`test/flying-sled/`) and are generic enough for any aircraft /
rotor / flap case.

## New location type: `fixedPoints`

Interfaces can now use `locations fixedPoints` to register an arbitrary,
explicit list of vertices (instead of patch face-centres/nodes or volume
centres). This is how a propeller hub or a control-surface hinge is exposed
as a single-vertex preCICE mesh.

Two ways to specify the points in the interface dictionary:

```cpp
// (a) from propellerDisk fvModels (one vertex per fvModel centre)
Interface2
{
    mesh        Propeller-Mesh;
    locations   fixedPoints;
    propellers  (propeller);
    writeData   (Thrust PropTorque);
}

// (b) explicit points (e.g. control-surface hinges)
Interface3
{
    mesh        Control-Mesh;
    locations   fixedPoints;
    points      ((0.55 0.19 0.01) (0.55 0.21 0.01));
    patches     (elevonLeft elevonRight);
    readData    (Deflection);
    writeData   (HingeForce HingeMoment);
}
```

For fixedPoints interfaces the `patches`, `writeData` and `readData` entries
are all optional.

## Propeller loads: `FSI/PropellerLoad`

Writes the resultant actuator-disk loads to a single-vertex mesh at the hub:

- `Thrust`     = `-propellerDisk.force()`  (force of the propeller on the
  airframe, opposite to the momentum added to the fluid),
- `PropTorque` = `-propellerDisk.moment()` (reaction torque about the hub).

The propellerDisk fvModels to use are listed in the FSI sub-dictionary:

```cpp
FSI
{
    solverType  incompressible;
    propellers  (propeller);
}
```

## Control surfaces: `FSI/ControlDeflection` (reader) + `FSI/SurfaceHinge` (writer)

Declare each surface in the FSI sub-dictionary, in the same order as the
`Control-Mesh` fixed points:

```cpp
FSI
{
    controlSurfaces
    (
        { patch elevonLeft;  hinge (0.55 0.19 0.01); axis (0 1 0); }
        { patch elevonRight; hinge (0.55 0.21 0.01); axis (0 1 0); }
    );
}
```

- `ControlDeflection::read` rotates each surface patch about its hinge line
  (Rodrigues rotation of the reference points by the received `Deflection`
  angle) and writes the displacement into the `pointDisplacement` field on
  that patch. The case must use a displacement-based mesh-motion solver
  (`dynamicMeshDict` → `mover displacementLaplacian`).
- `SurfaceHinge::write` computes the net aerodynamic force and the moment
  about the hinge of the fluid on each surface patch (pressure + viscous,
  same calculation as `ForceBase`), and writes them as `HingeForce` /
  `HingeMoment` (one vector per fixed point).

## Fixed a missing read

`Adapter::execute()` now calls `readCouplingData(timestepSolver_)` after
`advance()`, so the moving-wall `AirVelocity`, the structural `Displacement`
and the control-surface `Deflection` readers actually apply the values they
receive. Without this call the adapter only wrote coupling data and never read
it back into OpenFOAM fields.

## Build

`Make/options` adds the `propellerDisk` include path and links
`-lpropellerDisk` (needed by the `fixedPoints` interface to look up the
fvModel hub centres and by `PropellerLoad` for the RTTI cast).

See `../multiphysics/test/flying-sled/README.md` for a complete working
example (case dictionaries, parameters, run modes).