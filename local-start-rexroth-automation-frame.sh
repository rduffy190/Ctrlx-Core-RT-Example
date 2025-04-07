#!/usr/bin/env bash
# shellcheck disable=SC1091

# Exit immediately if a command exits with a non-zero status.
set -e

# build with debug mode
source build-debug.sh

# Delete bundle cache
rm -rf .cache/

# Print verbose logs
export SNAP_ENV=DEBUG
# Do not change SNAP_NAME
export SNAP_NAME=rexroth-automationcore
export SNAP_VERSION=3.0.0
export SNAP_REVISION=x0
export SNAP_ARCH=amd64
export SNAP=${SNAP_DATA_ROOT}/snap
export SNAP_COMMON=${SNAP_DATA_ROOT}/snap-common
export SNAP_DATA=${SNAP_DATA_ROOT}/snap-data
export SNAP_USER_COMMON=${SNAP_DATA_ROOT}/snap-user-common
export SNAP_USER_DATA=${SNAP_DATA_ROOT}/snap-user-data

# Get architecture, define x-build architecture
arch=$(dpkg --print-architecture)
if [[ ${arch} == "amd64" ]];then
    ubuntu_platform=ubuntu22-gcc-x64
else
    ubuntu_platform=ubuntu22-gcc-aarch64
fi

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:../../bin/oss.celix/${ubuntu_platform}"
export LD_DEBUG=libs
export LD_DEBUG_OUTPUT=./ld_debug.log

../../bin/app.automationcore/${ubuntu_platform}/rexroth-automation-frame generated/config.properties