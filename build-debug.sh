#!/usr/bin/env bash
rm application.state 2>/dev/null
mkdir -p solutions/activeConfiguration
rm -rf .cache/
rm -rf generated/
rm -rf build/
mkdir build/
pushd build
cmake \
    -DBUILD_KIT=x86_64-linux-gnu \
    -DCMAKE_BUILD_TYPE=Debug \
    ..
make install
popd