# preCICE-adapter for the CFD code OpenFOAM

_**Note:** This adapter is currently under active development / not stable_

This adapter is developed as part of Gerasimos Chourdakis' master's thesis.
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www5.in.tum.de/pub/Cheung2016_Thesis.pdf), in cooperation with [SimScale](https://www.simscale.com/)).

## Build
In order to build this adapter, simply run the `Allwmake` script.
The respective `Allclean` script cleans up.

You may need to adjust the location of some libraries and headers
in the `Allwmake` file. The following dependencies are required:

* [yaml-cpp](https://github.com/jbeder/yaml-cpp) headers and shared library.
* preCICE headers and library, as well as the dependencies described in its [Building wiki page](https://github.com/precice/precice/wiki/Building).

You may provide the `-DADAPTER_DEBUG_MODE` flag inside `PREP_FLAGS` to get additional debug messages.
You may also change the target directory or specify the number of threads to use for the compilation.


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

precice-config-file: /path/to/precice-config.xml

interfaces:
- mesh: Fluid-Mesh
  patches: [interface]
  write-data: Temperature
  read-data: Heat-Flux

# Optional, use in special cases
#
# preventEarlyExit: No      # Default: yes
# evaluateBoundaries: No    # Default: yes
# subcycling: No            # Default: yes
# disableCheckpointing: Yes # Default: no

```

The `participant` needs to be the same as the one specified in the `precice-config-file`,
which is the main preCICE configuration file.

In the `interfaces`, a list with the coupled interfaces is provided.
The `mesh` needs to be the same as the one specified in the `precice-config-file`.
The `patches` specifies a list of the names of the OpenFOAM boundary patches that are
participating in the coupled simulation. These need to be defined in the files
included in the `0/` directory. The values for `write-data` and `read-data`
can be strings that contain the sequences `Temperature`, `Heat-Flux`, `Sink-Temperature`,
or `Heat-Transfer-Coefficient`.

The type of the `read-data` needs to be in accordance with the respective boundary
conditions set for each field in the `0/` directory of the case:
* For `read-data: Temperature`, use `type: fixedValue` for the `interface` in `0/T`.
OpenFOAM requires that you also give a `value`, but the adapter
will overwrite it. ParaView uses this value for the initial time.
* For `read-data: Heat-Flux`, use `type: fixedGradient` for the `interface` in `0/T`.
OpenFOAM requires that you also give a `gradient`, but the adapter will overwrite it.
* For `read-data: Sink-Temperature` or `Heat-Transfer-Coefficient`, use
`type: mixed` for the `interface` in `0/T`. OpenFOAM requires that you also give
a `refValue`, a `refGradient`, and a `valueFraction`, but the adapter will overwrite them.

The rest of the parameters are optional and expect a `yes` or a `no`. Use them only in special cases (e.g. debugging or developing).

* `preventEarlyExit: No` prevents the adapter from setting the solver's `endTime` to infinity.
* `evaluateBoundaries: No` prevents the adapter from computing the values on the faces (from the values on the cell centers) after reading a checkpoint.
* `subcycling: No` disallows the subcycling and an error is reported in that case.
* `disableCheckpointing: Yes` prevents the adapter from adding any fields into the list of checkpointed fields.

## Tutorials

In the `tutorials/` directory, you may find ready-to-run examples for OpenFOAM 5.0.

### CHT: Flow over a heated plate

This scenario consists of one fluid and one solid participant,
in this case the solvers `buoyantPimpleFoam` and `laplacianFoam`.
A fluid enters on a channel with temperature 300K, where it comes in contact
with a solid plate, which is heated from below on a constant temperature 310K.

A serial-implicit coupling is used, where the fluid participant reads heat fluxes
and the solid participant reads temperatures. Both participants are executed in
serial. The simulated time is 1s and results are written every 0.2s.

In order to run the example, execute the script `Allrun`. In order to clean
the results, use the script `Allclean`. An example of the visualized
expected results can be found in `overview.png`.

## Compatibility

### Compatible OpenFOAM versions

The following OpenFOAM versions have been tested and are known to work with this adapter:

* OpenFOAM 5.0 - openfoam.org, build 5.x-197d9d3bf20a (30/10/2017)
* OpenFOAM 4.1 - openfoam.org

OpenFOAM-dev from openfoam.org, build dev-6c8102bd9ad3 (04/11/2017) is also known to work.

The following OpenFOAM versions can compile with
this adapter but have not been tested:

* OpenFOAM+ v1706 - openfoam.com

The following versions are known to be currently _incompatible_:

* OpenFOAM 3.0.1 - openfoam.org (planned to support)
* OpenFOAM 2.3.1 - openfoam.org (planned to support)

### Compatible OpenFOAM solvers

The following OpenFOAM solvers (in the respective OpenFOAM versions) are known to work with the adapter. However, more solvers may be compatible.
See also the section "Solver requirements".

#### Heat Transfer

#### Compressible

* buoyantPimpleFoam (OF4.1, OF5.0)
* buoyantSimpleFoam (OF4.1, OF5.0)
* buoyantBoussinesqPimpleFoam (OF4.1, OF5.0)

#### Basic

* laplacianFoam (OF4.1, OF5.0)

### Solver requirements

The adapter can be loaded by any official OpenFOAM solver, but there are some
requirements to use the specific solver for conjugate heat transfer simulations.

First of all, the solver needs to be able to simulate heat transfer. This means
that the solver should create a Temperature field (named `T`) and provide
thermal conductivity or diffusivity fields.

Three categories of solvers are assumed: compressible, incompressible and basic solvers.

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
heat capacity. Therefore, values for these need to be provided.
The adapter looks for them in the `transportProperties` dictionary.

For example, the following lines need to be added in the `constant/transportProperties`
file:
```c++
rho              rho [ 1 -3  0  0 0 0 0 ] 50;
Cp               Cp  [ 0  2 -2 -1 0 0 0 ] 5;
```

The solver itself does not need to read these values.

Assumptions:
* Temperature is a registered IOObject named `T`.
* The dictionaries `turbulenceProperties` and `transportProperties`
are provided.
* `transportProperties` contains density `rho` and heat capacity `Cp`.
* The turbulent thermal diffusivity is a registered IOObject named `alphat`.
If it is not found, then only the laminar part of the thermal diffusivity is
used (a warning is triggered in this case).

#### Basic solvers
For example `laplacianFoam` can simulate heat transfer, using a
thermal diffusion parameter `DT`.
The adapter additionally expects a value for the conductivity `k` in the `transportProperties`
dictionary.

For example, the following lines need to be present in the `constant/transportProperties`
file for the `laplacianFoam`:
```c++
DT               DT  [ 0  2 -1  0 0 0 0 ] 1;
k                k   [ 1  1 -3 -1 0 0 0 ] 100;
```

Do not delete the, already provided in the pure solver, `DT`, as `laplacianFoam` expects it.
The value of `k` is connected to the one of `DT`
and depends on the density (`rho [ 1 -3  0  0 0 0 0 ]`) and heat capacity (`Cp  [ 0  2 -2 -1 0 0 0 ]`). It needs to hold `DT = k / rho / Cp`.
The solver itself does not need to read the additional parameter.

Assumptions:
* Temperature is a registered IOObject named `T`.
* The dictionary `transportProperties` is provided.
* `transportProperties` contains the conductivity `k`.

#### Parameters and fields with different names

The names of the parameters and fields that the adapter looks for
can be changed, in order to support a wider variety of solvers.
You may specify the following parameters in the adapter's configuration
file (the values correspond to the default values):

```yaml
# Temperature field
nameT: T
# transportProperties dictionary
nameTransportProperties: transportProperties
# thermal conductivity
nameKappa: k
# density
nameRho: rho
# heat capacity for constant pressure
nameCp: Cp
# Prandtl number
namePr: Pr
# turbulent thermal diffusivity
nameAlphat: alphat
```

### User-defined solver type

The adapter tries to automatically determine the solver type,
based on the dictionaries that the solver uses.
However, you may manually specify the solver type to be `basic`,
`incompressible` or `compressible`:

```yaml
solverType: compressible
```

This will force the adapter use the boundary condition implementations
for the respective type.

### Notes on OpenFOAM features

#### End of the simulation

Both the solver and preCICE try to control when the simulation should end.
While in an explicit coupling scenario this is clearly defined,
in an implicit coupling scenario the solver may schedule its exit
(and therefore the last call to the adapter) before the coupling is complete.
See [how function objects are called](https://cpp.openfoam.org/v5/Time_8C_source.html#l00781)
for more details on this.

In order to prevent early exits from the solver, the solver's `endTime`
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

The preCICE version corresponding to the commit [5034762](https://github.com/precice/precice/commit/5034762c86ca92ee6f3f0edf2d2a78f75d9b805d) (November 3, 2017) is known to work with the adapter. Newer versions should also be compatible.

Please note that, if you are using preCICE as a shared library, you need
to have it added in your `LD_LIBRARY_PATH`.

### Notes on preCICE features

The adapter is developed with all the features of preCICE in mind. However,
some specific features are not yet fully supported.

#### Nearest-projection mapping

The nearest-projection mapping is currently not supported by the OpenFOAM adapter,
as topological information is not yet provided to preCICE. This will be
implemented in future releases.

#### Checkpointing

In the case of implicit coupling, the participants are required to store
checkpoints of their state. The adapter tracks all the registered objects
of type `volScalarField`, `volVectorField`, `surfaceScalarField` and `surfaceVectorField`.
After reading a checkpoint, the boundaries are evaluated again for all the
tracked `volScalarField` and `volVectorField` objects, to improve the stability.

However, there is a known bug in the current implementation, where trying to
evaluate the boundaries after reading a checkpoint, for some fields, will lead
to an error. This is currently known to happen only for the
`epsilon` field of the kEpsilon turbulence model. In case this field is
available, it is not tracked and a warning is reported. Please let us
know if this happens in any other case.

You may also disable the evaluation of the boundaries after
reading a checkpoint, by using the `evaluateBoundaries: No` option.
Additionally, you may disable the checkpointing completely,
by using the `disableCheckpointing: Yes` option. This tricks preCICE
by not adding any fields to the checkpoints. You may use this only
for development purposes, as implicit coupling should always be used
with checkpointing.

### Notes on configuration

#### Valid combinations of `write-data` and `read-data`

For a Dirichlet-Neumann coupling, the `write-data` and `read-data` can be
either (`Temperature`, `Heat-Flux`) or (`Heat-Flux`, `Temperature`).

For a Robin-Robin coupling, they can be either (`Heat-Transfer-Coefficient`, `Sink-Temperature`)
or (`Sink-Temperature`, `Heat-Transfer-Coefficient`).

Note that these strings need to be present but the names can also contain more characters. You may also use names like `Sink-Temperature-Domain1` and `Sink-Temperature-Domain2` in more complex coupling scenarios.

### How to couple a different variable?

In case you want to couple a different variable, you need to create a new
coupling data user class in the `preciceAdapter::CHT` namespace or in a new one.
Then you need to add an option for it in the configuration part
to add objects of it into the `couplingDataWriters` and `couplingDataReaders`
whenever requested.

There are some `NOTE`s in the files `Adapter.H`, `Adapter.C`, `CHT/CHT.C`, and
`CHT/Temperature.H` to guide you through the process.

_Note:_ make sure to include any additional required libraries in the `LIB_LIBS`
section of the `Make/options`. Since the adapter is a shared library,
another missing library will trigger an "undefined symbol" runtime error.
