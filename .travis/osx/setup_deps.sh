#!/bin/bash
set -ev

brew update
brew outdated cmake || brew upgrade cmake
brew install ninja
if [ -z "$QT_VERSION" ]; then
    brew install qt
    export QT_PREFIX=/usr/local/opt/qt
else
    pip3 install aqtinstall
    aqt install -O "$HOME/Qt" "$QT_VERSION" mac desktop
    export QT_PREFIX="$HOME/Qt/$QT_VERSION/clang_64"
fi
