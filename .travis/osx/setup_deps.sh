#!/bin/bash

brew update || exit 1
brew outdated cmake || brew upgrade cmake || exit 1
brew install ninja || exit 1

if [ -z "$QT_VERSION" ]; then
    brew install qt || exit 1
    export QT_PREFIX=/usr/local/opt/qt
else
    pip3 install aqtinstall || exit 1
    aqt install -O "$HOME/Qt" "$QT_VERSION" mac desktop || exit 1
    export QT_PREFIX="$HOME/Qt/$QT_VERSION/clang_64"
fi
