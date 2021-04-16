# OpenFOAM-preCICE adapter
[![Building (OpenFOAM v2012)](https://github.com/precice/openfoam-adapter/actions/workflows/build.yml/badge.svg)](https://github.com/precice/openfoam-adapter/actions/workflows/build.yml)
<!-- <a style="text-decoration: none" href="https://travis-ci.org/precice/openfoam-adapter" target="_blank">
    <img src="https://travis-ci.org/precice/openfoam-adapter.svg?branch=master" alt="Build status">
</a> -->
<a style="text-decoration: none" href="https://github.com/precice/openfoam-adapter/blob/master/LICENSE" target="_blank">
    <img src="https://img.shields.io/github/license/precice/openfoam-adapter.svg" alt="GNU GPL license">
</a>

## Start here

See the [adapter documentation](https://precice.org/adapter-openfoam-overview.html) and related [tutorials](https://precice.org/tutorials.html).

Please [report any issues](https://github.com/precice/openfoam-adapter/issues) here and give us feedback through the [one of our community channels](https://precice.org/community-channels.html).

## History

This adapter was developed as part of [Gerasimos Chourdakis' master's thesis](https://mediatum.ub.tum.de/1462269) [1].
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www5.in.tum.de/pub/Cheung2016_Thesis.pdf) [2], in cooperation with [SimScale](https://www.simscale.com/)).

The fluid-structure interaction module was developed in close collaboration between Gerasimos Chourdakis and Derek Risseeuw (TU Delft), in the context of the [master's thesis of the latter](http://resolver.tudelft.nl/uuid:70beddde-e870-4c62-9a2f-8758b4e49123) [3]. We would also like to thank David Schneider (Univ. Siegen / TUM) and Maximilian Müller (TU Braunschweig) for sharing the code and experience of their similar previous work.

The fluid-fluid coupling module was added by Gerasimos Chourdakis, in the context of his dissertation. [#67](https://github.com/precice/openfoam-adapter/pull/67)

The adapter is [easily extensible](https://precice.org/adapter-openfoam-extend.html).

## References

[1] Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.

[2] Lucia Cheung Yau. Conjugate heat transfer with the multiphysics coupling library preCICE. Master’s thesis, Department of Informatics, Technical University of Munich, 2016.

[3] Derek Risseeuw. Fluid Structure Interaction Modelling of Flapping Wings. Master's thesis, Faculty of Aerospace Engineering, Delft University of Technology, 2019.

## Disclaimer

This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks.
