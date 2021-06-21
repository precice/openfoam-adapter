FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update && apt-get -qq install wget

# Install OpenFOAM
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | bash && \
    apt-get -qq install openfoam2012-dev

# Bust the cache: we want that the next steps are always executed
ARG CACHEBUST

# Build the OpenFOAM adapter
USER precice
WORKDIR /home/precice
RUN git clone --depth 1 --branch develop https://github.com/precice/openfoam-adapter.git
WORKDIR /home/precice/openfoam-adapter
RUN /usr/bin/openfoam2012 ./Allwmake
