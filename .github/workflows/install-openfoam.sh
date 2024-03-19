#!/bin/bash -e

# Install OpenFOAM v2312
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get update
sudo apt-get install openfoam2312-dev
