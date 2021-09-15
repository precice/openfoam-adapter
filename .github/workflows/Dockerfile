FROM ghcr.io/precice/precice:latest

# Install OpenFOAM
ENV OPENFOAM_VERSION=2106
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | sudo bash
RUN sudo apt-get -qq install openfoam${OPENFOAM_VERSION}-dev


# Bust the cache: we want that the next steps are always executed
ARG CACHEBUST

# Build the OpenFOAM adapter
RUN git clone --depth 1 --branch develop https://github.com/precice/openfoam-adapter.git
WORKDIR /home/precice/openfoam-adapter
RUN /usr/bin/openfoam${OPENFOAM_VERSION} ./Allwmake
