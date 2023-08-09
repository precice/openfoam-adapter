#!/bin/bash -e

# Install OpenFOAM v2306
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get update
sudo apt-get install openfoam2306-dev

# Install preCICE dependencies
sudo apt-get install build-essential cmake libeigen3-dev libxml2-dev libboost-all-dev petsc-dev python3-dev python3-numpy

# Install preCICE from develop
git clone --depth=1 --branch develop https://github.com/precice/precice.git
cd precice
mkdir -p build && cd build/
cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF -Wno-dev ..
make -j "$(nproc)"
rm -fv ./*.deb && make package
sudo apt-get install -y ./libprecice3_*.deb

# Remove generated packages to save space (approx. 70MB)
rm -rfv ./*.deb ./*.tar.gz _CPack_Packages
