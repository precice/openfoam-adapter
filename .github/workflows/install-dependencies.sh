#!/bin/bash -e

# Install OpenFOAM v2212
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get install openfoam2212-dev

# Install preCICE v2.5.0
wget https://github.com/precice/precice/releases/download/v2.5.0/libprecice2_2.5.0_focal.deb
sudo apt install ./libprecice2_2.5.0_focal.deb
