#!/usr/bin/env bash
set -e
rm -rf bfbs/
rm -rf generated/
rm -rf build/
mkdir build/
pushd build
cmake \
    -DBUILD_KIT=x86_64-linux-gnu \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    ..
make install
popd

snapcraft clean  --destructive-mode
snapcraft --enable-experimental-target-arch --target-arch=amd64 --destructive-mode
