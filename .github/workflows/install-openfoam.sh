#!/bin/bash -e

# Install OpenFOAM v2406
wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
sudo apt-get update -y
sudo apt-get install -y openfoam2406-dev
