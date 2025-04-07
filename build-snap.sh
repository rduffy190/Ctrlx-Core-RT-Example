#!/usr/bin/env bash
# Exit immediately if a command exits with a non-zero status.
set -e

arch=$(dpkg --print-architecture)
build=release
verbosity=verbose

usage="Usage: build-snap.sh [options]
Build with cmake and snap. 
Options:
  --arch=<architecture>     Architecture is either 'amd64' or 'arm64'.
                            Default is the result of 'dpkg --print-architecture'.
  --build=<build type>      Build type is either 'debug' or 'release'.
                            Default is 'release'.
  --verbosity=<level>       Set the verbosity level of snapcraft: 'brief', 'verbose', 'debug' or 'trace'.
                            Default is 'verbose'.
  --help                    Display this information.
"


for arg in "$@"; do
    case "$arg" in
    --arch=*) arch="${arg#*=}" ;;
    --build=*) build="${arg#*=}" ;;
    --verbosity=*) verbosity="${arg#*=}" ;;
    --help)
        printf "$usage"
        exit 1
        ;;
    esac
done

echo "-------------------------------------------------------"
echo "Print options ..."
echo "-------------------------------------------------------"
echo "architecture: ${arch}"
echo "  build type: ${build}"
echo "   verbosity: ${verbosity}"

echo "-------------------------------------------------------"
echo "Build with cmake presets ..."
echo "-------------------------------------------------------"
sudo rm -rf ./build
sudo rm -rf ./generated
cmake --preset ${arch}-${build}
cmake --build --preset ${arch}-${build}

echo "-------------------------------------------------------"
echo "Snapcraft clean and build  ..."
echo "-------------------------------------------------------"
sudo snapcraft clean
sudo snapcraft --build-for=${arch} --verbosity=${verbosity}

# Remove artifacts (may be wrong architecture and build type)
sudo rm -rf ./build