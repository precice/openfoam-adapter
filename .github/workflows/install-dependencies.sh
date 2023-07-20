#!/bin/bash -e

# Install OpenFOAM v2306
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get update
sudo apt-get install openfoam2306-dev

# Install preCICE v2.5.0
wget https://github.com/precice/precice/releases/download/v2.5.0/libprecice2_2.5.0_jammy.deb
sudo apt install ./libprecice2_2.5.0_jammy.deb
