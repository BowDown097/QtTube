#!/bin/sh -e

EXTRA_PACKAGES="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/get-debloated-pkgs.sh"

pacman -Syu --noconfirm --overwrite "*" \
    base-devel \
    cmake \
    libxss \
    patchelf \
    qt6-imageformats \
    qt6-multimedia-ffmpeg \
    qt6-svg \
    qt6-tools \
    qt6-webengine \
    wget \
    xorg-server-xvfb \
    zsync

wget --retry-connrefused --tries=30 "$EXTRA_PACKAGES"
chmod +x get-debloated-pkgs.sh
./get-debloated-pkgs.sh ffmpeg-mini icu-mini libxml2-mini qt6-base-mini
