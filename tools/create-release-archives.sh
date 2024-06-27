#!/bin/bash
# Execute this script from the top-level directory (./tools/create-release-archives.sh)

set -u

remote_name=origin
adapter_version="v1.3.1"

# Declare branch names for which archives will be created
declare -a branches=("master" "OpenFOAMv1806" "OpenFOAM10" "OpenFOAM9" "OpenFOAM8" "OpenFOAM7" "OpenFOAM6" "OpenFOAM5")

mkdir -p release-archives
for i in "${branches[@]}"
    do
    archive_name=openefoam-adapter-"${adapter_version}"-"${i}"
    git archive --format=tar.gz --prefix="${archive_name}"/ "${remote_name}"/"${i}" >"release-archives/${archive_name}".tar.gz
done
