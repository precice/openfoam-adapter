---
title: The OpenFOAM adapter
permalink: adapter-openfoam-overview.html
keywords: adapter, openfoam, cite, versions
summary: An OpenFOAM function object for CHT, FSI, and fluid-fluid coupled simulations using preCICE.
---

## What is this?

This preCICE adapter is a plug-in (function object) for OpenFOAM, which can work with any recent version of OpenFOAM (.com / .org, see [supported OpenFOAM versions](adapter-openfoam-support.html)). It supports fluid-structure interaction (fluid part), conjugate heat transfer (fluid and solid parts), and fluid-fluid simulations, while it is also easily extensible.

## What can it do?

This adapter can read/write the following fields:

- Temperature (read + write)
- Heat flux (read + write)
- Sink temperature (read + write)
- Heat transfer coefficient (read + write)
- Force (write)
- Stress (write)
- Displacement (read)
- Displacement delta (read)
- Pressure (read + write)
- Pressure gradient (read + write)
- Velocity (read + write)
- Velocity gradient (read + write)

All features of preCICE are supported, including implicit coupling and nearest-projection mapping. Even though OpenFOAM is 3D, this adapter can also work in the 2D mode of preCICE, defining only one layer of interface nodes (automatically).

## Try

Here you will find how to [get the adapter](adapter-openfoam-get.html), how to [configure](adapter-openfoam.config.html) a case, how to [extend the adapter](adapter-openfoam-extend.html) to cover additional features, as well as a few notes on [supported OpenFOAM versions](adapter-openfoam-support.html).

## Learn

Apart from following the documentation here, you will also often find us in OpenFOAM-related conferences.
Before diving into preCICE and the OpenFOAM adapter for the first time, you may want to watch the recording of
our [training session from the 15th OpenFOAM Workshop](https://mediatum.ub.tum.de/1551809):

<iframe width="560" height="315" src="https://www.youtube-nocookie.com/embed/INGsFlCW3B8" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

## Cite

We are currently working on an up-to-date reference paper. Until then, please cite this adapter using [1]:

```text
Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.
```

For CHT-specific topics, you may want to additionally look into [2] and for FSI into [3].

### Related literature

[1] Gerasimos Chourdakis. [A general OpenFOAM adapter for the coupling library preCICE](https://mediatum.ub.tum.de/1462269). Master's thesis, Department of Informatics, Technical University of Munich, 2017.

[2] Lucia Cheung Yau. [Conjugate heat transfer with the multiphysics coupling library preCICE](http://www5.in.tum.de/pub/Cheung2016_Thesis.pdf). Master’s thesis, Department of Informatics, Technical University of Munich, 2016.

[3] Derek Risseeuw. [Fluid Structure Interaction Modelling of Flapping Wings](https://repository.tudelft.nl/islandora/object/uuid:70beddde-e870-4c62-9a2f-8758b4e49123). Master's thesis, Faculty of Aerospace Engineering, Delft University of Technology, 2019.

{% include disclaimer.html content="This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks." %}
