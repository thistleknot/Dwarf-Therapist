#!/bin/bash

brew update
brew outdated cmake || brew upgrade cmake
brew install ninja
if [ -z "$QT_VERSION" ]; then
    brew install qt
else
    brew install python
    pip3 install aqtinstall
    aqt install -O "$TRAVIS_BUILD_DIR/Qt" "$QT_VERSION" mac desktop
fi
