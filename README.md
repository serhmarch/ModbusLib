# ModbusLib

## Overview

ModbusLib is a free, open-source Modbus library written in C++. 

It implements client and server functions for TCP, RTU and ASCII versions of Modbus Protocol.

It has interface for plain C language (implements in cModbus.h header file).

Also it has optional wrapper to use with Qt (implements in ModbusQt.h header file).

Library implements such Modbus functions as:
* 1  (0x01) - `READ_COILS`
* 2  (0x02) - `READ_DISCRETE_INPUTS`
* 3  (0x03) - `READ_HOLDING_REGISTERS`
* 4  (0x04) - `READ_INPUT_REGISTERS`
* 5  (0x05) - `WRITE_SINGLE_COIL`
* 6  (0x06) - `WRITE_SINGLE_REGISTER`
* 7  (0x07) - `READ_EXCEPTION_STATUS`
* 15 (0x0F) - `WRITE_MULTIPLE_COILS`
* 16 (0x10) - `WRITE_MULTIPLE_REGISTERS`

## Build using CMake

### Brief

1.  Build Tools

    Previously you need to install c++ compiler kit, git and cmake itself (qt tools if needed).

    Then set PATH env variable to find compliler, cmake, git etc.

    Don't forget to use appropriate version of compiler, linker (x86|x64).

2.  Create project directory, move to it and clone repository:
    ```console
    $ cd ~
    $ mkdir src
    $ cd src
    $ git clone https://github.com/serhmarch/ModbusLib.git
    ```
3.  Create and/or move to directory for build output, e.g. `~/bin/ModbusLib`:
    ```console
    $ cd ~
    $ mkdir -p bin/ModbusLib
    $ cd bin/ModbusLib
    ```
4.  Run cmake to generate project (make) files.
    ```console
    $ cmake -S ~/src/ModbusLib -B .
    ```
    To make Qt-compatibility (switch off by default for cmake build) you can use next command (e.g. for Windows 64):
    ```console
    >cmake -DMB_QT_ENABLED=ON -DCMAKE_PREFIX_PATH:PATH=C:/Qt/5.15.2/msvc2019_64 -S <path\to\src\ModbusLib> -B .
    ```
5.  Make binaries (+ debug|release config):
    ```console
    $ cmake --build .
    $ cmake --build . --config Debug
    $ cmake --build . --config Release
    ```    
    
6.  Resulting bin files is located in `./bin` directory.
