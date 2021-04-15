# OpenFOAM-preCICE adapter changelog

For more details, look at the [merged pull requests](https://github.com/precice/openfoam-adapter/pulls?q=is%3Apr+is%3Amerged).

## 2021-04-15

- Moved the wiki pages into the `docs/` folder here. These pages are now rendered in the [new adapter documentation](https://precice.org/adapter-openfoam-overview.html).
- Moved all the `tutorials/` to the [tutorials repository](https://github.com/precice/tutorials) and the related documentation to the [preCICE website](https://precice.org/tutorials.html).
- Disabled debug messages by default, to reduce visual noise and logs length. [a942c8](https://github.com/precice/openfoam-adapter/commit/a942c8dc6a9f9ec29f0bb1d6625501657cdd8b65)
- Added a fluid-fluid coupling (FF) module. You can find a new [partitioned pipe tutorial on the website](https://precice.org/tutorials-partitioned-pipe.html). [#67](https://github.com/precice/openfoam-adapter/pull/67)
- Added support for reading displacements on face centers and further improved the 2D mode. [#147](https://github.com/precice/openfoam-adapter/pull/147)
- Added an assertion to restrict the user in valid orientation of 2D cases. [#155](https://github.com/precice/openfoam-adapter/pull/155)
- Improved the `Allwmake` and `Allclean` scripts, dropping support for preCICE 1.x. [#157](https://github.com/precice/openfoam-adapter/pull/157)
- Refactored the `Force` class. [#148](https://github.com/precice/openfoam-adapter/pull/148)
- Cleaned up a few non-const references to boundary fields. [#132](https://github.com/precice/openfoam-adapter/pull/132)

## Earlier changes

This changelog was established in April 2021 and the previous release was on September 2020. Notable changes before that include:

- Added support for stress data. [#125](https://github.com/precice/openfoam-adapter/pull/125)
- Changed the solver type determination to a dimensions-based one.  [#124](https://github.com/precice/openfoam-adapter/pull/124)
- Removed boundary evaluation under checkpointing. [#110](https://github.com/precice/openfoam-adapter/pull/110)
- Updated for preCICE v2. [#117](https://github.com/precice/openfoam-adapter/pull/117)
- Added support for nearest-projection mapping. [#46](https://github.com/precice/openfoam-adapter/pull/46)
- Added support for computing forces in compressible and turbulent flows. [#64](https://github.com/precice/openfoam-adapter/pull/64)
- Added support for fluid-structure interaction (FSI) module. [#56](https://github.com/precice/openfoam-adapter/pull/56)

The first commit to this codebase was on May 28, 2017.