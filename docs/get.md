---
title: Get the OpenFOAM adapter
permalink: adapter-openfoam-get.html
keywords: adapter, openfoam, building
summary: "Get the code from GitHub and run ./Allwmake. If this fails, look into wmake.log and ldd.log."
---

To build the adapter, you need to install a few dependencies and then execute the `Allwmake` script.

1. Install [a compatible OpenFOAM distribution](https://precice.org/adapter-openfoam-support.html).
2. Install [preCICE v3](https://precice.org/installation-overview.html).
    * In case you need preCICE v2, please install an older version of the adapter ([v1.2.3](https://github.com/precice/openfoam-adapter/releases/tag/v1.2.3) was the last release to support preCICE v2).
3. [Download the latest release](https://github.com/precice/openfoam-adapter/releases/latest) for your OpenFOAM version.
4. Execute the build script: `./Allwmake`.
    * See and adjust the configuration in the beginning of the script first, if needed.
    * Check for any error messages and suggestions at the end.

The adapter also requires [pkg-config](https://linux.die.net/man/1/pkg-config) to [link to preCICE](https://precice.org/installation-linking.html). This is a very common dependency on Linux and is usually already installed.

You can set compile flags by either changing the `ADAPTER_PREP_FLAGS` variable in the `Allwmake` script, or directly setting the value of `ADAPTER_PREP_FLAGS`  as an environment variable.
To do so, `export ADAPTER_PREP_FLAGS="-D<desired> -D<options>"` before compiling the adapter.

Adding the `-DADAPTER_DEBUG_MODE` flag to the `ADAPTER_PREP_FLAGS` activates additional debug messages. You may also change the target directory or specify the number of threads to use for the compilation. See the comments in `Allwmake` for more.

Adding the `-DADAPTER_ENABLE_TIMINGS` flag to the `ADAPTER_PREP_FLAGS` activates time measurements for several regions of the adapter, printed at the end of the simulation output (available since v1.2.0).

If you are building the adapter often, you may want to build it in parallel. You can set the environment variable `WM_NCOMPPROCS` to the number of parallel threads you want WMake to use.

Next: [configure and load the adapter](https://precice.org/adapter-openfoam-config.html) or [run a tutorial](https://precice.org/tutorials.html).

## What does the adapter version mean?

We use [semantic versioning](https://semver.org/) (MAJOR.MINOR.PATCH), adapted to the nature of an adapter:

* As "API" we define the tutorial configuration files. If you would need to update your `preciceDict`, `controlDict` or any other configuration file to keep using your simulation cases with the same OpenFOAM version, this would be a new major version.
* If you could run the same cases without any changes, but you would also get new features or modified behavior (non-trivial), then this would be a new minor version.
* If there would be only bugfixes or trivial changes not affecting the configuration or behavior, then this would be a new patch version.

Note that the OpenFOAM version is not part of the version of the adapter. It is only reflected in the release archives, which target a range of compatible versions. By default, we support the latest OpenFOAM version from OpenCFD (openfoam.com) and we update our release archives or release a new adapter version (including more than compatibility changes) as soon as there is a new OpenFOAM version.

Read the [discussion that lead to this versioning strategy](https://github.com/precice/openfoam-adapter/issues/52) for more details.

## Troubleshooting

The following are common problems that may appear during building the OpenFOAM adapter if something went wrong in the described steps. Make sure to always check for error messages at every step before continuing to the next.

The `Allwmake` script prints the environment variables it uses in the beginning (as well as in `Allwmake.log`) and it writes the building commands in the file `wmake.log`. Afterwards, it checks (using `ldd`) if the library was linked correctly and writes the output to `ldd.log`. **Please check these files and include them in your report if you have need help.**

If you don't have access to the log files, you can also try running `foamHasLibrary -verbose precice libpreciceAdapterFunctionObject`, which should lead to the following message:

```text
Can load "precice"
Can load "libpreciceAdapterFunctionObject"
```

If the libraries are available but cannot be loaded, the most common issue is conflicting or missing dependencies. Run `ldd ${FOAM_USER_LIBBIN}/libpreciceAdapterFunctionObject.so` and check for any undefined symbols messages at the bottom.

### Unknown function type `preciceAdapterFunctionObject`

<details markdown="1">
<summary>Did building & linking the adapter succeed? Any errors in wmake.log or ldd.log? Details: (click)</summary>

If in the beginning of the simulation you get the following warning:

```text
Starting time loop

 --> FOAM Warning :
     From function void* Foam::dlOpen(const Foam::fileName&, bool)
     in file POSIX.C at line 1604
     dlopen error : libprecice.so: cannot open shared object file: No such file or directory
 --> FOAM Warning :
     From function bool Foam::dlLibraryTable::open(const Foam::fileName&, bool)
     in file db/dynamicLibrary/dlLibraryTable/dlLibraryTable.C at line 105
     **could not load "libpreciceAdapterFunctionObject.so"**
 --> FOAM Warning :
     From function bool Foam::dlLibraryTable::open(const Foam::dictionary&, const Foam::word&, const TablePtr&) [with TablePtr = Foam::HashTable<Foam::autoPtr<Foam::functionObject> (*)(const Foam::word&, const Foam::Time&, const Foam::dictionary&), Foam::word, Foam:     :string::hash>*]
     in file lnInclude/dlLibraryTableTemplates.C at line 62
     Could not open library "libpreciceAdapterFunctionObject.so"

 --> FOAM Warning :
 Unknown function type preciceAdapterFunctionObject
```

then this probably means that something went wrong while building the OpenFOAM adapter. Check the files `wmake.log` (for building errors) and `ldd.log` (for runtime linking errors). Make sure that, when you run the simulation, you have the same OpenFOAM and any other required environment variables as when you built the adapter.

If everything during building has gone well, the adapter must be installed into your `$FOAM_USER_LIBBIN` directory. Check that it exists (`ls $FOAM_USER_LIBBIN`) and that `ldd $FOAM_USER_LIBBIN/libpreciceAdapterFunctionObject.so` does not return any errors.

Note that the simulation will continue without loading the adapter and there will be no coupling.
</details>

### wmkdep: could not open file X

This is an info/warning message that is printed when WMake tries to distinguish between the object files it already has (and can save time by not recompiling them) and the files it needs to compile. You can safely ignore this message.

### A header file cannot be found (during compilation)

This is a common problem e.g. when installing dependencies in non-system directories. Have a look in the page [linking to preCICE](https://precice.org/installation-linking.html).

### Rellocation-related errors

Make sure to build both preCICE as a shared library (i.e. `.so`, not `.a`).

### Undefined symbols from FFTW

When building the adapter, it may fail to at the very end, reporting the following in the `ldd.log`:

```text
undefined symbol: fftw_taint    (/lib/x86_64-linux-gnu/libfftw3_mpi.so.3)
undefined symbol: fftw_join_taint    (/lib/x86_64-linux-gnu/libfftw3_mpi.so.3)
```

This seems to always be related to building OpenFOAM from source, while also already having FFTW (an OpenFOAM dependency) installed. Removing FFTW from the `ThirdParty` directory of the OpenFOAM source code, and running `Allwmake` in OpenFOAM (and then also in the adapter) should help. This should also be very fast, as it will only relink, not rebuild.
