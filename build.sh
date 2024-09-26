#!/usr/bin/bash
cmake --preset "Linux-All-Debug"
cmake --build --preset "Linux-All-Debug"

cmake --preset "Linux-All-Release"
cmake --build --preset "Linux-All-Release"

cmake --preset "Linux-Qt-No-Examples-Tests-Debug"
cmake --build --preset "Linux-Qt-No-Examples-Tests-Debug"

cmake --preset "Linux-Qt-No-Examples-Tests-Release"
cmake --build --preset "Linux-Qt-No-Examples-Tests-Release"