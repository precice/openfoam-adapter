# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
Read more details in the issue [#52: Releases and versioning](https://github.com/precice/openfoam-adapter/issues/52).

<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

## [v1.3.1] 2024-07-27

### Fixed

- Fixed looking for velocity object that should first be created by the adapter [#330](https://github.com/precice/openfoam-adapter/pull/330).

### Added

- Added clang-format to the pre-commit hook [#331](https://github.com/precice/openfoam-adapter/pull/331).
- Added OpenFOAM v2406 to the supported versions (earlier versions already support it as well) [#332](https://github.com/precice/openfoam-adapter/pull/332).

## [v1.3.0] 2024-03-20

### Added

- Added a locationType for volume coupling and implemented the respective functionality (over all internal fields) for Pressure and Velocity (FF) and Temperature (FF) [#255](https://github.com/precice/openfoam-adapter/pull/255).
- Added volume coupling over one or multiple domain regions specified by OpenFOAM cellSets (for Pressure and Velocity (FF) and Temperature (FF))[#270](https://github.com/precice/openfoam-adapter/pull/270).
- Added phase variable and flux coupling for interFOAM in the FF module [#308](https://github.com/precice/openfoam-adapter/pull/308).
- Added custom inletOutlet boundary conditions for pressure and velocity to the FF module [#281](https://github.com/precice/openfoam-adapter/pull/281).
- Added temperature and temperature gradient as coupling data to the FF module [#281](https://github.com/precice/openfoam-adapter/pull/281).
- Added option to write velocity that is corrected by the face flux `phi` [#281](https://github.com/precice/openfoam-adapter/pull/281).
- Added a pre-commit hook to check formatting and linting of various aspects [#321](https://github.com/precice/openfoam-adapter/pull/321).
- Added a workflow for triggering system regression tests [#309](https://github.com/precice/openfoam-adapter/pull/309).
- Added citing guidelines, referring to the [new reference article at the OpenFOAM Journal](https://doi.org/10.51560/ofj.v3.88). [#287](https://github.com/precice/openfoam-adapter/pull/287)

### Changed

- Changed the preCICE dependency version to v3. The adapter is not compatible with v2 anymore. [#285](https://github.com/precice/openfoam-adapter/pull/285).
- Changed the default OpenFOAM version to v2312 in the documentation and CI. [#325](https://github.com/precice/openfoam-adapter/pull/325).
- Changed the way mesh connectivity (face triangles) are provided to preCICE, adapting to preCICE v3 [#297](https://github.com/precice/openfoam-adapter/pull/297).
- Renamed the `adjustSolverTimeStep()` method to `adjustSolverTimeStepAndReadData()`, changing the behavior to always read data at the determined time step size [#298](https://github.com/precice/openfoam-adapter/pull/298).
- Changed the build CI workflow to use the setup-precice-action to install preCICE [#299](https://github.com/precice/openfoam-adapter/pull/299).
- Improved building time by grouping together compilation units of each module [#301](https://github.com/precice/openfoam-adapter/pull/301).

## [v1.2.3] 2023-06-14

### Fixed

- Fixed incorrect reading and writing of the FSI-related data buffers, if multiple patches are combined in an interface mesh [commit 846affd](https://github.com/precice/openfoam-adapter/commit/846affdd00ea8024cee98f34d8ad4205fdc83c5f).

## [v1.2.2] 2023-01-03

### Changed

- Updated the default OpenFOAM version in documentation and CI to v2212 [commit 9b2b2](https://github.com/precice/openfoam-adapter/commit/9b2b27fb6d0c9506c109e5a714d64607d4f73565).

### Fixed

- Fixed compatibility with OpenFOAM v2212, removing an unneeded `refCast` [commit e40fe](https://github.com/precice/openfoam-adapter/commit/e40fec1681a85d5147faa3100d21d28c2e205004).

## [v1.2.1] 2022-12-15

### Fixed

- Replaced a `std::make_unique` by `new` in order to remain C++11 compatible and support older compilers [#264](https://github.com/precice/openfoam-adapter/pull/264).

## [v1.2.0] 2022-11-14

### Added

- Added support for a restart from the initial undeformed interface mesh for Fluid solvers in FSI simulations [#224](https://github.com/precice/openfoam-adapter/pull/224).
- Added functionality to allow use of solids4foam with the OpenFOAM adapter (new solver type "solid"). This includes reading forces and writing displacements.
- Enabled mesh motion solvers that do not register a pointDisplacement field (such as the RBFMeshMotionSolver from solids4foam) to work with the adapter [#241](https://github.com/precice/openfoam-adapter/pull/241)
- Added timers for tracking the time spent in the adapter and preCICE (disabled by default) [#256](https://github.com/precice/openfoam-adapter/pull/256).
- Added a warning for trying to build without pkg-config being available and more suggestions for possible problems [#220](https://github.com/precice/openfoam-adapter/pull/220).
- Added more documentation, specifically for configuring the FF module. [#254](https://github.com/precice/openfoam-adapter/pull/254)

### Changed

- Removed the default `-j 4` option from the wmake command. Instead, documented the `WM_NCOMPPROCS` option of OpenFOAM. [#244](https://github.com/precice/openfoam-adapter/pull/244)
- Changed virtual function declarations to explicitly define one (only) of virtual/override/final. If you need to extend a method marked as `final`, please report. [#245](https://github.com/precice/openfoam-adapter/pull/245)
- Changed the xy-plane error to a warning, to support 2D axisymmetric cases. [#246](https://github.com/precice/openfoam-adapter/pull/246)
- OpenFOAM version bumped to v2206 in GitHub Actions (including preCICE v2.3.0 --> v2.5.0) and documentation [#230](https://github.com/precice/openfoam-adapter/pull/230).

### Removed

- Removed the (deprecated since preCICE v2.5.0) call to isReadDataAvailable. [#247](https://github.com/precice/openfoam-adapter/pull/247)

## [v1.1.0] 2022-02-08

### Added

- Added a check for data fields and supported location types of the coupling interface [#206](https://github.com/precice/openfoam-adapter/pull/206).
- Extended the adapter's DEBUG output to print the rank in addition to the message in parallel runs
[#201](https://github.com/precice/openfoam-adapter/pull/201).
- Added a custom build GitHub workflow to check building the adapter with any supported OpenFOAM version [#214](https://github.com/precice/openfoam-adapter/pull/214).
- Added a release pull request template and documented the versioning strategy and release artifact names. [#216](https://github.com/precice/openfoam-adapter/pull/216)
- Added a hint to also cite the adapter, at the end of the coupling. [#218](https://github.com/precice/openfoam-adapter/pull/218)

### Changed

- Removed explicit casting of boundary conditions in the adapter's write function in order to allow more boundary conditions to be compatible with the adapter (e.g. groovyBC) [#195](https://github.com/precice/openfoam-adapter/pull/195).
- Cleaned-up the handling of adding checkpoint fields and replaced various unnecessary copies by references [#209](https://github.com/precice/openfoam-adapter/pull/209).
- OpenFOAM version bumped to v2112 in GitHub Actions (including preCICE v2.2.1 --> v2.3.0) and documentation. GitHub Action clang-format-action switched to its main branch. [#211](https://github.com/precice/openfoam-adapter/pull/211).
- Disabled automatic checking of links in GitHub Actions. This is now a manual workflow [#215](https://github.com/precice/openfoam-adapter/pull/215).

### Fixed

- Fixed a potential memory access issue in the xy-alignment check for parallel 2D cases [#202](https://github.com/precice/openfoam-adapter/issues/202).
- Fixed a bug that was not allowing more than one module at the same time and added an error message for the case when a dataset is not known by any or too many modules. [#197](https://github.com/precice/openfoam-adapter/pull/197).
- Fixed the misplaced data reading in the adapter 'advance' function [#188](https://github.com/precice/openfoam-adapter/pull/188).

## [v1.0.0] 2021-04-29

### Added

- Automatic code formatting with clang-format. [#173](https://github.com/precice/openfoam-adapter/pull/173)
- GitHub actions to build the adapter with OpenFOAM v2012 and to check shell scripts, documentation formatting, and links. [#165](https://github.com/precice/openfoam-adapter/pull/165) [#164](https://github.com/precice/openfoam-adapter/pull/164) [#169](https://github.com/precice/openfoam-adapter/pull/169) [#171](https://github.com/precice/openfoam-adapter/pull/171)
- Status of the project in `README.md`: CI badges, license, maintainers, links to issues. [#167](https://github.com/precice/openfoam-adapter/pull/167)
- Code of conduct, to ensure a safe environment for the community. [#166](https://github.com/precice/openfoam-adapter/pull/166)

### Changed

- Replaced the deprecated `lookupType` and `subDictPtr` calls with `get` and `findDict` (supported in OpenFOAM v1812 and later). [#176](https://github.com/precice/openfoam-adapter/pull/176)
- This Changelog now follows the [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) format.

### Removed

- Automatic triggering of system tests with Travis-CI. We will reimplement this later. [#165](https://github.com/precice/openfoam-adapter/pull/165)

## [pre-versioning-0] 2021-04-15

### Added

- Support for reading displacements on face centers and further improved the 2D mode. [#147](https://github.com/precice/openfoam-adapter/pull/147)
- Restrict the user in valid orientation of 2D cases with an assertion. [#155](https://github.com/precice/openfoam-adapter/pull/155)
- New fluid-fluid coupling (FF) module. You can find a new [partitioned pipe tutorial on the website](https://precice.org/tutorials-partitioned-pipe.html). [#67](https://github.com/precice/openfoam-adapter/pull/67)

### Changed

- Moved the wiki pages into the `docs/` folder here. These pages are now rendered in the [new adapter documentation](https://precice.org/adapter-openfoam-overview.html).
- Moved all the `tutorials/` to the [tutorials repository](https://github.com/precice/tutorials) and the related documentation to the [preCICE website](https://precice.org/tutorials.html).
- Disabled debug messages by default, to reduce visual noise and logs length. [a942c8](https://github.com/precice/openfoam-adapter/commit/a942c8dc6a9f9ec29f0bb1d6625501657cdd8b65)
- Added `-j 4` to `wmake` in `Allwmake` to build with four threads by default. Remove this you face any issues (in which case, let us know). [#162](https://github.com/precice/openfoam-adapter/pull/162)
- Refactored the `Force` class. [#148](https://github.com/precice/openfoam-adapter/pull/148)

### Removed

- Dropped support for preCICE v1.x. [#157](https://github.com/precice/openfoam-adapter/pull/157)

### Fixed

- Improved the `Allwmake` and `Allclean` scripts. [#157](https://github.com/precice/openfoam-adapter/pull/157)
- Cleaned up a few non-const references to boundary fields. [#132](https://github.com/precice/openfoam-adapter/pull/132)

## [ancient-0] 2020-09-02

This changelog was established in April 2021 and the previous release was on September 2020. Notable changes before that and since the beginning of the project include, in arbitrary order:

### Added

- New fluid-structure interaction (FSI) module. [#56](https://github.com/precice/openfoam-adapter/pull/56)
- Support stress data in FSI. [#125](https://github.com/precice/openfoam-adapter/pull/125)
- Support computing forces in compressible and turbulent flows. [#64](https://github.com/precice/openfoam-adapter/pull/64)
- Support nearest-projection mapping. [#46](https://github.com/precice/openfoam-adapter/pull/46)
- Support preCICE v2. [#117](https://github.com/precice/openfoam-adapter/pull/117)

### Changed

- Replaced the YAML-based configuration format with an OpenFOAM dictionary. [#105](https://github.com/precice/openfoam-adapter/pull/105)
- The compressible/incompressible solver type is not determined based on the pressure dimensions.  [#124](https://github.com/precice/openfoam-adapter/pull/124)

### Removed

- Boundary evaluation under checkpointing. [#110](https://github.com/precice/openfoam-adapter/pull/110)

The first commit to this codebase was on May 28, 2017 and the adapter has been used by non-developers since 2018.
