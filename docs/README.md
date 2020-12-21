---
title: The OpenFOAM adapter
permalink: adapter-openfoam-overview.html
keywords: adapter, openfoam, cite, versions
summary: An OpenFOAM function object for CHT, FSI, and fluid-fluid coupled simulations using preCICE.
---

This adapter is a plug-in (function object) for OpenFOAM, which can work with any recent version of OpenFOAM (.com / .org, version-specific branches for the latter). It supports fluid-structure interaction, conjugate heat transfer, and fluid-fluid simulations, while it is also [easily extensible](adapter-openfoam-extend.html).

This adapter supports all features of preCICE, including implicit coupling and nearest-projection mapping. It can also act as a 2D solver, defining only one layer of interface nodes (automatically).

## How to cite

We are currently working on an up-to-date reference paper. Until then, please cite this adapter using:
```
Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.
```

## Related literature

[1] Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.

[2] Lucia Cheung Yau. Conjugate heat transfer with the multiphysics coupling library preCICE. Master’s thesis, Department of Informatics, Technical University of Munich, 2016.

[3] Derek Risseeuw. Fluid Structure Interaction Modelling of Flapping Wings. Master's thesis, Faculty of Aerospace Engineering, Delft University of Technology, 2019.


{% include disclaimer.html content="This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks." %}



