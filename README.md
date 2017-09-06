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

#### Heat Transfer

* buoyantPimpleFoam (OF4, OF5)

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
Still, if the solver exits before the coupling completes, an error
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

The preCICE version corresponding to the commit [284c466](https://github.com/precice/precice/commit/284c466e93ac5a63ebf3a13ecf04a6e8b325a794) (July 25, 2017) is known to work with the adapter.

Please note that, if you are using preCICE as a shared library, you need
to have it added in your `LD_LIBRARY_PATH`.
