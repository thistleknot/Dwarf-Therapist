#!/bin/bash

choco install python3 || exit 1
# Refresh PATH (https://travis-ci.community/t/windows-builds-refreshenv-command-not-found/5803)
eval $(powershell -NonInteractive -Command 'write("export PATH=`"" + ([Environment]::GetEnvironmentVariable("PATH","Machine") + ";" + [Environment]::GetEnvironmentVariable("PATH","User")).replace("\","/").replace("C:","/c").replace(";",":") + ":`$PATH`"")') || exit 1
pip install aqtinstall || exit 1
aqt install -O "$HOME/Qt" "$QT_VERSION" windows desktop "$QT_ARCH" || exit 1
export QT_PREFIX="$HOME/Qt/$QT_VERSION/${QT_ARCH#*_}"
