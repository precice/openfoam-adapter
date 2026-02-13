---
title: OpenFOAM support
permalink: adapter-openfoam-support.html
keywords: adapter, openfoam, support, versions
summary: Recent OpenFOAM.com versions work out-of-the-box. Recent OpenFOAM.org versions are also supported, but you will need a version-specific branch.
---

## How to get OpenFOAM

The easiest way to start is to get binary packages for your Linux distribution. For example, to [get OpenFOAM v2512 on Ubuntu](https://gitlab.com/openfoam/core/openfoam/-/wikis/precompiled/debian):

```bash
# Add the signing key, add the repository, update:
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash

# Install OpenFOAM v2512:
sudo apt-get install openfoam2512-dev
```

As these steps change your `.profile`, you need to log out and in again to make OpenFOAM fully discoverable.

## Supported OpenFOAM solvers

We support mainstream OpenFOAM solvers such as pimpleFoam and solids4Foam for FSI, buoyantPimpleFoam, buoyantSimpleFoam, and laplacianFoam for CHT, or pimpleFoam and sonicLiquidFoam for FF. Our community has, additionally, tried the adapter with multiple different solvers that support function objects.

## Supported OpenFOAM versions

OpenFOAM is a project with long history and many forks, of which we try to support as many as possible. Since several HPC systems only provide older versions, we try to also support a wide range of versions.

{% warning %}
Currently, and since v1.3.0 of the adapter (first one for preCICE v3), only the default variant of the adapter for the OpenCFD / ESI is available. We are working on supporting further versions again.
{% endwarning %}

We provide version-specific [release archives](https://github.com/precice/openfoam-adapter/releases/latest) and respective Git branches for:

- OpenCFD / ESI (openfoam.com) - main focus:
  - [OpenFOAM v1812-v2512](https://github.com/precice/openfoam-adapter) or newer
  - [OpenFOAM v1612-v1806](https://github.com/precice/openfoam-adapter/tree/OpenFOAMv1806) (not tested)
- OpenFOAM Foundation (openfoam.org) - secondary, consider experimental:
  - [OpenFOAM 10](https://github.com/precice/openfoam-adapter/tree/OpenFOAM10)
  - [OpenFOAM 9](https://github.com/precice/openfoam-adapter/tree/OpenFOAM9)
  - [OpenFOAM 8](https://github.com/precice/openfoam-adapter/tree/OpenFOAM8)
  - [OpenFOAM 7](https://github.com/precice/openfoam-adapter/tree/OpenFOAM7)
  - [OpenFOAM 6](https://github.com/precice/openfoam-adapter/tree/OpenFOAM6)
  - [OpenFOAM 5.x](https://github.com/precice/openfoam-adapter/tree/OpenFOAM5)
  - [OpenFOAM 4.0/4.1](https://github.com/precice/openfoam-adapter/tree/OpenFOAM4) (not tested)

Known not supported versions: OpenFOAM v1606+ or older, OpenFOAM 3 or older, foam-extend (any version).

## Availability of OpenFOAM packages

We use the Ubuntu repositories of each OpenFOAM vendor for testing. Quick reference for maintainers:

- OpenCFD / ESI (openfoam.com):
  - [PPA repository](https://dl.openfoam.com/repos/deb/)
  - More details on the [OpenFOAM documentation](https://gitlab.com/openfoam/core/openfoam/-/wikis/precompiled).
- OpenFOAM Foundation (openfoam.org):
  - [PPA repository](https://dl.openfoam.org/ubuntu/)
  - More details on the [OpenFOAM documentation](https://dl.openfoam.org/).
- foam-extend:
  - The project distributes pre-built Debian packages as release artifacts on [SourceForge](https://sourceforge.net/projects/foam-extend/files/).

The following table gives an overview, including versions that we do not support, but which might be useful for future or historical reference.

| version     | 24.04 | 22.04 | 20.04 | 18.04 |
| ---         | ---   | ---   | ---   | ---   |
| (com) v2512 | x     | x     | x     | x     |
| (com) v2506 | x     | x     | x     | x     |
| (com) v2412 | x     | x     | x     | x     |
| (com) v2406 | x     | x     | x     | x     |
| (com) v2312 | x     | x     | x     | x     |
| (com) v2306 | x     | x     | x     | x     |
| (com) v2212 | x     | x     | x     | x     |
| (com) v2206 | -     | x     | x     | x     |
| (com) v2112 | -     | x     | x     | x     |
| (com) v2106 | -     | x     | x     | x     |
| (com) v2012 | -     | x     | x     | x     |
| (com) v2006 | -     | x     | x     | x     |
| (com) v1912 | -     | x     | x     | x     |
| (com) older | -     | -     | -     | -     |
| (org) 12    | x     | x     | x     | -     |
| (org) 11    | x     | x     | x     | -     |
| (org) 10    | -     | x     | x     | x     |
| (org) 9     | -     | x     | x     | x     |
| (org) 8     | -     | -     | x     | x     |
| (org) 7     | -     | -     | x     | x     |
| (org) 6     | -     | -     | -     | x     |
| (org) 5     | -     | -     | -     | -     |
| (org) 4     | -     | -     | -     | -     |
| (ext) 5.0   | -     | x     | x     | -     |
| (ext) 4.1   | -     | -     | -     | x     |

## Differences between OpenFOAM versions

We take into account the following relevant differences between OpenFOAM versions, compared to the latest OpenCFD version. This list is important for maintainers of version-specific ports of the adapter.

### OpenFOAM v1612-v1806

- **clockValue:** The `clockValue.H` header is not available (used in `preciceAdapterFunctionObject.H`).
  - Disable the timers feature, controlled by the `ADAPTER_ENABLE_TIMINGS` preprocessor variable (set in `Allwmake`).
- **dictionary access:** In dictionaries (essentially: in the `preciceDict`), some methods are not available. These are mainly used in the configuration step in `Adapter.C`, but also in `ForceBase.C`.
  - Replace `preciceDict.get<fileName>("...")` with `static_cast<fileName>(preciceDict.lookup("...")`.
  - Replace `preciceDict.get<word>("...")` with `static_cast<word>(preciceDict.lookup("..."))`.
  - Replace `preciceDict.getOrDefault` with `preciceDict.lookupOrDefault`.
  - Replace `preciceDict.get<wordList>("...")` with `static_cast<wordList>(preciceDict.lookup("..."))`.
  - Replace `preciceDict.findDict("...")` with `preciceDict.subDictPtr("...")`.
- **Db access:** In the macro that deals with adding checkpointed fields, some methods are not available.
  - Replace `mesh_.sortedNames<GeomFieldType>()` with `mesh_.lookupClass<GeomFieldType>().sortedToc()`.
  - Replace `mesh_.thisDb().getObjectPtr<GeomFieldType>(obj)` with `&(const_cast<GeomFieldType&>(mesh_.thisDb().lookupObject<GeomFieldType>(obj)))`.
- **Pointers:** `std::unique_ptr` is not available.
  - Replace `std::unique_ptr<T>` with `Foam::autoPtr<T>`.
  - Replace calls to pointer `.reset()`, such as `ForceOwning_.reset(new volVectorField(`, with `ForceOwning_ = new volVectorField(`. Adjust the number of closing parentheses.
  - Replace calls to pointer `.get();`, such as `ForceOwning_.get()`, with `ForceOwning_.ptr()`.

### OpenFOAM 4 and 5

OpenFOAM 4 was [released](https://openfoam.org/release/4/) in June 2016 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-4.x), [Doxygen](https://cpp.openfoam.org/v4)).

OpenFOAM 5 was [released](https://openfoam.org/release/5/) in July 2017 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-5.x), [Doxygen](https://cpp.openfoam.org/v5)).

Same as in OpenFOAM v1612-v1806.

### OpenFOAM 6

OpenFOAM 6 was [released](https://openfoam.org/release/6/) in July 2018 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-6), [Doxygen](https://cpp.openfoam.org/v6)). Compared to OpenFOAM 5:

- **adjustTimeStep:** In the function objects interface, `adjustTimeStep()` was renamed to `setTimeStep()`.
  - Replace `adjustTimeStep()` with `setTimeStep()`, both for the function object and the adapter members.
- **setDeltaT:** The `Time` method `setDeltaT(..., false)` was replaced by `setDeltaTNoAdjust(...)`.
  - Replace `setDeltaT(timestepSolver_, false)` with `setDeltaTNoAdjust(timestepSolver_)`.
- **Case files:**
  - In `system/fvSolution`, replace `residualControl` with `outerResidualControl`.

Related work in the adapter: [PR #33](https://github.com/precice/openfoam-adapter/pull/33).

### OpenFOAM 7

OpenFOAM 7 was [released](https://openfoam.org/release/7/) in July 2019 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-7), [Doxygen](https://cpp.openfoam.org/v7)). Compared to OpenFOAM 6:

- **writeEntry:** The interface for `writeEntry` was changed.
  - Where in the latest adapter (`FF/BoundaryConditions`) you see `this->writeEntry("value", os)`, replace it with `writeEntry(os, "value", *this)`.
  - Similarly, where `this->valueFraction().writeEntry("valueFraction", os);`, replace it with `writeEntry(os, "valueFraction", this->valueFraction())`. Similarly for `this->refValue().writeEntry("refValue", os)`.
- **fileName:** `fileName::DIRECTORY` was renamed to `fileType::directory`.
  - Replace this in the `Adapter.C`.

Related work in the adapter: [PR #91](https://github.com/precice/openfoam-adapter/pull/91).

### OpenFOAM 8

OpenFOAM 8 was [released](https://openfoam.org/release/8/) in July 2020 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-8), [Doxygen](https://cpp.openfoam.org/v8)). Compared to OpenFOAM 7:

- **Function objects:** Function objects changed behavior, and the `execute()` method is now also executed at start. See the [adapter issue #179](https://github.com/precice/openfoam-adapter/issues/179) and [PR #180](https://github.com/precice/openfoam-adapter/pull/180).
  - In the beginning of `preciceAdapterFunctionObject::read()`, add `this->executeAtStart_ = false;`.
- **Thermophysical models:** Some properties were moved from the previous `TurbulenceModels` and `transportModels` to the new `ThermophysicalTransportModels` and `MomentumTransportModels`.
  - In `Make/options`, replace the following include paths:
    - Remove the include paths to `transportModels/incompressible`, `transportModels/compressible`, `transportModels/twoPhaseMixture`, `transportModels/interfaceProperties`, `transportModels/immiscibleIncompressibleTwoPhaseMixture`, `TurbulenceModels/turbulenceModels`, `TurbulenceModels/compressible`, `TurbulenceModels/incompressible`.
    - Add include paths to `ThermophysicalTransportModels`, `MomentumTransportModels/momentumTransportModels`, `MomentumTransportModels/compressible`, `MomentumTransportModels/incompressible`, `transportModels`.
    - Remove the linking paths to `compressibleTurbulenceModels`, `incompressibleTurbulenceModels`.
    - Add linking paths to `thermophysicalTransportModels`, `fluidThermoMomentumTransportModels`, `incompressibleMomentumTransportModels`.
  - In `CHT/KappaEffective.H`:
    - Remove the header inclusions `turbulentFluidThermoModel.H`, `turbulentTransportModel.H`.
    - Include the headers `fluidThermoMomentumTransportModel.H`, `kinematicMomentumTransportModel.H`, `thermophysicalTransportModel.H`.
    - Remove the variable declarations for `turbulence_` (2x), `nameRho_`, `nameCp_`, `namePr_`, `nameAlphat_`, `rho_`, `Cp_`, `Pr_` from the compressible and incompressible variants.
    - In the incompressible variant's constructor signature, remove the `nameRho`, `nameCp`, `namePr`, `nameAlphat`.
    - Add a new declaration: `const Foam::thermophysicalTransportModel& thermophysicalTransportModel_;` for each one of the `turbulence_` removed.
  - In `CHT/KappaEffective.C`:
    - Replace all instances of `turbulence_` with `thermophysicalTransportModel_` and update the respective comments.
    - Replace `compressible::turbulenceModel` and `incompressible::turbulenceModel` with `thermophysicalTransportModel`.
    - Replace `turbulenceModel::propertiesName` with `thermophysicalTransportModel::typeName`.
    - In the incompressible variant, remove all lines of code working with `nameRho_`, `nameCp_`, `namePr_`, `nameAlphat_` from the constructor (which ends up with an empty body) and the `extract()` method. Replace the usage of `kappaEff_temp` with `thermophysicalTransportModel_.kappaEff()().boundaryField()[patchID]`.
    - Replace the `const dictionary& CHTDict` with `const dictionary CHTDict`.
  - In `CHT/HeatFlux.C` and in `CHT/HeatTransferCoefficient.C`:
    - Replace `KappaEff_Incompressible(mesh, nameRho, nameCp, namePr, nameAlphat)` with `KappaEff_Incompressible(mesh)`.
  - In `FSI/ForceBase.H`:
    - Remove the header inclusions `immiscibleIncompressibleTwoPhaseMixture.H`, `turbulentFluidThermoModel.H`, `turbulentTransportModel.H`.
    - Include the headers `fluidThermoMomentumTransportModel.H`, `kinematicMomentumTransportModel.H`.
  - In `FSI/ForceBase.C`, in the `ForceBase::devRhoReff()`:
    - Replace `compressible::turbulenceModel` with `compressible::momentumTransportModel`.
    - Replace `incompressible::turbulenceModel` with `incompressible::momentumTransportModel`.
    - Replace `cmpTurbModel::propertiesName` with `cmpTurbModel::typeName`.
    - Replace `turb(mesh_.lookupObject<>())` with `turb = mesh_.lookupObject<>()` (where `const cmpTurbModel& turb`).
    - Replace `turb.devRhoReff()` with `turb.devTau()`.
    - Replace `turb.devReff()` with `turb.devSigma()`.
    - Replace `U(mesh_.lookupObject<>())` with `U = mesh_.lookupObject<>()` (where `const volVectorField& U`).
  - In `FSI/ForceBase.C`, in the `ForceBase::mu()`:
    - Remove the line `typedef immiscibleIncompressibleTwoPhaseMixture iitpMixture;`.
    - Replace the type `iitpMixture` with `fluidThermo` and rename the respective object from `mixture` to `thermo`.
    - Replace `"mixture"` in the lookups with `basicThermo::dictName`.

Related work in the adapter: [PR #130](https://github.com/precice/openfoam-adapter/pull/130).

### OpenFOAM 9

OpenFOAM 9 was [released](https://openfoam.org/release/9/) in July 2021 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-9), [Doxygen](https://cpp.openfoam.org/v9)). Compared to OpenFOAM 8:

- **Function objects:** The `setTimeStep()` method was removed.
  - Remove the method from the `preciceAdapterFunctionObject.H` and `preciceAdapterFunctionObject.C`. Adjust the comments for the equivalent method in `Adapter.H`.
  - **Impact:** See limitations in adjustable time step size ([#261](https://github.com/precice/openfoam-adapter/issues/261)).
- **Thermophysical models:** Different libraries need to be linked and different headers need to be included:
  - In `Make/options`, remove the linking of `fluidThermoMomentumTransportModels` and `incompressibleMomentumTransportModels`.
  - In `Make/options`, link to `transportModels`, `momentumTransportModels`, `incompressibleMomentumTransportModels`, `compressibleMomentumTransportModels`.
  - In `CHT/KappaEffective.H`, replace the headers `fluidThermoMomentumTransportModel.H` and `kinematicMomentumTransportModel.H` with `momentumTransportModel.H`.
  - In `FSI/ForceBase.H`, replace the header `fluidThermoMomentumTransportModel.H` with `compressibleMomentumTransportModel.H`.
  - In `FSI/ForceBase.C`, add the header `fluidThermo.H`.
  - In `FSI/ForceBase.C`, replace:
    - `compressible::momentumTransportModel` with `compressibleMomentumTransportModel`
    - `incompressible::momentumTransportModel` with `incompressibleMomentumTransportModel`. Further down, replace the usage `const incompressible::momentumTransportModel&` with `const icoTurbModel&`.
- **Triangulation:** In `Adapter.C`:
  - Replace the header `faceTriangulation.H` with `polygonTriangulate.H`.
- **Case files:**
  - In `constant/dynamicMeshDict`, replace `solver` with `motionSolver`.
  - In `system/fvSolution`, replace `residualControl` with `outerCorrectorResidualControl`. This was already renamed to `outerResidualControl` in OpenFOAM 6.

Related work in the adapter: [PR #221](https://github.com/precice/openfoam-adapter/pull/221).

### OpenFOAM 10

OpenFOAM 10 was [released](https://openfoam.org/release/10/) in July 2022 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-10), [Doxygen](https://cpp.openfoam.org/v10)). Compared to OpenFOAM 9:

- **Function objects:** The method `fields()` has been made pure virtual, meaning it needs to be implemented.
  - In `preciceAdapterFunctionObject.H`, implement it as (more of a workaround to avoid implementation):

    ```c++
    virtual wordList fields() const
    {
        return wordList::null();
    }
    ```

- **Physical properties and models:** The `physicalProperties` was replaced by `transportModels`.
  - In `Make/options`, replace `physicalProperties` with `transportModels` in the included paths and in the linked libraries.
  - In `FSI/ForceBase.H`, replace `kinematicMomentumTransportModel.H` with `incompressibleMomentumTransportModel.H`.
  - In `FSI/ForceBase.C`, replace `basicThermo::dictName` with `physicalProperties::typeName`.
- **Fields `V0` and `V00`:** The fields `V0` and `V00` have been removed or renamed (not sure to what).
  - In `Adapter.C`, method `preciceAdapter::Adapter::setupMeshVolCheckpointing()`, guard each of these two fields with a `if (mesh_.foundObject<volScalarField::Internal>("V0"))` (and `if (... ("V00"))`, respectively). Alternatively, remove the implementation of this method altogether.
  - **Impact:** This might lead to stability issues in FSI simulations with implicit coupling. Check your results.
- **Case files:** Several [changes to the tutorials](https://github.com/precice/tutorials/tree/OpenFOAM10) are needed, read the [discussion](https://github.com/precice/tutorials/pull/283).

### OpenFOAM 11

OpenFOAM 11 was [released](https://openfoam.org/release/11/) in July 2023 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-11), [Doxygen](https://cpp.openfoam.org/v11)). We do not yet know which are relevant changes. Please help us port the adapter to OpenFOAM 11.

Related work in the adapter: [PR #310](https://github.com/precice/openfoam-adapter/pull/310).

### OpenFOAM 12

OpenFOAM 12 was [released](https://openfoam.org/release/12/) in July 2024 ([GitHub mirror](https://github.com/OpenFOAM/OpenFOAM-12), [Doxygen](https://cpp.openfoam.org/v12)). We do not yet know which are relevant changes. Please help us port the adapter to OpenFOAM 12.

### foam-extend

We have not yet documented all related changes that would make the adapter work for foam-extend. See the repository [`precice/foam-extend-adapter`](https://github.com/precice/foam-extend-adapter) for the latest state and please contribute to this page.

Related work in the adapter: [PR #302](https://github.com/precice/openfoam-adapter/pull/302).

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
