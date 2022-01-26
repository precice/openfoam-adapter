#!/bin/bash -e

# Install OpenFOAM 8
sudo sh -c "wget -O - https://dl.openfoam.org/gpg.key | apt-key add -"
sudo add-apt-repository http://dl.openfoam.org/ubuntu
sudo apt-get update
sudo apt-get -y install openfoam8

# Install preCICE v2.3.0
wget https://github.com/precice/precice/releases/download/v2.3.0/libprecice2_2.3.0_focal.deb
sudo apt install ./libprecice2_2.3.0_focal.deb
