---
title: The OpenFOAM adapter
permalink: adapter-openfoam-overview.html
redirect_from: adapter-openfoam.html
keywords: adapter, openfoam, cite, versions
summary: An OpenFOAM function object for CHT, FSI, and fluid-fluid coupled simulations using preCICE.
---

## What is this?

This preCICE adapter is a plug-in (function object) for OpenFOAM, which can work with any recent version of OpenFOAM (.com / .org, see [supported OpenFOAM versions](https://precice.org/adapter-openfoam-support.html)). It supports fluid-structure interaction (fluid part), conjugate heat transfer (fluid and solid parts), and fluid-fluid simulations, while it is also easily extensible. Besides surface coupling, the adapter also supports volume coupling (overlapping domains).

## What can it do?

This adapter has been demonstrated on different use cases (conjugate heat transfer, fluid-structure interaction, fluid-fluid coupling, CFD-DEM), both in 2D (3D with one layer of cells in the z-axis) and 3D, and both for flow and solid OpenFOAM-based solvers (see [tutorials](https://precice.org/tutorials.html)).
The fields to read/write are provided by different adapter modules that one needs to [configure](https://precice.org/adapter-openfoam-config.html) and are currently the following.

Legend on locations to read/write:

- **N:** Mesh nodes (surface coupling)
- **F:** Face centers (surface coupling)
- **C:** Cell centers (volume coupling)
- ***:** Mesh connectivity supported (for, e.g., nearest-projection mapping)

### Module: Conjugate heat transfer

| Field | Write | Read | Config prefix |
| --- | --- | --- | --- |
| Heat flux | N*, F | N, F* | `Heat-Flux` |
| Heat transfer coefficient | N*, F | N, F* | `Heat-Transfer-Coefficient` |
| Sink temperature | N*, F | N, F* | `Sink-Temperature` |
| Temperature | N*, F, C | N, F*, C | `Temperature` |

All fields are supported for both flow (compressible/incompressible) and basic (e.g., laplacianFoam) solvers.

### Module: Fluid-structure interaction

| Field | Write | Read | Config prefix |
| --- | --- | --- | --- |
| Displacement: absolute | N*, [F*](https://github.com/precice/openfoam-adapter/issues/153) | N*, F* | `Displacement` |
| Displacement: relative | N*, F* | N*, F* | `DisplacementDelta` |
| Force | F* (flow) | F* | `Force` |
| Stress | F* (flow) | F* | `Stress` |

Displacement reading and writing is supported both for flow (compressible or incompressible) and structure solvers.
Force and stress writing in only supported for flow solvers, reading for both.

### Module: Fluid-fluid coupling

| Field | Write | Read | Config prefix |
| --- | --- | --- | --- |
| Drag force | F*, C | F*, C | `DragForce` |
| Momentum: explicit | F*, C | F*, C | `ExplicitMomentum` |
| Momentum: implicit | F*, C | F*, C | `ImplicitMomentum` |
| Phase flux | F* | F* | `Phi` |
| Volume fraction | F*, C | F*, C | `Alpha` |
| Phase fraction gradient | F* | F* | `AlphaGradient` |
| Pressure | F*, C | F*, C | `Pressure` |
| Pressure: full gradient | F*, C* | F* | `PressureGradientFull` (TODO: read-only) |
| Pressure: surface-normal gradient | F* | F* | `PressureGradient` |
| Temperature | F* | F* | `FlowTemperature` |
| Temperature surface-normal gradient | F* | F* | `FlowTemperatureGradient` |
| Velocity | F*, C | F*, C | `Velocity` |
| Velocity surface-normal gradient | F* | F* | `VelocityGradient` |

All fields assume a flow solver.

## Try

Here you will find how to [get the adapter](https://precice.org/adapter-openfoam-get.html), how to [configure](https://precice.org/adapter-openfoam-config.html) a case, how to [extend the adapter](https://precice.org/adapter-openfoam-extend.html) to cover additional features, as well as a few notes on [supported OpenFOAM versions](https://precice.org/adapter-openfoam-support.html).

## Learn

Apart from following the documentation here, you will also often find us in OpenFOAM-related conferences.
Before diving into preCICE and the OpenFOAM adapter for the first time, you may want to watch the recording of
our [training session from the 15th OpenFOAM Workshop](https://mediatum.ub.tum.de/1551809):

<iframe width="560" height="315" src="https://www.youtube-nocookie.com/embed/INGsFlCW3B8" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

## Cite

Please cite this adapter using our [reference paper in the OpenFOAM Journal](https://doi.org/10.51560/ofj.v3.88)[^1]. See the [preCICE literature guide](https://precice.org/fundamentals-literature-guide.html) for more details.

## History

This project is actively maintained on [precice/openfoam-adapter](https://github.com/precice/openfoam-adapter). Current maintainers: [@MakisH](https://github.com/MakisH/) and [@DavidSCN](https://github.com/DavidSCN).

This adapter was developed as part of [Gerasimos Chourdakis' master's thesis](https://mediatum.ub.tum.de/1462269)[^2].
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www.simscale.com/)[^3], in cooperation with [SimScale](https://www.simscale.com/)).

The fluid-structure interaction module was developed in close collaboration between Gerasimos Chourdakis and Derek Risseeuw (TU Delft), in the context of the [master's thesis of the latter](http://resolver.tudelft.nl/uuid:70beddde-e870-4c62-9a2f-8758b4e49123)[^4]. We would also like to thank David Schneider (Univ. Siegen / TUM) and Maximilian Müller (TU Braunschweig) for sharing the code and experience of their similar previous work.

The fluid-fluid coupling module was added by Gerasimos Chourdakis, in the context of his dissertation. [#67](https://github.com/precice/openfoam-adapter/pull/67). Further contributions in this direction by Markus Mühlhäußer ([master's thesis](https://mediatum.ub.tum.de/node?id=1696254&change_language=en)[^5], [related publication](https://mediatum.ub.tum.de/node?id=1732401&change_language=en)[^6]).

The volume coupling functionality was contributed by Tina Vladimirova, in the context of her [interdisciplinary project](https://mediatum.ub.tum.de/1734883)[^7], based on previous work by various community contributors.

The adapter is [easily extensible](https://precice.org/adapter-openfoam-extend.html).

{% disclaimer %}
This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks.
{% enddisclaimer %}

### Related literature

[^1]: Chourdakis, G., Schneider, D., & Uekermann, B. (2023). OpenFOAM-preCICE: Coupling OpenFOAM with External Solvers for Multi-Physics Simulations. OpenFOAM® Journal, 3, 1–25.<br/>
[DOI: 10.51560/ofj.v3.88](https://doi.org/10.51560/ofj.v3.88)

[^2]: Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.<br/>
URL: https://mediatum.ub.tum.de/1462269

[^3]: Lucia Cheung Yau. Conjugate heat transfer with the multiphysics coupling library preCICE. Master’s thesis, Department of Informatics, Technical University of Munich, 2016.<br/>
URL: https://mediatum.ub.tum.de/1461907

[^4]: Derek Risseeuw. Fluid Structure Interaction Modelling of Flapping Wings. Master's thesis, Faculty of Aerospace Engineering, Delft University of Technology, 2019.<br/>
URL: http://resolver.tudelft.nl/uuid:70beddde-e870-4c62-9a2f-8758b4e49123

[^5]: Markus Mühlhäußer. Partitioned flow simulations with preCICE and OpenFOAM. Master's thesis, School of Computation, Information and Technology, Technical University of Munich, 2022.<br/>
URL: https://mediatum.ub.tum.de/node?id=1696254&change_language=en

[^6]: Mühlhäußer, M., Chourdakis, G., & Uekermann, B. (2023). Partitioned flow simulations with preCICE and OpenFOAM. Proceedings of the ECCOMAS Coupled Problems 2023.<br/>
[DOI: 10.23967/c.coupled.2023.014](https://doi.org/10.23967/c.coupled.2023.014)

[^7]: Tina Vladimirova. Design, implementation, and validation of a volume coupling extension for the OpenFOAM-preCICE adapter. IDP report, School of Computation, Information and Technology, Technical University of Munich, 2023.<br/>
URL: https://mediatum.ub.tum.de/1734883
