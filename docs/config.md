---
title: Configure the OpenFOAM adapter
permalink: adapter-openfoam-config.html
keywords: adapter, openfoam, configuration, preciceDict, controlDict
summary: "Write a system/preciceDict, set compatible boundary conditions, and activate the adapter in your system/controlDict."
---

In order to run a coupled simulation, you need to:

1. prepare a preCICE configuration file (described in the [preCICE configuration](https://www.precice.org/configuration-overview.html)),
2. prepare an adapter's configuration file,
3. set the coupling boundaries in the OpenFOAM case,
4. load the adapter, and
5. start all the solvers normally, from the same directory, e.g. in two different terminals.

If you prefer, you may find an already prepared case in our [Tutorial for CHT: Flow over a heated plate](https://precice.org/tutorials-flow-over-heated-plate.html).

You may skip the section _"Advanced configuration"_ in the beginning, as it only concerns special cases. You may also find more details in the [Pull Request #105](https://github.com/precice/openfoam-adapter/pull/105), especially for changes regarding the previous, yaml-based configuration format.

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

The list `modules` can contain `CHT` or/and `FSI` (separated by space).

In the `interfaces`, we specify the coupling interfaces (here only one).
The `mesh` needs to be the same as the one specified in the `preciceConfig`.
The `patches` specifies a list of the names of the OpenFOAM boundary patches that are
participating in the coupled simulation. These need to be defined in the files
included in the `0/` directory. The names of the interfaces (e.g. `Interface1`) are arbitrary and are not used.

The `locations` field is optional and its default value is `faceCenters` (with `faceCentres` also accepted), signifying that the interface mesh is defined on the cell face centers. The alternative option is `faceNodes`, which defines the mesh on the face nodes and is needed e.g. for reading displacements in an FSI scenario.

The values for `readData` and `writeData`
for conjugate heat transfer
can be `Temperature`, `Heat-Flux`, `Sink-Temperature`,
or `Heat-Transfer-Coefficient`. Values like `Sink-Temperature-Domain1` are also allowed.
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
  Sink-Temperature          // e.g. Sink-Temperature-Solid
  Heat-Transfer-Coefficient // e.g. Heat-Transfer-Coefficient-Solid
);

writeData
(
  Sink-Temperature          // e.g. Sink-Temperature-Fluid
  Heat-Transfer-Coefficient // e.g. Heat-Transfer-Coefficient-Fluid
);
```

For fluid-structure interaction, `writeData` can be `Force` or `Stress`, where `Stress` is essentially a force vector scaled by the cell face in spatial coordinates (with any postfix), thus, a conservative quantity as well.`readData` can be `Displacement` and `DisplacementDelta` (with any postfix). `DisplacementDelta` refers to the last coupling time step, which needs to considered in the case of subcycling.

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

* For `readData(Temperature)`, use `type fixedValue` for the `interface` in `0/T`.
OpenFOAM requires that you also give a (redundant) `value`, but the adapter
will overwrite it. ParaView uses this value for the initial time. As a placeholder, you can e.g. use the value from the `internalField`.

```c++
interface
{
    type            fixedValue;
    value           $internalField;
}
```

* For `readData(Heat-Flux)`, use `type fixedGradient` for the `interface` in `0/T`.
OpenFOAM requires that you also give a (redundant) `gradient`, but the adapter will overwrite it.

```c++
interface
{
    type            fixedGradient;
    gradient        0;
}
```

* For `readData(Sink-Temperature)` or `Heat-Transfer-Coefficient`, use
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

* For `readData(Displacement)` or `DisplacementDelta`, you need the following:
  * `type movingWallVelocity` for the interface (e.g. `flap`) in `0/U`,
  * `type fixedValue` for the interface (e.g. `flap`) in the `0/pointDisplacement`, and
  * `solver displacementLaplacian` in the `constant/dynamicMeshDict`.

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

### Load the adapter

To load this adapter, you must include the following in
the `system/controlDict` configuration file of the case:

```c++
functions
{
    preCICE_Adapter
    {
        type preciceAdapterFunctionObject;
        libs ("libpreciceAdapterFunctionObject.so");
    }
}
```

This directs the solver to use the `preciceAdapterFunctionObject` function object,
which is part of the `libpreciceAdapterFunctionObject.so` shared library.
The name `preCICE_Adapter` can be arbitrary.

***

## Advanced configuration

These additional parameters may only concern some users is special cases. Keep reading if you want to use nearest-projection mapping, an incompressible or basic (e.g. laplacianFoam) solver, if you are using a solver with different variable names (e.g. a multiphase solver) or if you are trying to debug a simulation.

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
As already mentioned, the `Fluid` participant does not need to provide the mesh connectivity in case of a standard FSI. Therefore, the `Solid` participant needs to provide it and nothing special needs to be considered compared to other mapping methods. This implementation supports all CHT-related fields, which are mapped with a `consistent` constraint.
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

The adapter's FSI functionality supports both compressible and incompressible solvers.

For incompressible solvers, it tries to read uniform values for the density and kinematic viscosity (if it is not already available) from the `FSI` subdictionary of `preciceDict`:

```c++
nu              nu [ 0 2 -1 0 0 0 0 ] 1e-03;
rho             rho [1 -3 0 0 0 0 0] 1;
```

Notice that here, in contrast to the `CHT` subdict, we need to provide both the keyword (first `nu`) and the word name (second `nu`). We are working on bringing consistency on this.

### Additional parameters in the adapter's configuration file

Some optional parameters can allow the adapter to work with more solvers, whose type is not determined automatically, their fields have different names, or they do not work well with some features of the adapter.

#### User-defined solver type

The adapter tries to automatically determine the solver type,
based on the dictionaries that the solver uses.
However, you may manually specify the solver type to be `basic`,
`incompressible` or `compressible` for a CHT or FSI simulation:

```c++
CHT
{
    solverType incompressible;
};
```

This will force the adapter use the boundary condition implementations
for the respective type.

#### Parameters and fields with different names

The names of the parameters and fields that the adapter looks for
can be changed, in order to support a wider variety of solvers.
You may specify the following parameters in the adapter's configuration
file (the values correspond to the default values):

```c++
CHT
{
   # Temperature field
   nameT T1;
   # Thermal conductivity
   nameKappa k1;
   # Density
   nameRho rho1;
   # Heat capacity for constant pressure
   nameCp Cp1;
   # Prandtl number
   namePr Pr1;
   # Turbulent thermal diffusivity
   nameAlphat alphat1;
};
```

#### Debugging

The adapter also recognizes a few more parameters, which are mainly used in debugging or development.
These are optional and expect a `true` or a `false` value. Some or all of these options may be removed in the future.

The user can toggle debug messages at [build time](https://precice.org/adapter-openfoam-get.html).

## Coupling OpenFOAM with 2D solvers

The adapter asks preCICE for the dimensions of the coupling data defined in the `precice-config.xml` (2D or 3D). It then automatically operates in either 3D (normal) or 2D (reduced) mode, with z-axis being the out-of-plane dimension. [Read more](https://github.com/precice/openfoam-adapter/pull/96).

## Porting your older cases to the current configuration format

In earlier versions of the adapter, we were using a yaml-based configuration format,
with the adapter configuration file usually named as `precice-adapter-config.yml`.
We moved to a OpenFOAM dictionary format in [#105](https://github.com/precice/openfoam-adapter/pull/105),
to reduce the dependencies. You may also find the [tutorials #69](https://github.com/precice/tutorials/pull/69)
to be a useful reference (file changes).
