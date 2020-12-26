---
title: Get the OpenFOAM adapter
permalink: adapter-openfoam-get.html
keywords: adapter, openfoam, building
summary: "Get the code from GitHub and run ./Allwmake. If this fails, look into wmake.log and ldd.log."
---

To build the adapter, you need to install a few dependencies and then execute the `Allwmake` script.

## Dependencies
1. Install [a compatible OpenFOAM distribution](adapter-openfoam-support.html).
2. Install [preCICE](installation-overview.html)
    * By default, the adapter supports preCICE as a shared library. If you want to use preCICE as a static library, check and adjust its dependencies in `Allwmake` (`ADAPTER_PRECICE_DEP`).

## Adapter
3. [Download](https://github.com/precice/openfoam-adapter/archive/master.zip) the adapter _or_ (better) install [git](https://git-scm.com/) and clone this repository: `git clone https://github.com/precice/openfoam-adapter.git`.
    * [Depending on your OpenFOAM version](adapter-openfoam-support.html), you may need to change to a different git branch.
4. Execute the build script: `./Allwmake`.
    * See and adjust the configuration in the beginning of the script first, if needed.
    * Check for any error messages and suggestions at the end.

The `-DADAPTER_DEBUG_MODE` flag inside `ADAPTER_PREP_FLAGS` activates additional debug messages. You may also change the target directory or specify the number of threads to use for the compilation. See the comments in `Allwmake` for more.

In order to upgrade the adapter, or before you build for another OpenFOAM version, run `./Allclean` first. Get the latest version using `git pull`.

Next: [configure and load the adapter](adapter-openfoam-config.html) or [run a tutorial](tutorials.html).

## Troubleshooting

The following are common problems that may appear during building the OpenFOAM adapter if something went wrong in the described steps. Make sure to always check for error messages at every step before continuing to the next.

The `Allwmake` script prints the environment variables it uses in the beginning (as well as in `allwmake.log`) and it writes the building commands in the file `wmake.log`. Afterwards, it checks (using `ldd`) if the library was linked correctly and writes the output to `ldd.log`. **Please check these files and include them in your report if you have need help.**

The most important information in these files is the `-I` and `-L` flags used during compilation and linking.

### Unknown function type `preciceAdapterFunctionObject`

<details markdown="1">
<summary>Did building & linking the adapter succeed? Any errors in wmake.log or ldd.log? Details: (click)</summary>

If in the beginning of the simulation you get the following warning:

```
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

This is a common problem e.g. when installing dependencies in non-system directories. Have a look in the page [linking to preCICE](installation-linking.html).

### Rellocation-related errors

Make sure to build both preCICE as a shared library (i.e. `.so`, not `.a`).
