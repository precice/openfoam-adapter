FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update && apt-get -qq install wget software-properties-common git

# Install OpenFOAM
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | bash && \
    apt-get -qq install openfoam2012-dev

# Create user precice
ARG uid=1000
ARG gid=1000
RUN groupadd -g ${gid} precice \
    && useradd -u ${uid} -g ${gid} -m -s /bin/bash precice
USER precice
WORKDIR /home/precice

# Use bash instead of default sh
SHELL ["/bin/bash", "-c"]

# Bust the cache: we want that the next steps are always executed
ARG CACHEBUST

# Install preCICE v2.2.1 from the Debian package
RUN wget https://github.com/precice/precice/releases/download/v2.2.0/libprecice2_2.2.0_focal.deb && \
    apt-get -qq install ./libprecice2_2.2.1_focal.deb

# Build the OpenFOAM adapter
RUN git clone --depth 1 --branch develop https://github.com/precice/openfoam-adapter.git
WORKDIR /home/precice/openfoam-adapter
RUN /usr/bin/openfoam2012 ./Allwmake
