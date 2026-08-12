#!/bin/sh -e

qt_path="$RUNNER_WORKSPACE/Qt"

if [[ "$QT_CACHE_HIT" != "true" ]]; then
    qt_archive="$RUNNER_TEMP/Qt.7z"
    qt_extract="$RUNNER_TEMP/QtExtract"

    # unofficial build is used so we can have proprietary codecs with the WebEngine
    curl -L "https://downloads.sourceforge.net/project/fsu0413-qtbuilds/Qt6.7/Windows-x86_64/MSVC/Qt6.7.2-Windows-x86_64-VS2019-16.11.37-20240621.7z" -o "$qt_archive"

    7z x "$qt_archive" "-o$qt_extract" -aoa
    mkdir "$qt_path"
    mv "$qt_extract"/*/* "$qt_path"

    rm -rf "$qt_extract"
    rm -f "$qt_archive"
fi

echo "QT_ROOT_DIR=$qt_path" >> $GITHUB_ENV
echo "QT_PLUGIN_PATH=$qt_path/plugins" >> $GITHUB_ENV
echo "QML2_IMPORT_PATH=$qt_path/qml" >> $GITHUB_ENV
echo "$qt_path/bin" >> $GITHUB_PATH
