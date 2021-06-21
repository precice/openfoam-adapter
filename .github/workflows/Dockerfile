FROM ubuntu:20.04

# Set timezone (to avoid questions in apt-get)
ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get -qq update && apt-get -qq install wget

# Install OpenFOAM
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | bash && \
    apt-get -qq install openfoam2012-dev

# Bust the cache: we want that the next steps are always executed
ARG CACHEBUST

# Build the OpenFOAM adapter
RUN git clone --depth 1 --branch develop https://github.com/precice/openfoam-adapter.git && cd openfoam-adapter
RUN /usr/bin/openfoam2012 ./Allwmake
