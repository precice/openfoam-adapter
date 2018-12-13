# Tutorial for a coupled simulation with OpenFOAM and CalculiX

# Setup

The setup for this tutorial is based on the the 2D perpendicular flap in a channel. 

In the [precice/openfoam-adapter](https://github.com/precice/openfoam-adapter). Please refer to [this wiki page](https://github.com/precice/openfoam-adapter/wiki/Tutorial-for-CHT:-Flow-over-a-heated-plate) of the openfoam-adapter for details and references regarding the experimental setup.

## OpenFOAM

See [Download v5.0 | Ubuntu](https://openfoam.org/download/5-0-ubuntu/). Don't forget to also update your `~/.bashrc`! See [Download v5.0 | Ubuntu -> User Configuration](https://openfoam.org/download/5-0-ubuntu/).

## Boundary conditions
The flow in the domain is determined by a fixedValue at the inlet of 10 m/s. The domain is one cell in width and empty conditions are imposed on the front and back. The outlet has a zero pressure specified and a zerogradient for the velocity. The top, bottom and flap are walls with the noslip condition.

On the stuctural side, the flap is clamped at the bottom. For all points, movement in z-direction and rotation about the x- and y-axis is prohibited.

## preCICE + OpenFOAM adapter

**preCICE:** See [preCICE wiki](https://github.com/precice/precice/wiki/Building). If you have problems compiling, see the "Troubleshooting" section below.
**OpenFOAM adapter:** See [OpenFOAM adapter wiki](https://github.com/precice/openfoam-adapter/wiki/Building). If you have problems compiling, see the "Troubleshooting" section below.

## Calculix

ccx 2.13 is used in the creation of this tutorial. 
Calculix sourcecode can be found at [calculix.de](http://www.calculix.de/)

# Running

To start the coupled simulation, run the command 
'Allrun'
and to clean the case, 
'Allclean'.

Alternatively, you can start the separate solvers by running './runFluid' and './runSolid' in seperate terminals. 

## applying changes. 
The nature of the fluid flow can be changed by adapting the velocity boundary conditions. Also the fluid density and viscosity can be changed in the file 'constant/transportProperties'.

The structure is defined by the Young's modulus and the material density. 

## Coupling approach
Simulations which are more demanding (eg higher forces or lower structure stiffness) are better handled using implicit coupling.

The tutorial is setup as a default in explicit coupling mode, but it is easy to select the implicit coupling mode using IQN-ILS. (see []()) 