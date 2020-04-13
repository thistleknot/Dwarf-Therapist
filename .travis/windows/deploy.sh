#!/bin/bash

ARCH=${QT_ARCH%%_*}

dest=DwarfTherapist-${TRAVIS_TAG}-${ARCH}
mkdir $dest

cp "$TRAVIS_BUILD_DIR/build/Release/DwarfTherapist.exe" "$dest/"
"$QT_PREFIX/bin/windeployqt.exe" "$dest/DwarfTherapist.exe"

cp "$TRAVIS_BUILD_DIR/README.rst" "$dest/"
cp "$TRAVIS_BUILD_DIR/LICENSE.txt" "$dest/"
cp "$TRAVIS_BUILD_DIR/CHANGELOG.txt" "$dest/"
cp "$TRAVIS_BUILD_DIR/build/lnp/manifest.json" "$dest/"

cp -R "$TRAVIS_BUILD_DIR/share" "$dest/data"

mkdir "$dest/doc"
cp "Dwarf Therapist.pdf" "$dest/doc/"

# Compile Qt style plugins
case $ARCH in
win32)
    VSARCH=x86
    ;;
win64)
    VSARCH=amd64
    ;;
esac
mkdir "$dest/styles"
for plugin in fusiondark DarkStyle; do
    git clone https://github.com/cvuchener/$plugin
    cd $plugin
    cmd //C $TRAVIS_BUILD_DIR/.travis/windows/build_plugin.bat $VSARCH $QT_PREFIX
    cd -
    cp "$QT_PREFIX/plugins/styles/$plugin.dll" "$dest/styles/"
done

# TODO openssl dlls

7z a "$dest.zip" "$dest/"
