FROM ubuntu:20.04

RUN apt-get -qq update && apt-get -qq install cmake

# Install OpenFOAM
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
RUN apt-get -qq install openfoam2012-dev

# Bust the cache: we want that the next steps are always executed
ARG CACHEBUST

# Build the OpenFOAM adapter
RUN git clone --depth 1 --branch develop https://github.com/precice/openfoam-adapter.git && cd openfoam-adapter
RUN /usr/bin/openfoam2012 ./Allwmake
