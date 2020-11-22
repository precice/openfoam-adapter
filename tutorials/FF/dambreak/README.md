# Shallow Water Equations and OpenFOAM with preCICE
In this tutorial, we will be doing simulations using a [SWE solver](https://github.com/TUM-I5/SWE/) and OpenFOAM, using preCICE as the coupling library. The scenarios use [OpenFOAM 7](https://openfoam.org/version/7/), and [Scons](https://scons.org/) as building tool. Make sure to have these two installed.

The repository of this tutorial contains the following cases for partitioned problems:

  1. swe-swe_subcritical
  2. swe-swe_supercritical
  3. swe-of_subcritical
  4. swe-of_supercritical
  5. of-swe_subcritical
  6. of-swe_supercritical
  7. of-of_subcritical
  8. of-of_supercritical

plus the monolithic cases:

  7. of_monolithic_subcritical
  8. of_monolithic_supercritical
  9. swe_monolithic (supercritical and subcritical) (version with adapter)

Additionally, there is one directory `swe_solver` that is a submodule that links to the *SWE solver* repository used in this tutorial. This solver is added to each of the cases from 1 to 8 as a symbolic link, in order to simplify the process and organize the output within each subdirectory accordingly.

The partitioned cases from 1 to 8 use scripts that source the corresponding environment variables to each case, that consequently run the scripts inside the *SWE solver* in the `swe_solver/` directory (from the symbolic link) for starting the computation on the SWE side. For running *interFoam*, each directory contains a subdirectory that has the OpenFOAM configurations with the corresponding setup to each case, and including a script for starting *interFoam* solver. Detailed instructions for each case can be found further down.

The *SWE solver* is written in such a way that binds to each particular case at startup-time, depending on the environmental variables that are set on the scripts.

A short description for running each case can be found further down.

##  Partitioned Cases

#### swe-swe subcritical and supercritical

The  `swe-swe_subcritical` and `swe-swe_supercritical` directories contain two scripts, `runSWE1` and `runSWE2`, that, through the symbolic link and sourcing the corresponding environment variables, call the `compile` and `run.sh` scripts in the *SWE solver*.
For running either of these cases, do:

```bash
# In two different terminals go to the directory
cd <swe-swe_subcritical || swe-swe_supercritical>
# execute in one terminal
./runSWE
# execute in the second terminal
./runInterFoam
```

The SWE output will be saved in the SWE_output_< case>.

If we only want to compile, and not run, we can execute

 `./compile`


#### swe-of subcritical and supercritical

Each of the `swe-of_subcritical` and `swe-of_supercritical` directories contain two scripts for launching each solver:

1. `runSWE`  sources the environment variables for these cases, and, through the symbolic link,  calls the `compile` and `run.sh` scripts on the *SWE solver*.
2. `runInterFoam` runs the *interFoam* solver for each case.

For running either of these cases, do:

```bash
# In two different terminals go to the directory
cd <swe-of_subcritical || swe-of_supercritical>
# execute in one terminal
./runSWE
# execute in the second terminal
./runInterFoam
```

SWE output will be saved in the `SWEoutput_swe-of_<case>`, while the *interFoam* output will be saved in the `interFoam_solver_swe-of_<case>`

For deleting the output on both solvers run `./Allclean`. For a complete run on the SWE_solver, execute `./cleanAll_SWE` from the `swe_solver` directory.


#### of-swe subcritical and supercritical

Each of the `of-swe_subcritical` and `of-swe_supercritical` directories contain two scripts for launching each solver:

1. `runSWE`  sources the environment variables for these cases, and, through the symbolic link,  calls the `compile` and `run.sh` scripts on the *SWE solver*.
2. `runInterFoam` runs the interFoam solver for each case.

```bash
# In two different terminals go to the directory
cd <of-swe_subcritical || of-swe_supercritical>
# execute in one terminal
./runSWE
# execute in the second terminal
./runInterFoam
```
The SWE output will be saved in the `SWEoutput_of-swe_<case>` directory, while the *interFoam* output will be saved in the `interFoam_solver_of-swe_<case>`

For deleting the output on both solvers run `./Allclean`. For a complete run on the SWE_solver, execute `./swe_solver/cleanAll_SWE`.

#### of-of supercritical and subcritical

In these directories we will find two subdirectories containing the OpenFOAM configurations, `IF_Left` and `IF_Right`,  corresponding the left and right domains. Output for each domain can be found also here.

For running either of this scenarios, execute the following commands

```bash
# In two different terminals go to the directory
cd <of-of_subcritical || of-of_supercritical>
# execute in one terminal
./runIFLEft
# execute in the second terminal
./runIFRight
```

For cleaning the whole directory, execute `./Allclean`.



## Monolithic Cases

#### swe_monolithic

This is simplified copy of the *SWE solver* without the adapter. Inside of `src/examples/swe_simple.cpp` on line 80, we can choose a subcritical or supercritical scenario.

For running this scenario, execute

```bash
cd swe_monolithic
./compile
./run
```

The output can be found under `/build/output/<case>`. **NOTE** inside the `run` script on line 8, we need to select accordingly the name of the output.

For a complete clean, run `./Allclean`. For only deleting the output, run `./cleanOutput`.

#### of_monolithic subcritical and supercritical

We can find the `of_monolithic_subcritical` and `of_monolithic_supercritical` directories for this scenearios. On each directory, we will find a single script `runInterFoam` for running each case.

For starting the simulation, execute

```bash
cd <of_monolithic_supercritical || of_monolithic_subcritical>
./runInterFoam
```

The output can be found under `interFoam_monolithic_<case>` subdirectory.

Run `./Allclean` for a complete clean of the output
