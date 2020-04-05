#!/bin/bash

choco install python3
# Refresh PATH (https://travis-ci.community/t/windows-builds-refreshenv-command-not-found/5803)
eval $(powershell -NonInteractive -Command 'write("export PATH=`"" + ([Environment]::GetEnvironmentVariable("PATH","Machine") + ";" + [Environment]::GetEnvironmentVariable("PATH","User")).replace("\","/").replace("C:","/c").replace(";",":") + ":`$PATH`"")')
pip install aqtinstall
aqt install -O "$TRAVIS_BUILD_DIR/Qt" "$QT_VERSION" windows desktop "$QT_ARCH"
