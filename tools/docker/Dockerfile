ARG PRECICE_TAG=latest
ARG from=precice/precice:${PRECICE_TAG}
FROM $from

ARG OPENFOAM_EXECUTABLE=openfoam2112
# Install OpenFOAM
# Update the package lists prior to adding the openfoam rep. This is needed because the script installs some packages without updating the package lists. 
# So sometimes a mirror moved and we get timeouts. 
RUN apt-get update &&\
    wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | bash &&\
    apt-get -qq install ${OPENFOAM_EXECUTABLE}-dev git

ARG OPENFOAM_ADAPTER_REF=develop
# Build the OpenFOAM adapter
RUN git clone --depth 1 --branch ${OPENFOAM_ADAPTER_REF} https://github.com/precice/openfoam-adapter.git
WORKDIR /openfoam-adapter
RUN /usr/bin/${OPENFOAM_EXECUTABLE} ./Allwmake &&\
    ln -s $(which ${OPENFOAM_EXECUTABLE} ) /usr/bin/openfoam

