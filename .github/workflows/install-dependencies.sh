#!/bin/bash -e

# Install OpenFOAM v2012
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get install openfoam2012-dev

# Install preCICE v2.2.0
wget https://github.com/precice/precice/releases/download/v2.2.0/libprecice2_2.2.0_focal.deb
sudo apt install ./libprecice2_2.2.0_focal.deb