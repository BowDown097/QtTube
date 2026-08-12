#!/bin/sh -e

ARTIFACTS_DIR="$PWD/artifacts"
BUILD_DIR="$PWD/build"
INSTALL_DIR="$PWD/install"
ROOT_DIR="$PWD"

mkzip() {
    _prev="$PWD"
    _dir="$1"
    _suffix="$2"
    _zip="QtTube-Windows-$ARCH-$VERSION$_suffix.zip"

    cd "$_dir"
    7z a -tzip "$ARTIFACTS_DIR/$_zip"
    cd "$_prev"
}

# portable zip creation
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR" --config $BUILD_TYPE
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR/bin" --component portable
cmd.exe //c "${GITHUB_WORKSPACE}\\.github\\workflows\\scripts\\windows\\package_portable.cmd"
mkzip "$INSTALL_DIR/bin" "-Portable"

# installer creation
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR/bin" --component selfcontained
cd "$INSTALL_DIR/bin"
makensis -NOCD "$BUILD_DIR/program_info/win_install.nsi"
cp "$ROOT_DIR"/*.exe "$ARTIFACTS_DIR/QtTube-Windows-Setup-$ARCH-$VERSION.exe"
