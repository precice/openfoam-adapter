# preCICE-adapter for the CFD code OpenFOAM
<a style="text-decoration: none" href="https://travis-ci.org/precice/openfoam-adapter" target="_blank">
    <img src="https://travis-ci.org/precice/openfoam-adapter.svg?branch=master" alt="Build status">
</a>

_**Note:** This adapter is currently in a testing phase. Please [report any issues](https://github.com/precice/openfoam-adapter/issues) here and give us feedback through the [preCICE mailing list](https://mailman.informatik.uni-stuttgart.de/mailman/listinfo/precice)._

This adapter was developed as part of [Gerasimos Chourdakis' master's thesis](https://www5.in.tum.de/pub/Chourdakis2017_Thesis.pdf) [1].
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www5.in.tum.de/pub/Cheung2016_Thesis.pdf) [2], in cooperation with [SimScale](https://www.simscale.com/)).

The fluid-structure interaction module was developed in close collaboration between Gerasimos Chourdakis and Derek Risseeuw (TU Delft), in the context of the master's thesis of the latter (in progress). We would also like to thank David Schneider (Univ. Siegen / TUM) and Maximilian Müller (TU Braunschweig) for sharing the code and experience of their similar previous work.

The adapter is [easily extensible](https://github.com/precice/openfoam-adapter/wiki/How-to-extend-the-adapter) and support for fluid-fluid multi-model coupling [is planned](https://github.com/precice/openfoam-adapter/issues/60).

## Start here

Our [wiki](https://github.com/precice/openfoam-adapter/wiki) will help you start. 

You may find OpenFOAM-only tutorials here, in order to test your installation, or cases involving other adapters in the [precice/tutorials](https://github.com/precice/tutorials) repository. Step-by-step instructions for them are provided in the [preCICE wiki](https://github.com/precice/precice/wiki).



## References

[1] Gerasimos Chourdakis. A general OpenFOAM adapter for the coupling library preCICE. Master's thesis, Department of Informatics, Technical University of Munich, 2017.

[2] Lucia Cheung Yau. Conjugate heat transfer with the multiphysics coupling library preCICE. Master’s thesis, Department of Informatics, Technical University of Munich, 2016.

## Disclaimer

This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM®  and OpenCFD®  trade marks.
