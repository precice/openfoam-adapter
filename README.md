# preCICE-adapter for the CFD code OpenFOAM

_**Note:** This adapter is under development and currently only adds limited functionality. This README file also corresponds to the current
development of the adapter and is therefore incomplete._

This adapter is developed as part of Gerasimos Chourdakis' master's thesis.
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www5.in.tum.de/pub/Cheung2016_Thesis.pdf), in cooperation with [SimScale](https://www.simscale.com/)).

## Build
In order to build this adapter, we need to use an MPI C++ compiler.
To do so, let's create a new target for WMake (build tool used by OpenFOAM):

```bash
cd $WM_DIR/rules
sudo cp -r ${WM_ARCH}${WM_COMPILER} ${WM_ARCH}Mpicc
cd ${WM_ARCH}Mpicc
```

Edit the file `c++` and change the compiler to `mpic++`. For example:

```make
CC          = mpic++ -std=c++11 -m64
```

Note: If your MPI library triggers many "old style cast" warnings, you may
want to change the `-Wold-style-cast` flag to the `-Wno-old-style-cast`.

Now that we have the new target, we have to specify that we want to use it.
For this, run (or put in your `~/.bashrc`) the following:

```bash
export WM_COMPILER=Mpicc
```

Next, simply run `wmake libso` inside the `openfoam-adapter` directory.
The respective command to clean is `wclean`.

## Run
To run this adapter, you must include the following in
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
which is part of the `libpreciceAdapterFunctionObject.so` shared library
(which you built as shown above).
The name `preCICE_Adapter` can be different.

You must also provide a configuration file (see below).

## Configuration
The adapter is configured via the file `precice-adapter-config.yml`, which
needs to be present in the case directory. This file is a YAML file with the
following form:

```yaml
participant: Fluid

precice-config-file: ../precice-config.xml

interfaces:
- mesh: Fluid-Mesh
  patches: [interface]
  write-data: Temperature
  read-data: Heat-Flux

# Optional, use in special cases
#
# preventEarlyExit: No # Default: yes
#
# subcycling: No # Default: yes

```

The `participant` needs to be the same as the one specified in the `precice-config-file`,
which is the main preCICE configuration file.

In the `interfaces`, a list with the coupled interfaces is provided.
The `mesh` needs to be the same as the one specified in the `precice-config-file`.
The `patches` specifies a list of the names of the OpenFOAM boundary patches that are
participating in the coupled simulation. These need to be defined in the files
included in the `0/` directory. The values for `write-data` and `read-data` are
currently placeholders.

The parameters `preventEarlyExit` and
`subcycling` expect a `Yes` or a `No`.
The first one gives the complete control
of the simulation's end to the adapter,
while the second one disables the
subcycling. Both may be used in special
situation, in which these features may cause problems.


## Setup an example

TODO: Tutorials and examples will be added later.

## Compatibility

### Compatible OpenFOAM versions

The following OpenFOAM versions and solvers have been tested to work with this adapter:

* OpenFOAM 5.0 - openfoam.org
* OpenFOAM 4.1 - openfoam.org (TODO: last check was a while ago)

The following versions are known to be currently _incompatible_:

* OpenFOAM 3.0.1 - openfoam.org (differences in the `functionObject`)
* OpenFOAM 2.3.1 - openfoam.org (differences in the `functionObject`)

### Compatible OpenFOAM solvers

The following OpenFOAM solvers (in the respective OpenFOAM versions) are known to work with the adapter. However, more solvers may be compatible.

#### Basic

* laplacianFoam (modified to read `k`, `rho`, and `Cp`) (OF4, OF5)

#### Heat Transfer

* buoyantPimpleFoam (OF4, OF5)
* buoyantBoussinesqPimpleFoam (modified to read `rho`, and `Cp`) (OF4, OF5)

### Solver requirements

The adapter can be loaded by any official OpenFOAM solver, but there are some
requirements to use the specific solver for conjugate heat transfer simulations.

First of all, the solver needs to be able to simulate heat transfer. This means
that the solver should create a Temperature field (named `T`) and provide
thermal conductivity or diffusivity fields.

Three categories of solvers are assumed: compresible, incompressible and basic solvers.

#### Compressible turbulent flow solvers
For example `buoyantPimpleFoam` or `buoyantSimpleFoam`. These solvers simulate
heat transfer and compute the effective thermal conductivity automatically.
They include the file `turbulentFluidThermoModel.H` and instantiate
a `compressible::turbulenceModel`. This is needed in the adapter as a part of
the effective conductivity is affected by the turbulence.

Assumptions:
* Temperature is a registered IOObject named `T`.
* The dictionaries `turbulenceProperties` and `thermophysicalProperties`
are provided.

#### Incompressible turbulent flow solvers
For example `buoyantBoussinesqPimpleFoam` or `buoyantBoussinesqSimpleFoam`.
These solvers simulate heat transfer but do not compute the effective thermal
conductivity, as they don't know the density of the fluid or the
specific heat capacity. Therefore,
the solver needs to be modified to read these parameters from the
`transportProperties` dictionary.

The following lines need to be added at the end of the `createFields.H`
file of the solver:

```c++
Info<< "Reading density rho\n" << endl;

dimensionedScalar rho
(
    laminarTransport.lookup("rho")
);

Info<< "Reading specific heat Cp\n" << endl;

dimensionedScalar Cp
(
    laminarTransport.lookup("Cp")
);
```

No other changes are required in the code. If this is an official solver,
adjust the name and the build path in the `Make/files`. Afterwards,
recompile the solver.

_Note:_ if you are trying to build `buoyantBoussinesqPimpleFoam`, keep in mind
that it looks for some source files in the `../buoyantBoussinesqSimpleFoam`
directory.

Assumptions:
* Temperature is a registered IOObject named `T`.
* The dictionaries `turbulenceProperties` and `transportProperties`
are provided.
* `transportProperties` contains density `rho` and specific heat capacity `Cp`.
* The turbulent thermal diffusivity is a registered IOObject named `alphat`.
If it is not found, then only the laminar part of the thermal diffusivity is
used (a warning is triggered in this case).

#### Basic solvers
For example `laplacianFoam`. These solvers can
simulate heat transfer, but they need to also provide a conductivity `k`,
density `rho` and specific heat capacity `Cp`.

In order to read these parameters, the following lines need to be added
in the solver's `createFields.H` file:

```c++
Info<< "Reading conductivity k\n" << endl;

dimensionedScalar k
(
    transportProperties.lookup("k")
);

Info<< "Reading density rho\n" << endl;

dimensionedScalar rho
(
    transportProperties.lookup("rho")
);

Info<< "Reading specific heat Cp\n" << endl;

dimensionedScalar Cp
(
    transportProperties.lookup("Cp")
);
```

Recompile the solver as for the incompressible solvers.

Assumptions:
* Temperature is a registered IOObject named `T`.
* The dictionariy `transportProperties` is provided.
* `transportProperties` contains conductivity `k`, density `rho`,
and specific heat capacity `Cp`.

### Notes on OpenFOAM features

#### End of the simulation

Both the solver and preCICE try to control when the simulation should end.
While in an explicit coupling scenario this is clearly defined,
in an implicit coupling scenario the solver may schedule its exit
(and therefore the last call to the adapter) before the coupling is complete.
See [how function objects are called](https://cpp.openfoam.org/v5/Time_8C_source.html#l00781)
for more details on this.

In order to prevent early exits from the solver, the solver's endTime
is set to infinity and it is later set to the current time when
the simulation needs to end. This has the side effect of not calling
any function object's `end()` method normally, so these are triggered
explicitly at the end of the simulation.

In order to disable this behavior, you may define the option
`preventEarlyExit: No` in the adapter's configuration file.
Still, if the solver exits before the coupling completes, a warning
will be reported.

#### Function Objects

In principle, using other function objects alongside the preCICE adapter
is possible. They should be defined *before* the adapter in the
`system/controlDict`, as (by default and opt-out) the adapter controls when the
simulation should end and explicitly triggers (only) the `end()` methods
of any other function objects at the end of the simulation.
If the `end()` of a function object depends on its `execute()`, then
the latter should have been called before the preCICE adapter's `execute()`.

If you want to test this behavior, you may
also include e.g. the `systemCall` function
object in your `system/controlDict`:

```c++
functions
{

    systemCall1
    {
        type        systemCall;
        libs        ("libutilityFunctionObjects.so");

        executeCalls
        (
            "echo \*\*\* systemCall execute \*\*\*"
        );

        writeCalls
        (
            "echo \*\*\* systemCall write \*\*\*"
        );

        endCalls
        (
            "echo \*\*\* systemCall end \*\*\*"
        );
    }

    preCICE_Adapter
    {
        type preciceAdapterFunctionObject;
        libs ("libpreciceAdapterFunctionObject.so");
    }

}
```

#### Writing results

As soon as OpenFOAM writes the results, it will not try to write again
if the time takes the same value again. Therefore, during an implicit
coupling, we write again when the coupling timestep is complete.

#### Adjustable timestep and modifiable runTime

In the `system/controlDict`, you may optionally specify the
following:

```c++
adjustTimeStep  yes;
maxCo           0.5;

runTimeModifiable yes;
```

The adapter works both with fixed and adjustable timestep
and it supports the `runTimeModifiable` feature.
However, if you set a *fixed timestep* and *runTimeModifiable*,
changing the configured timestep *during the simulation* will
not affect the timestep used. A warning will be shown in this case.

### Compatible preCICE versions

The preCICE version corresponding to the commit [e05fbc0](https://github.com/precice/precice/commit/e05fbc0101021c12eff0c41ef58c16ec75acba54) (October 11, 2017) is known to work with the adapter. Newer versions should also be compatible.

Please note that, if you are using preCICE as a shared library, you need
to have it added in your `LD_LIBRARY_PATH`.

### Notes on preCICE features

The adapter is developed with all the features of preCICE in mind. However,
some specific features are not yet fully supported.

#### Nearest-projection mapping

The nearest-projection mapping is currently not supported by the OpenFOAM adapter,
as topological information is not yet provided to preCICE. This will be
implemented in future releases.

### Notes on configuration

#### Valid combinations of `write-data` and `read-data`

For a Dirichlet-Neumann coupling, the `write-data` and `read-data` can be
either (`Temperature`, `Heat-Flux`) or (`Heat-Flux`, `Temperature`).

For a Robin-Robin coupling, they can be either (`Heat-Transfer-Coefficient`, `Sink-Temperature`)
or (`Sink-Temperature`, `Heat-Transfer-Coefficient`).

#### Debug mode

Additional debug messages can be printed. For performance reasons, these
messages are disabled at compile-time. In order to activate them,
the adapter needs to be built with the `ADAPTER_DEBUG_MODE` defined.
For this, comment-out the respective line in the beginning of the `Adapter.C` file.

### How to couple a different variable?

In case you want to couple a different variable, you need to create a new
coupling data user library in the `preciceAdapter::User` namespace.
Then you need to add an option for it in the configuration reading part
to add it to the `couplingDataWriters` and `couplingDataReaders`
whenever requested.

TODO: More details will be added in the future.

_Note:_ make sure to include any additional required libraries in the `LIB_LIBS`
section of the `Make/options`. Since the adapter is a shared library,
another missing library will trigger an "undefined symbol" runtime error.
