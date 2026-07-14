# Implicit Injectors Library

This library is provided in addition to the OpenFOAM preCICE adapter in order to properly handle implicit coupling of Lagrangian particles via preCICE. Proper implicit coupling of Lagrangian particles involves not only accounting for existing particles in the domain during checkpointing, but also adjusting the particle injection logic for implicit coupling loops within time-windows as well. Particle injection is handled in OpenFOAM via injectionModels. injectionModels lie outside the OpenFOAM solver objectRegistry, so they cannot be directly modified by the existing OpenFOAM preCICE adapter functionObject. Therefore, in order to properly handle checkpoint logic for injectionModels, this independent library was added.

## Source Files

implicitInjectionWrapper.H - a wrapper template class used to override the prepareForNextTimeStep function from the native OpenFOAM InjectionModel class for every InjectionModel type. This creates "implicit" versions of the native OpenFOAM InjectionModel types (implicitPatchInjection, implicitConeNozzleInjection, implicitManualInjection, etc.) that support checkpoint logic from preCICE implicit coupling.
makeImplicitInjectorsKinematic.C - uses macro defined in implicitInjectionWrapper.H to create wrapped template classes for implicitPatchInjection, implicitConeNozzleInjection, and implicitManualInjection types registered to the basicKinematicCloud type
makeImplicitInjectorsKinematicColliding.C - uses macro defined in implicitInjectionWrapper.H to create wrapped template classes for implicitPatchInjection, implicitConeNozzleInjection, and implicitManualInjection types registered to the basicKinematicCollidingCloud type
makeImplicitInjectorsReacting.C - uses macro defined in implicitInjectionWrapper.H to create wrapped template classes for implicitPatchInjection, implicitConeNozzleInjection, and implicitManualInjection types registered to the basicReactingCloud type
makeImplicitInjectorsReactingMultiphase.C - uses macro defined in implicitInjectionWrapper.H to create wrapped template classes for implicitPatchInjection, implicitConeNozzleInjection, and implicitManualInjection types registered to the basicReactingMultiphaseCloud type
makeImplicitInjectorsThermo.C - uses macro defined in implicitInjectionWrapper.H to create wrapped template classes for implicitPatchInjection, implicitConeNozzleInjection, and implicitManualInjection types registered to the basicThermoCloud type

## User-Guide

The successfully built `libimplicitInjectors.so` file should exist in the same location as a successfully built `libpreciceAdapterFunctionObject.so` file. To use one of these implicit-aware injection types the user must do two things:

1. Load the `libimplicitInjectors.so` file as a libarary in your system/controlDict file so the solver is aware of the existence of these implicit-aware injectors. To do so, add the following just prior to your "functions" entries:
    ```bash
    libs
    (
        "libimplicitInjectors.so"
    );
    ```

2. Define the implicit injector type in your cloud properties file (constant/KinematicCloudProperties, constant/reactingCloud1Properties, etc.) Particle injector types are usually defined within this file in the subModels.InjectionModels.{modelName} sub-dictionary. Define the type to be either implicitPatchInjection, implicitConeNozzleInjection, or implicitManualInjection.

## Disclaimer

This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks.
