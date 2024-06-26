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

This adapter can read/write the following fields in a surface coupling setup:

- Temperature (read + write)
- Temperature surface-normal gradient (read + write)
- Heat flux (read + write)
- Sink temperature (read + write)
- Heat transfer coefficient (read + write)
- Force (read + write)
- Stress (write)
- Displacement (read + write)
- Displacement delta (read)
- Pressure (read + write)
- Pressure surface-normal gradient (read + write)
- Velocity (read + write)
- Velocity surface-normal gradient (read + write)
- Phase fraction (alpha) (read + write)
- Phase fraction (alpha) gradient (read + write)
- Phase flux (phi) (read + write)

In addition, the adapter supports the following fields in a volume coupling setup:

- Temperature (write)
- Pressure (write)
- Velocity (read + write)

All features of preCICE are supported, including implicit coupling and nearest-projection mapping. Even though OpenFOAM is 3D, this adapter can also work in the 2D mode of preCICE, defining only one layer of interface nodes (automatically).

## Try

Here you will find how to [get the adapter](https://precice.org/adapter-openfoam-get.html), how to [configure](https://precice.org/adapter-openfoam-config.html) a case, how to [extend the adapter](https://precice.org/adapter-openfoam-extend.html) to cover additional features, as well as a few notes on [supported OpenFOAM versions](https://precice.org/adapter-openfoam-support.html).

## Learn

Apart from following the documentation here, you will also often find us in OpenFOAM-related conferences.
Before diving into preCICE and the OpenFOAM adapter for the first time, you may want to watch the recording of
our [training session from the 15th OpenFOAM Workshop](https://mediatum.ub.tum.de/1551809):

<iframe width="560" height="315" src="https://www.youtube-nocookie.com/embed/INGsFlCW3B8" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

## Cite

Please cite this adapter using our [reference paper in the OpenFOAM Journal](https://doi.org/10.51560/ofj.v3.88) [1]. See the [preCICE literature guide](https://precice.org/fundamentals-literature-guide.html) for more details.

## History

This project is actively maintained on [precice/openfoam-adapter](https://github.com/precice/openfoam-adapter). Current maintainers: [@MakisH](https://github.com/MakisH/) and [@DavidSCN](https://github.com/DavidSCN).

This adapter was developed as part of [Gerasimos Chourdakis' master's thesis](https://mediatum.ub.tum.de/1462269) [2].
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www5.in.tum.de/pub/Cheung2016_Thesis.pdf) [3], in cooperation with [SimScale](https://www.simscale.com/)).

The fluid-structure interaction module was developed in close collaboration between Gerasimos Chourdakis and Derek Risseeuw (TU Delft), in the context of the [master's thesis of the latter](http://resolver.tudelft.nl/uuid:70beddde-e870-4c62-9a2f-8758b4e49123) [4]. We would also like to thank David Schneider (Univ. Siegen / TUM) and Maximilian Müller (TU Braunschweig) for sharing the code and experience of their similar previous work.

The fluid-fluid coupling module was added by Gerasimos Chourdakis, in the context of his dissertation. [#67](https://github.com/precice/openfoam-adapter/pull/67)

The adapter is [easily extensible](https://precice.org/adapter-openfoam-extend.html).

### Related literature

[1] Chourdakis, G., Schneider, D., & Uekermann, B. (2023). OpenFOAM-preCICE: Coupling OpenFOAM with External Solvers for Multi-Physics Simulations. OpenFOAM® Journal, 3, 1–25. [DOI: 10.51560/ofj.v3.88](https://doi.org/10.51560/ofj.v3.88)

[2] Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.

[3] Lucia Cheung Yau. Conjugate heat transfer with the multiphysics coupling library preCICE. Master’s thesis, Department of Informatics, Technical University of Munich, 2016.

[4] Derek Risseeuw. Fluid Structure Interaction Modelling of Flapping Wings. Master's thesis, Faculty of Aerospace Engineering, Delft University of Technology, 2019.

{% disclaimer %}
This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks.
{% enddisclaimer %}
