# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
A fitting versioning scheme for this project is [under discussion](https://github.com/precice/openfoam-adapter/issues/52).

<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

## [Unreleased]

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
