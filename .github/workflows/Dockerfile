ARG PRECICE_TAG=latest
ARG from=ghcr.io/precice/precice:${PRECICE_TAG}
FROM $from

ARG OPENFOAM_EXECUTABLE=openfoam2112
ARG OPENFOAM_ADAPTER_REF=develop
# Install OpenFOAM
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
RUN sudo apt-get -qq install $OPENFOAM_EXECUTABLE-dev

# Bust the cache: we want that the next steps are always executed
ARG CACHEBUST

# Build the OpenFOAM adapter
RUN git clone --depth 1 --branch $OPENFOAM_ADAPTER_REF https://github.com/precice/openfoam-adapter.git
WORKDIR /openfoam-adapter
RUN /usr/bin/$OPENFOAM_EXECUTABLE ./Allwmake
