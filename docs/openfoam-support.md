---
title: OpenFOAM support
permalink: adapter-openfoam-support.html
keywords: adapter, openfoam, support, versions
summary: Recent OpenFOAM.com versions work out-of-the-box. Recent OpenFOAM.org versions are also supported, but you will need a version-specific branch.
---

## How to get OpenFOAM

The easiest way to start is to get binary packages for your Linux distribution. For example, to [get OpenFOAM v2406 on Ubuntu](https://develop.openfoam.com/Development/openfoam/-/wikis/precompiled/debian#precompiled-packages-debianubuntu):

```bash
# Add the signing key, add the repository, update:
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash

# Install OpenFOAM v2406:
sudo apt-get install openfoam2406-dev
```

As these steps change your `.profile`, you need to log out and in again to make OpenFOAM fully discoverable.

## Supported OpenFOAM versions

OpenFOAM is a project with long history and many forks, of which we try to support as many as possible. Since several HPC systems only provide older versions, we try to also support a wide range of versions.

We provide version-specific [release archives](https://github.com/precice/openfoam-adapter/releases/latest) and respective Git branches for:

- OpenCFD / ESI (openfoam.com) - main focus:
  - [OpenFOAM v1812-v2406](https://github.com/precice/openfoam-adapter) or newer
    - OpenFOAM v2212 and newer is only supported since v1.2.2 of the adapter.
  - [OpenFOAM v1612-v1806](https://github.com/precice/openfoam-adapter/tree/OpenFOAMv1806) (not tested)
- OpenFOAM Foundation (openfoam.org) - secondary, consider experimental:
  - [OpenFOAM 10](https://github.com/precice/openfoam-adapter/tree/OpenFOAM10)
    - Several [changes to the tutorials](https://github.com/precice/tutorials/tree/OpenFOAM10) are also needed, read the [discussion](https://github.com/precice/tutorials/pull/283).
    - Same limitations as for OpenFOAM 9.
  - [OpenFOAM 9](https://github.com/precice/openfoam-adapter/tree/OpenFOAM9)
    - Rename `solver` to `motionSolver` in `constant/dynamicMeshDict`.
    - Modify also `residualControl` to `outerCorrectorResidualControl` in `system/fvSolution`.
    - Limitations in adjustable time step size ([#261](https://github.com/precice/openfoam-adapter/issues/261)).
  - [OpenFOAM 8](https://github.com/precice/openfoam-adapter/tree/OpenFOAM8)
  - [OpenFOAM 7](https://github.com/precice/openfoam-adapter/tree/OpenFOAM7)
  - [OpenFOAM 6](https://github.com/precice/openfoam-adapter/tree/OpenFOAM6)
    - Modify also `residualControl` to `outerResidualControl` in `system/fvSolution`.
  - [OpenFOAM 5.x](https://github.com/precice/openfoam-adapter/tree/OpenFOAM5)
  - [OpenFOAM 4.0/4.1](https://github.com/precice/openfoam-adapter/tree/OpenFOAM4) (not tested)

Known not supported versions: OpenFOAM v1606+ or older, OpenFOAM 3 or older, foam-extend (any version).

## Supported OpenFOAM solvers

We support mainstream OpenFOAM solvers such as pimpleFoam and solids4Foam for FSI, buoyantPimpleFoam, buoyantSimpleFoam, and laplacianFoam for CHT, or pimpleFoam and sonicLiquidFoam for FF. Our community has, additionally, tried the adapter with multiple different solvers that support function objects.

## Notes on OpenFOAM features

### End of the simulation

The adapter (by default) ignores the `endTime` set in the `controlDict` and stops the simulation when preCICE says so.

Let's see this with more details. During the simulation, both the solver and preCICE try to control when the simulation should end.
While in an explicit coupling scenario this is clearly defined,
in an implicit coupling scenario the solver may schedule its exit
(and therefore the last call to the adapter) before the coupling is complete.
See [how function objects are called](https://www.openfoam.com/documentation/guides/latest/api/Time_8C_source.html#l00890)
for more details on this.

In order to prevent early exits from the solver, the solver's `endTime`
is set to infinity and it is later set to the current time when
the simulation needs to end. This has the side effect of not calling
any function object's `end()` method normally, so these are triggered
explicitly at the end of the simulation.

### Function Objects

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

### Writing results

As soon as OpenFOAM writes the results, it will not try to write again
if the time takes the same value again. Therefore, during an implicit
coupling, we write again when the coupling timestep is complete.
See also a [relevant issue](https://github.com/precice/openfoam-adapter/issues/34).

### Adjustable timestep and modifiable runTime

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

{% disclaimer %}
This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks.
{% enddisclaimer %}
