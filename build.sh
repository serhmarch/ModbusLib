#!/usr/bin/bash
cmake --preset "Linux-No-Examples-Tests-Release"
cmake --build --preset "Linux-No-Examples-Tests-Release"

cmake --preset "Linux-Qt-No-Examples-Tests-Release"
cmake --build --preset "Linux-Qt-No-Examples-Tests-Release"