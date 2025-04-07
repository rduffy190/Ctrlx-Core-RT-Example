#!/usr/bin/env bash
# shellcheck disable=SC1091

# Exit immediately if a command exits with a non-zero status.
set -e

# Supported: amd64, arm64
arch=$(dpkg --print-architecture)

# amd64-debug, arm64-debug
cmake --preset ${arch}-debug
cmake --build --preset ${arch}-debug

source build-config-properties.sh

# Create a link to /tmp/PROJECT_NAME. This is the storage for data generated during local debugging
PROJECT_NAME=$(basename "$PWD")
SNAP_DATA_LINK=./data
export SNAP_DATA_ROOT=/tmp/$PROJECT_NAME
if [ ! -L ${SNAP_DATA_LINK} ] ; then
	ln -sfv "$SNAP_DATA_ROOT" "$SNAP_DATA_LINK"
fi