# preCICE-adapter for the CFD code OpenFOAM

_**Note:** This adapter is under development and currently does not add
any functionality. This README file also corresponds to the current
development of the adapter and is therefore incomplete._

This adapter is developed as part of Gerasimos Chourdakis' master's thesis.
It is based on [previous work](https://github.com/ludcila/CHT-preCICE) by Lucia Cheung ([master's thesis](https://www5.in.tum.de/pub/Cheung2016_Thesis.pdf), in cooperation with [SimScale](https://www.simscale.com/)).

## Build
In order to build this adapter, we need to use an MPI C++ compiler.
To do so, let's create a new target for WMake (build tool used by OpenFOAM):

```bash
cd $WM_DIR/rules
sudo cp -r ${WM_ARCH}${WM_COMPILER} ${WM_ARCH}Mpicc
cd ${WM_ARCH}Mpicc
```

Edit the file `c++` and change the compiler to `mpic++`. For example:

```make
CC          = mpic++ -std=c++11 -m64
```

Note: If your MPI library triggers many "old style cast" warnings, you may
want to change the `-Wold-style-cast` flag to the `-Wno-old-style-cast`.

Now that we have the new target, we have to specify that we want to use it.
For this, run (or put in your `~/.bashrc`) the following:

```bash
export WM_COMPILER=Mpicc
```

Next, simply run `wmake libso` inside the `openfoam-adapter` directory.
The respective command to clean is `wclean`.

## Run
To run this adapter, you must include the following in
the `system/controlDict` configuration file of the case:

```c++
functions
{
    preCICE_Adapter
    {
        type preciceAdapterFunctionObject;
        libs ("libpreciceAdapterFunctionObject.so");
    }
}
```
This directs the solver to use the `preciceAdapterFunctionObject` function object,
which is part of the `libpreciceAdapterFunctionObject.so` shared library
(which you built as shown above).
The name `preCICE_Adapter` can be different.

You must also provide a configuration file (see below).

## Configuration
The adapter is configured via the file `precice-adapter-config.yml`, which
needs to be present in the case directory. This file is a YAML file with the
following form:

```yaml
participant: Fluid

precice-config-file: ../precice-config.xml

interfaces:
- mesh: Fluid-Mesh
  patches: [interface]
  write-data: Temperature
  read-data: Heat-Flux

subcycling: Yes

checkpointing: Yes
```

The `participant` needs to be the same as the one specified in the `precice-config-file`,
which is the main preCICE configuration file.

In the `interfaces`, a list with the coupled interfaces is provided.
The `mesh` needs to be the same as the one specified in the `precice-config-file`.
The `patches` specifies a list of the names of the OpenFOAM boundary patches that are
participating in the coupled simulation. These need to be defined in the files
included in the `0/` directory. The values for `write-data` and `read-data` are
currently placeholders.

The parameters `subcycling` and `checkpointing` expect a `Yes` or a `No`.
The first one allows the solver to subcycle, while the second enables the
checkpointing. See the preCICE documentation for more information on this.

## Setup an example
The adapter currently only prints information about the calls to the
functionObject's methods and does not add any useful functionality.

For now, you may use it e.g. with the solver buoyantPimpleFoam.
An easy way to test it is by adding the call to the preciceAdapterFunctionObject
in the "Hot Room" tutorial, provided with OpenFOAM.

1. Copy the tutorial to your case directory:
```
$ cp -r $FOAM_TUTORIALS/heatTransfer/buoyantPimpleFoam/hotRoom/ $FOAM_RUN/
```

2. Modify the `system/controlDict` file and add at the end:
```c++
functions
{
    preCICE_Adapter
    {
        type preciceAdapterFunctionObject;
        libs ("libpreciceAdapterFunctionObject.so");
    }
}
```

3. Add the configuration file `precice-adapter-config.yml` in the case directory.

4. Set the boundary patches that are participating in the coupled simulation.
This needs to be done both in the files in the `0/` directory and in the adapter's
configuration file.

5. Run the case:
```
$ ./Allrun
```

The solver outputs information into the `log.buoyantPimpleFoam` log file.
If everything went well, you should be able to find lines like the following in it:
```
---[preciceAdapter] CONSTRUCTOR --------
```

## Compatibility

### Compatible OpenFOAM versions

The following OpenFOAM versions and solvers have been tested to work with this adapter:

* OpenFOAM 5.0 - openfoam.org
* OpenFOAM 4.1 - openfoam.org

The following versions are known to be currently _incompatible_:

* OpenFOAM 3.0.1 - openfoam.org (differences in the `functionObject`)
* OpenFOAM 2.3.1 - openfoam.org (differences in the `functionObject`)

### Compatible OpenFOAM solvers

The following OpenFOAM solvers (in the respective OpenFOAM versions) are known to work with the adapter. However, more solvers may be compatible.

#### Heat Transfer

* buoyantPimpleFoam (OF4, OF5)

### Compatible preCICE versions

The preCICE version corresponding to the commit [284c466](https://github.com/precice/precice/commit/284c466e93ac5a63ebf3a13ecf04a6e8b325a794) (July 25, 2017) is known to work with the adapter.

Please note that, if you are using preCICE as a shared library, you need
to have it added in your `LD_LIBRARY_PATH`.
