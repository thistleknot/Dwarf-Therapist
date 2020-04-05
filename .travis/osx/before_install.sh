#!/bin/bash

brew update
brew install qt
brew outdated cmake || brew upgrade cmake
brew install ninja
