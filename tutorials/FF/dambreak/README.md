### Compiling and running SWE-SWE subcritical and supercritical

On the `swe-swe_subcritical` and `swe-swe_supercritical` directories, there is a symbolic link to the SWE solver directory (`../swe_solver`), that is added as a submodule. Additionally, each directory contains two scripts `runSWE1` and `runSWE2` that call the `compile` and `run.sh` scripts as sourced from the symbolic directory of the SWE solver.

For running either of these cases, do:

```bash
cd <swe-swe_subcritical || swe-swe_supercritical>
#Open one terminal and execute
./runSWE1
#Open a second terminal and execute
./runSWE2
```

If we only want to compile, and not run, we can execute

 `./compile`

 
