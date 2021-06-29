#!/bin/bash -e

# Install OpenFOAM v2106
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get install openfoam2106-dev

# Install preCICE v2.2.1
wget https://github.com/precice/precice/releases/download/v2.2.1/libprecice2_2.2.1_focal.deb
sudo apt install ./libprecice2_2.2.1_focal.deb
