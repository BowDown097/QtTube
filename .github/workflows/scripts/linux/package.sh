#!/bin/sh -e

ARTIFACTS_DIR="$PWD/artifacts"
BUILD_DIR="$PWD/build"
INSTALL_DIR="$PWD/install"

SHARUN="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/quick-sharun.sh"

cmake --install "$BUILD_DIR" --prefix "${INSTALL_DIR}/usr"
cmake --install "$BUILD_DIR" --prefix "${INSTALL_DIR}/usr/bin" --component selfcontained

export ICON="$PWD/application/res/qttube.svg"
export DESKTOP="$BUILD_DIR/program_info/qttube.desktop"
export OPTIMIZE_LAUNCH=1
export DEPLOY_OPENGL=0
export DEPLOY_VULKAN=0
export ADD_HOOKS=""
export OUTPATH="$ARTIFACTS_DIR"
export OUTNAME="QtTube-Linux-$ARCH-$VERSION.AppImage"

curl -L --retry 30 "$SHARUN" -o quick-sharun
chmod a+x quick-sharun

./quick-sharun "${INSTALL_DIR}/usr/bin/qttube" "${INSTALL_DIR}/usr/share/"
cp "${INSTALL_DIR}/usr/bin/selfcontained.txt" ./AppDir/bin/
./quick-sharun --make-appimage
