#!/bin/sh -e

case "$(uname -s)" in
    Linux*)
        INSTALL_DIR="/usr"
        ;;
    Darwin*)
        set -- "$@" -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
        INSTALL_DIR="$PWD/install"
        ;;
    *)
        INSTALL_DIR="$PWD/install"
        ;;
esac

cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DQTTUBE_ENABLE_LTO=ON "$@"
