# preCICE-adapter for the CFD code OpenFOAM

This adapter is under development and currently does not add
any functionality.

## Build
In order to build this adapter, simply run `wmake`
inside the `openfoam-adapter` directory. Please notice the "w" in the begining.

## Run
To run this adapter, you must include the following in
the `system/controlDict` configuration file of the case:

```c++
functions
{
    preCICE_Adapter
    {
        type preciceAdapter;
        libs ("libpreciceAdapterFunctionObject.so");
    }
}
```
This directs the solver to use the `preciceAdapter` function object,
which is part of the `libpreciceAdapterFunctionObject.so` shared library
(which you built as shown above).
The name `preCICE_Adapter` can be different.

You also need to set `adjustTimeStep` to `yes` in the same file.

## Setup an example
The adapter currently only prints information about the calls to the
functionObject's methods and does not add any useful functionality.

For now, you may use it e.g. with the solver buoyantPimpleFoam.
An easy way to test it is by adding the call to the preciceAdapter
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
        type preciceAdapter;
        libs ("libpreciceAdapterFunctionObject.so");
    }
}
```

and change the value of `adjustTimeStep` to `yes`.

3. Run the case:
```
$ ./Allrun
```

The solver outputs information into the `log.buoyantPimpleFoam` log file.
If everything went well, you should be able to find lines like the following in it:
```
---[preciceAdapter] CONSTRUCTOR --------
```

## Compatibility

The following OpenFOAM versions have been tested to work with this adapter:

* OpenFOAM 4.1 - openfoam.org
