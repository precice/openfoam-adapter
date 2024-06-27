---
title: Configure the OpenFOAM adapter
permalink: adapter-openfoam-config.html
keywords: adapter, openfoam, configuration, preciceDict, controlDict
summary: "Write a system/preciceDict, set compatible boundary conditions, and activate the adapter in your system/controlDict."
---

In order to run a coupled simulation, you need to:

1. prepare a preCICE configuration file (described in the [preCICE configuration](https://precice.org/configuration-overview.html)),
2. prepare an adapter's configuration file,
3. set the coupling boundaries in the OpenFOAM case,
4. load the adapter, and
5. start all the solvers normally, from the same directory, e.g., in two different terminals.

You may skip the section _"Advanced configuration"_ in the beginning, as it only concerns special cases.

## The adapter's configuration file

The adapter is configured via the file `system/preciceDict`. This file is an OpenFOAM dictionary with the following form:

```c++
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    location    "system";
    object      preciceDict;
}

preciceConfig "precice-config.xml";

participant Fluid;

modules (CHT);

interfaces
{
  Interface1
  {
    mesh              Fluid-Mesh;
    patches           (interface);
    locations         faceCenters;

    readData
    (
      Heat-Flux
    );

    writeData
    (
      Temperature
    );
  };
};
```

The `participant` needs to be the same as the one specified in the `preciceConfig`,
which is the main preCICE configuration file. The `preciceConfig` can be a path and needs to be wrapped with quotation marks.

The list `modules` can contain `CHT`, `FSI`, or/and `FF` (separated by space).
Each module provides some data fields common in the respective type of simulation, and data fields from different modules can be combined by selecting multiple modules.
For example, this is a valid combination: `modules (CHT FF);`.

In the `interfaces`, we specify the coupling interfaces (here only one).
The `mesh` needs to be the same as the one specified in the `preciceConfig`.
The `patches` specifies a list of the names of the OpenFOAM boundary patches that are
participating in the coupled simulation. These need to be defined in the files
included in the `0/` directory. The names of the interfaces (e.g., `Interface1`) are arbitrary and are not used.

The `locations` field is optional and its default value is `faceCenters` (with `faceCentres` also accepted), signifying that the interface mesh is defined on the cell face centers. An alternative option is `faceNodes`, which defines the mesh on the face nodes and is needed, e.g., for reading displacements in an FSI scenario.
The final type is `volumeCenters` (alternatively `volumeCentres`), which allows the user to couple over a volume using the cell centers of the domain. The user can also specify patches, which will be coupled additionally to the cells using the `faceCenters` mesh.
The `volumeCenters` location is currently implemented for fluid-fluid coupling (`Pressure` and `Velocity`) and conjugate heat transfer (`Temperature`).

The `cellSets` field can be used to specify one or multiple coupling regions (defined by OpenFOAM `cellSets`) for volume coupling. The field can only be used with the `volumeCenters` location and it is optional. If no `cellSets` are specified, the full domain will be coupled.

The values for `readData` and `writeData`
for conjugate heat transfer
can be `Temperature`, `Heat-Flux`, `Sink-Temperature`,
or `Heat-Transfer-Coefficient`. Postfixed names like `Sink-Temperature-Domain1` are also allowed (e.g., in order to distinguish multiple data sets of the same type).
For a Dirichlet-Neumann coupling, the `writeData` and `readData` can be
either:

```c++
readData
(
  Heat-Flux
);

writeData
(
  Temperature
);
```

or:

```c++
readData
(
  Temperature
);

writeData
(
  Heat-Flux
);
```

For a Robin-Robin coupling, we need to write and read both of `Sink-Temperature` and `Heat-Transfer-Coefficient`:

```c++
readData
(
  Sink-Temperature          // e.g., Sink-Temperature-Solid
  Heat-Transfer-Coefficient // e.g., Heat-Transfer-Coefficient-Solid
);

writeData
(
  Sink-Temperature          // e.g., Sink-Temperature-Fluid
  Heat-Transfer-Coefficient // e.g., Heat-Transfer-Coefficient-Fluid
);
```

For fluid-structure interaction, coupled quantities can be:

- `writeData`:
  - fluid participants: `Force`, `Stress` (force over area, consistent)
  - solid participants: `Displacement`
- `readData`:
  - fluid participants: `Displacement`, `DisplacementDelta` (difference to the displacement at the last coupling time window)
  - solid participants: `Force`, `Stress`

{% warning %}
You will run into problems when you use `Displacement(Delta)` as write data set and execute RBF mappings in parallel. This would affect users who use OpenFOAM and the adapter as the Solid participant in order to compute solid mechanics with OpenFOAM (currently not officially supported at all). Have a look [at this issue on GitHub](https://github.com/precice/openfoam-adapter/issues/153) for details.
{% endwarning %}

## Configuration of the OpenFOAM case

A few changes are required in the configuration of an OpenFOAM case, in order to specify the interfaces and load the adapter. For some solvers, additional parameters may be needed (see "advanced configuration").

### Boundary conditions

The type of the `readData` needs to be compatible with the respective boundary
conditions set for each field in the `0/` directory of the case.

Read the [OpenFOAM User Guide](https://www.openfoam.com/documentation/user-guide/boundaries.php) for more on boundary conditions.

#### CHT

- For `readData(Temperature)`, use `type fixedValue` for the `interface` in `0/T`.
OpenFOAM requires that you also give a (redundant) `value`, but the adapter
will overwrite it. ParaView uses this value for the initial time. As a placeholder, you can, e.g., use the value from the `internalField`.

```c++
interface
{
    type            fixedValue;
    value           $internalField;
}
```

- For `readData(Heat-Flux)`, use `type fixedGradient` for the `interface` in `0/T`.
OpenFOAM requires that you also give a (redundant) `gradient`, but the adapter will overwrite it.

```c++
interface
{
    type            fixedGradient;
    gradient        0;
}
```

- For `readData(Sink-Temperature)` or `Heat-Transfer-Coefficient`, use
`type mixed` for the `interface` in `0/T`. OpenFOAM requires that you also give (redundant) values for
`refValue`, `refGradient`, and `valueFraction`, but the adapter will overwrite them.

```c++
interface
{
    type            mixed;
    refValue        uniform 293;
    valueFraction   uniform 0.5;
    refGradient     uniform 0;
}
```

#### FSI

- For `readData(Displacement)` or `DisplacementDelta`, you need the following:
  - `type movingWallVelocity` for the interface (e.g., `flap`) in `0/U`,
  - `type fixedValue` for the interface (e.g., `flap`) in the `0/pointDisplacement`, and
  - `solver displacementLaplacian` in the `constant/dynamicMeshDict`. The solver [`RBFMeshMotionSolver` from solids4foam is also known to work](https://github.com/precice/openfoam-adapter/pull/241), since the OpenFOAM adapter v1.2.0 and the solids4foam v2.0.

```c++
// File 0/U
interface
{
    type            movingWallVelocity;
    value           uniform (0 0 0);
}

// File 0/pointDisplacement
interface
{
    type            fixedValue;
    value           $internalField;
}

// File constant/dynamicMeshDict
dynamicFvMesh       dynamicMotionSolverFvMesh;
motionSolverLibs    ("libfvMotionSolvers.so");
solver              displacementLaplacian;
```

#### FF

The fluid-fluid coupling module supports reading and writing `Pressure`, `Velocity`, `PressureGradient`, `VelocityGradient`, `FlowTemperature`, `FlowTemperatureGradient`, `Alpha`, `AlphaGradient` and the face flux `Phi`.

Similarly to the CHT module, you need a `fixedValue` boundary condition of the respective primary field in order to read and apply values, and a `fixedGradient` boundary condition of the respective gradient field in order to read and apply gradients.

Alternatively, the adapter also ships custom boundary conditions for pressure (`coupledPressure`) and velocity (`coupledVelocity`). These boundary conditions can be set on both sides of the coupling interface and can handle fluid flow in either direction. An initial `refValue` must be supplied to ensure convergence in the first time step. The adapter will overwrite the value afterwards.
If the OpenFOAM fields `phi` and `U` are given different names, they should be supplied to the boundary conditions as well.
The coupled boundary conditions act similar to the [`inletOutlet`](https://www.openfoam.com/documentation/guides/v2112/doc/guide-bcs-outlet-inlet-outlet.html) boundary conditions from OpenFOAM. However, the pressure gradient is calculated by OpenFOAM as for the [`fixedFluxExtrapolatedPressure`](https://www.openfoam.com/documentation/guides/v2112/api/classFoam_1_1fixedFluxExtrapolatedPressureFvPatchScalarField.html) boundary condition and thus no coupling of `PressureGradient` is required when using `coupledPressure`.

```c++
// File 0/U
interface
{
    type            coupledVelocity;
    refvalue        uniform (0 0 0);
    // phi            phiName
}

// File 0/p
interface
{
    type            coupledPressure;
    refValue        $internalField;
    // phi            phiName
    // U              UName
}
```

{% experimental %}
The FF module is still experimental and the boundary conditions presented here have not been rigorously tested.
{% endexperimental %}

`Alpha` refers to the phase variable used in e.g. the volume of fluid multiphase solver `interFoam`.

When coupling face flux `Phi`, usually no specific boundary condition needs to be set. The coupled boundary values are therefore not persistent and may change within a timestep.

### Volume coupling

Besides surface coupling on the domain boundaries, the OpenFOAM adapter also supports coupling overlapping domains, which can be the complete domain, or regions of it. In contrast to surface coupling, though, reading volume data (source terms) requires a few additional configuration steps compared to writing data.

In order to write volume data, it is enough to specify `volumeCenters` for the `locations` field. This will couple the whole internal field of the domain. Patches can be specified additionally, for surface coupling, or the list of patch names can be left empty.

In order to read volume data (enforce source terms), it is necessary to use the [finite volume options](https://www.openfoam.com/documentation/guides/latest/doc/guide-fvoptions.html) (`fvOptions`) feature of OpenFOAM. Without this additional configuration, the values read in OpenFOAM in each time step would later be overwritten by OpenFOAM.
The `fvOptions` construct provides many different options for sources, but the [coded sources](https://www.openfoam.com/documentation/guides/latest/doc/guide-fvoptions-sources-coded.html) is a convenient way to describe source terms in configuration.

Using a `codedSource` for reading fields and enforcing source terms in OpenFOAM would currently only work for `Velocity`. The adapter internally stores the received data in a separate velocity field, which the source term defined in OpenFOAM uses to update its own velocity field. For this reason, it is necessary to specify an alternative name for `U` in `preciceDict` when reading velocity in a volume-coupled scenario:

```c++
FF
{
  nameU       U_vol;
};
```

This essentially means two velocity variables are used: `U_vol` is the coupled velocity the adapter uses to carry over the desired value to OpenFOAM, and `U` is the variable OpenFOAM uses for its own velocity. In the `codedSource` you can explicitly set `U` to be equal to `U_vol`. Example from the [volume-coupled flow](https://precice.org/tutorials-volume-coupled-flow.html) tutorial:

```c++
// File constant/fvOptions

codedSource
{
    type            vectorCodedSource;
    selectionMode   cellSet;
    cellSet         box1;

    fields          (U);
    name            sourceTime;

    codeConstrain //constrain
    #{
        return;
    #};

    codeCorrect //correct
    #{
        const labelList& cells = this->cells();
        const volVectorField& U_vol = mesh_.lookupObject<volVectorField>("U_vol");
        for(auto cell : cells)
        {
            fld[cell].x() = U_vol[cell].x();
        }
    #};

    codeAddSup // source term
    #{
        return;
    #};

    codeAddSupRho
    #{
        return;
    #};
}
```

{% experimental %}
Reading volume-coupled variables in OpenFOAM is still experimental. This section simply contains suggestions about issues we have encountered and what has been found to work.
{% endexperimental %}

#### Volume coupling over a domain region

For reading values only over a region of the domain, we use the OpenFOAM [`cellSet` class](https://www.openfoam.com/documentation/guides/latest/api/classFoam_1_1cellSet.html) to define one or multiple volume coupling regions. You can define one or multiple `cellSets` in the `system/topoSetDict`:

```C++
actions
(
    {
        name    box1;
        type    cellSet;
        action  new;
        source  boxToCell;
        box     (3.0 1.0 0.0) (3.5 1.5 1.0);
    }
);
```

Additionally, list the `cellSets` you want to couple in the `preciceDict`:

```C++
Interface1
{
  ...
  cellSets          (box1);
  locations         volumeCenters;
}
```

Before running the solver, and after preparing the mesh, execute [topoSet](https://www.openfoam.com/documentation/guides/latest/man/topoSet.html) to construct the overlapping region.

### Load the adapter

To load this adapter, you must include the following in
the `system/controlDict` configuration file of the case:

```c++
libs ("libpreciceAdapterFunctionObject.so");
functions
{
    preCICE_Adapter
    {
        type preciceAdapterFunctionObject;
    }
}
```

This directs the solver to use the `preciceAdapterFunctionObject` function object,
which is part of the `libpreciceAdapterFunctionObject.so` shared library.
The name `preCICE_Adapter` can be arbitrary. It is important that the library is loaded outside the `functions` dictionary when you want to use the custom boundary conditions that we provide with the FF module.

If you are using other function objects in your simulation, add the preCICE adapter to the end of the list. The adapter will then be executed last, which is important, as the adapter also controls the end of the simulation. When the end of the simulation is detected, the adapter also triggers the `end()` method of all function objects.

***

## Advanced configuration

These additional parameters may only concern some users in special cases. Keep reading if you want to use the nearest-projection mapping, an incompressible or basic (e.g., laplacianFoam) solver, if you are using a solver with different variable names (e.g., a multiphase solver) or if you are trying to debug a simulation.

### Nearest-projection mapping

An example for for nearest-projection mapping is provided in the [nearest-projection tutorial case](https://precice.org/tutorials-flow-over-heated-plate-nearest-projection.html). The [preCICE documentation](https://precice.org/couple-your-code-defining-mesh-connectivity.html) contains a detailed description of nearest-projection mappings in preCICE. In summary, we need to explicitly enable the `connectivity` option to create edges between the interface mesh points and give them to preCICE:

```c++
interfaces
{
  Interface1
  {
    mesh              Fluid-Mesh-Centers;
    locations         faceCenters;
    connectivity      false;
    patches           (interface);

    // ... writeData, readData ...
  };

  Interface2
  {
    mesh              Fluid-Mesh-Nodes;
    locations         faceNodes;
    connectivity      true;
    patches           (interface);

    // ... writeData, readData ...
  };
};
```

This `connectivity` boolean is optional and defaults to `false`. Note that `connectivity true` can only be used with `locations faceNodes`.

Even if the coupling data is associated to `faceCenters` in the solver, we can select `faceNodes` as locations type: the respective data will be interpolated from faces to nodes. Also, connectivity is only needed and supported for `writeData`. Therefore, we need to split the interface in a "read" and a "write" part, as shown above.

More details about the rationale are given in the following section.

#### Adapter Implementation

Since OpenFOAM is a finite-volume based solver, data is located in the middle of the cell, or on the cell face centers for a coupling interface. Mesh connectivity can be given to preCICE using the methods `setMeshTriangle` and `setMeshEdge`. Using the face centers as arguments for these methods is cumbersome. The main reason is that, although OpenFOAM decomposes the mesh for parallel simulations and distributes the subdomains to different processes, mesh connectivity needs to be defined over the partitioned mesh boundaries. This problem vanishes if we define mesh connectivity based on the face nodes, since boundary nodes can be shared among processors. Therefore, mesh connectivity can only be provided on the face nodes (not on the face centers).

As described already, the data is not stored on the face nodes, but on the face centers. Therefore, we use OpenFOAM functions to interpolate from face centers to face nodes. The following image illustrates the workflow:

![nearest-projection](https://user-images.githubusercontent.com/33414590/55965109-3402b600-5c76-11e9-87eb-0cdb10b55f7b.png)

Data is obtained at the face centers, then interpolated to face nodes. Here, we have provided mesh connectivity and finally, preCICE performs the nearest-projection mapping.
It is important to notice that the target data location is again the face center mesh of the coupling partner. In the standard CHT case, where both data sets are exchanged by a nearest-projection mapping, this leads to two interface meshes (centers and nodes) per participant. Having both the centers and nodes defined, we can skip one interpolation step and read data directly to the centers (cf. picture solver B).

{% note %}
This is implemented for all CHT-related fields mapped with a `consistent` constraint, but it is not implemented for the `FSI` and `FF` modules.
{% endnote %}

### Additional properties for some solvers

Some solvers may not read all the material properties that are required for a coupled simulation. These parameters need to be added in the `preciceDict`.

#### Conjugate heat transfer

For conjugate heat transfer, the adapter assumes that a solver belongs to one of the following categories: _compressible_, _incompressible_, or _basic_. Most of the solvers belong in the _compressible_ category and do not need any additional information. The other two need one or two extra parameters, in order to compute the heat flux.

For **incompressible solvers** (like the buoyantBoussinesqPimpleFoam), you need to add the density and the specific heat in a `CHT` subdictionary of `preciceDict`. For example:

```c++
CHT
{
    rho [ 1 -3  0  0 0 0 0 ] 50;
    Cp  [ 0  2 -2 -1 0 0 0 ] 5;
};
```

For **basic solvers** (like the laplacianFoam), you need to add a constant conductivity:

```c++
CHT
{
    k   [ 1  1 -3 -1 0 0 0 ] 100;
};
```

The value of `k` is connected to the one of `DT` (set in `constant/transportProperties`)
and depends on the density (`rho [ 1 -3  0  0 0 0 0 ]`) and heat capacity (`Cp  [ 0  2 -2 -1 0 0 0 ]`). The relation between them is `DT = k / rho / Cp`.

#### Fluid-structure interaction

The adapter's FSI functionality supports both compressible and incompressible solvers, as well as solid (e.g., solids4Foam) solvers.

For incompressible solvers, it tries to read uniform values for the density and kinematic viscosity (if it is not already available) from the `FSI` subdictionary of `preciceDict`:

```c++
nu              nu [ 0 2 -1 0 0 0 0 ] 1e-03;
rho             rho [1 -3 0 0 0 0 0] 1;
```

Notice that here, in contrast to the `CHT` subdict, we need to provide both the keyword (first `nu`) and the word name (second `nu`). We are working on bringing consistency on this.

#### Fluid-fluid coupling

The FF module provides an option to correct the written velocity values for the face flux values `phi`. This may provide better mass consistency across the coupling interface when the used mesh is skewed. By default, this option is turned off.

```c++
FF
{
  fluxCorrection    true;
  namePhi           phi;
}

```

### Additional parameters in the adapter's configuration file

Some optional parameters can allow the adapter to work with more solvers, whose type is not determined automatically, their fields have different names, or they do not work well with some features of the adapter.

#### User-defined solver type

The adapter tries to automatically determine the solver type,
based on the dictionaries that the solver uses.
However, you may manually specify the solver type to be `basic`,
`incompressible` or `compressible` for a CHT simulation:

```c++
CHT
{
    solverType incompressible;
};
```

or the `incompressible`, `compressible`, or `solid` (e.g., for solids4Foam) for an FSI simulation:

```c++
FSI
{
    solverType solid;
}
```

Note that the adapter [does not currently adapt the name of the pressure used in the computations for compressible solvers](https://github.com/precice/openfoam-adapter/issues/253).

For an FF simulation, known types are `incompressible` and `compressible`:

```c++
FF
{
    solverType incompressible;
}
```

This will force the adapter to use the boundary condition implementations
for the respective type.

#### Parameters and fields with different names

The names of the parameters and fields that the adapter looks for
can be changed, in order to support a wider variety of solvers.
You may specify the following parameters in the adapter's configuration
file (the values correspond to the default values):

```c++
CHT
{
    // Temperature field
    nameT T1;
    // Thermal conductivity
    nameKappa k1;
    // Density
    nameRho rho1;
    // Heat capacity for constant pressure
    nameCp Cp1;
    // Prandtl number
    namePr Pr1;
    // Turbulent thermal diffusivity
    nameAlphat alphat1;
};
```

Similarly for FSI simulations:

```c++
FSI
{
    // Displacement fields
    namePointDisplacement pointD;
    nameCellDisplacement D;
    // Force field
    nameForce Force; // For solids4Foam: solidForce
}
```

Use the option `namePointDisplacement unused;` for solvers that do not create a pointDisplacement field, such as the RBFMeshMotionSolver.

For FF simulations:

```c++
FF
{
  // Velocity
  nameU U;
  // Pressure
  nameP p;
  // Face flux (phi for most sovlers)
  namePhi phi;
  // Temperature
  nameT T;
  // Multiphase variable
  nameAlpha alpha
}
```

Note that the adapter does not automatically adapt the pressure name for solvers that account for [hydrostatic pressure effects](https://www.openfoam.com/documentation/guides/latest/doc/guide-applications-solvers-variable-transform-p-rgh.html). In these cases, you may want to set `nameP p_rgh` to couple `p_rgh`, as `p` is a derived quantity for these solvers.

#### Restarting FSI simulations

Restarting a coupled simulation using the OpenFOAM adapter works in principle in the same way as restarting a standalone OpenFOAM solver. However, the adapter and preCICE define the coupling interface and the interface node locations based on the mesh at the particular time. In case of FSI simulations, the interface deforms over time, which leads to the definition of a deformed interface during the restart. The boolean variable `restartFromDeformed` (`true` by default) allows to account for the previously accumulated interface deformation such that the initial interface configuration (`t = 0`) is completely recovered. The setting here needs to agree with the behavior of the selected solid solver.

```c++
FSI
{
    // Account for previous displacement during a restart
    restartFromDeformed true;
}
```

{% important %}
The option here defines the way the interface mesh is initialized when restarting an FSI simulation in OpenFOAM. In order to restart a coupled simulation, your solid solver needs to be capable of restarting as well. Furthermore, the two participants need to follow the same assumption for the initialization, which for OpenFOAM you can configure with this option. You can find more information about restarting coupled simulations on [Dsicourse](https://precice.discourse.group/t/how-can-i-restart-a-coupled-simulation/675).
{% endimportant %}

#### Debugging

The user can toggle debug messages at [build time](https://precice.org/adapter-openfoam-get.html).

## Coupling OpenFOAM with 2D solvers

The adapter asks preCICE for the dimensions of the coupling data defined in the `precice-config.xml` (2D or 3D). It then automatically operates in either 3D (normal) or 2D (reduced) mode, with z-axis being the out-of-plane dimension. [Read more](https://github.com/precice/openfoam-adapter/pull/96). In 2D mode, the adapter also supports axisymmetric cases.

## Porting your older cases to the current configuration format

In earlier versions of the adapter, we were using a yaml-based configuration format,
with the adapter configuration file usually named as `precice-adapter-config.yml`.
We moved to a OpenFOAM dictionary format in [#105](https://github.com/precice/openfoam-adapter/pull/105),
to reduce the dependencies. You may also find the [tutorials #69](https://github.com/precice/tutorials/pull/69)
to be a useful reference (file changes).
