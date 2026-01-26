#!/bin/bash

## these environment variables are expected to be available:
### these should be readily available in the Actions environment:
## VERSION
## BUILD_TYPE
## BUILD_DIR
## INSTALL_APPIMAGE_DIR
## QT_ROOT_DIR
### these need to be supplied manually:
## LIBS_ROOT_DIR
## QT_VER (matrix.qt_ver)
## RUNNER_OS (runner.os)
## SOURCE_DIR


export EXTRA_QT_MODULES="opengl;positioning;printsupport;qml;qmlmodels;qmlworkerscript;quick;quickwidgets;sql;svg;waylandcompositor;webchannel;webenginecore;webenginewidgets" # everything but sql, svg, and waylandcompositor should be auto-detected but aren't, because LTO is enabled
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"
export LDAI_OUTPUT="QtTube-$RUNNER_OS-$VERSION-Qt$QT_VER-$BUILD_TYPE-x86_64.AppImage"
export LINUXDEPLOY_EXCLUDED_LIBRARIES="libmysqlclient.so*;libqsqlmimer.so;libqsqlmysql.so;libqsqlodbc.so;libqsqlpsql.so"

cmake --install $BUILD_DIR --prefix $INSTALL_APPIMAGE_DIR/usr
cmake --install $BUILD_DIR --prefix $INSTALL_APPIMAGE_DIR/usr/bin --component selfcontained

mkdir -p "$INSTALL_APPIMAGE_DIR/usr/lib"
mkdir -p "$INSTALL_APPIMAGE_DIR/usr/resources"

libs_to_copy=(
    libcrypto.so.3
    libssl.so.3
    'libicu*.so.*'
    # https://packages.ubuntu.com/noble/amd64/libnss3/filelist
    libfreebl3.chk
    libfreebl3.so
    libfreeblpriv3.chk
    libfreeblpriv3.so
    libnss3.so
    libnssckbi.so
    libnssdbm3.chk
    libnssdbm3.so
    libnssutil3.so
    libsmime3.so
    libsoftokn3.chk
    libsoftokn3.so
    libssl3.so
)

for pattern in "${libs_to_copy[@]}"; do
    eval "for file in $LIBS_ROOT_DIR/$pattern; do cp -L \$file $INSTALL_APPIMAGE_DIR/usr/lib; done"
done

cp -L $QT_ROOT_DIR/lib/libQt6QuickWidgets.so.$QT_VER $INSTALL_APPIMAGE_DIR/usr/lib
cp -L $QT_ROOT_DIR/lib/libQt6WebEngineWidgets.so.$QT_VER $INSTALL_APPIMAGE_DIR/usr/lib

./linuxdeploy-x86_64.AppImage \
    --appdir "$INSTALL_APPIMAGE_DIR" \
    --output appimage \
    --plugin qt \
    --custom-apprun "$SOURCE_DIR/program_info/AppRun" \
    -d "$SOURCE_DIR/program_info/qttube.desktop" \
    -i "application/res/qttube.svg"
