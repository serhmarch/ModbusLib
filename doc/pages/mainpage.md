\mainpage ModbusLib
\tableofcontents

## Overview

ModbusLib is a free, open-source Modbus library written in C++. 
It implements client and server functions for TCP, RTU and ASCII versions of Modbus Protocol.
It has interface for C language (implements in cModbus.h header file).
Also it has optional wrapper to use with Qt (implements in ModbusQt.h header file).
Library can work in both blocking and non-blocking mode.

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
* 23 (0x17) - `WRITE_MULTIPLE_REGISTERS`

## Using Library

### Common usage (C++)

Library was written in C++ and it is the main language to use it. 
To start using this library you must include `ModbusClientPort.h` (`ModbusClient.h`) or 
`ModbusServerPort.h` header files (of course after add include path to the compiler).
This header directly or indirectly include `Modbus.h` main header file.
`Modbus.h` header file contains declarations of main data types, functions and class interfaces
to work with the library.

It contains definition of `Modbus::StatusCode` enumeration that defines result of 
library operations, `ModbusInterface` class interface that contains list of functions which
the library implements, `Modbus::createClientPort` and `Modbus::createServerPort` functions, 
that creates corresponding `ModbusClientPort` and `ModbusServerPort` main working classes.
Those classes that implements Modbus functions for the library for client and server 
version of protocol, respectively.

#### Client
`ModbusClientPort` implements Modbus interface directly and can be used very simply:
```cpp
#include <ModbusClientPort.h>
//...
void main()
{
    Modbus::TcpSettings settings;
    settings.host = "someadr.plc";
    settings.port = 502;
    settings.timeout = 3000;
    ModbusClientPort *port = Modbus::createClientPort(Modbus::TCP, &settings, true);
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 10;
    uint16_t values[count];
    Modbus::StatusCode status = port->readHoldingRegisters(unit, offset, count, values);
    if (Modbus::StatusIsGood(status))
    {
        // process out array `values` ...
    }
    else
        std::cout << "Error: " << port->lastErrorText() << '\n';
    delete port;
}
//...
```

User don't need to create any connection or open any port, library makes it automatically.

User can use `ModbusClient` class to simplify Modbus function's interface (don't need to use
`unit` parameter):
```cpp
#include <ModbusClientPort.h>
//...
void main()
{
    //...
    ModbusClient c1(1, port);
    ModbusClient c2(2, port);
    ModbusClient c3(3, port);
    Modbus::StatusCode s1, s2, s3;
    while(1)
    {
        s1 = c1.readHoldingRegisters(0, 10, values);
        s2 = c2.readHoldingRegisters(0, 10, values);
        s3 = c3.readHoldingRegisters(0, 10, values);
        Modbus::msleep(1);
    }
    //...
}
//...
```
In this example 3 clients with unit address 1, 2, 3 are used. 
User don't need to manage its common resource `port`. Library make it automatically.
First `c1` client owns `port`, than when finished resource transferred to `c2` and so on.

#### Server
Unlike client the server do not implement `ModbusInterface` directly. 
It accepts pointer to `ModbusInterface` in its constructor as parameter and transfer all requests
to this interface. So user can define by itself how incoming Modbus-request will be processed:
```cpp
#include <ModbusServerPort.h>
//...
class MyModbusDevice : public ModbusInterface
{
#define MEM_SIZE 16
    uint16_t mem4x[MEM_SIZE];
public:
    MyModbusDevice() { memset(mem4x, 0, sizeof(mem4x)); }
    uint16_t getValue(uint16_t offset) { return mem4x[offset]; }
    void setValue(uint16_t offset, uint16_t value) { mem4x[offset] = value; }
    Modbus::StatusCode readHoldingRegisters(uint8_t  unit, 
                                            uint16_t offset, 
                                            uint16_t count, 
                                            uint16_t *values) override
    {
        if (unit != 1)
            return Modbus::Status_BadGatewayPathUnavailable;
        if ((offset + count) <= MEM_SIZE)
        {
            memcpy(values, mem4x, count*sizeof(uint16_t));
            return Modbus::Status_Good;
        }
        return Modbus::Status_BadIllegalDataAddress;
    }
};

void main()
{
    MyModbusDevice device;
    Modbus::TcpSettings settings;
    settings.port = 502;
    settings.timeout = 3000;
    ModbusServerPort *port = Modbus::createServerPort(&device, Modbus::TCP, &settings, false);
    int c = 0;
    while (1)
    {
        port->process();
        Modbus::msleep(1);
        if (c % 1000 == 0) setValue(0, getValue(0)+1);
    }
}
//...
```

In this example `MyModbusDevice` ModbusInterface class was created.
It imlements only single function: readHoldingRegisters (0x03).
All other functions will return `Modbus::Status_BadIllegalFunction` by default.

This example creates Modbus TCP server that process connections and increment 
first 4x register by 1 every second. This example uses non blocking mode.

#### Non blocking mode

In non blocking mode Modbus function exits immediately even if remote connection 
processing is not finished. In this case function returns `Modbus::Status_Processing`.
This is 'Arduino'-style of programing, when function must not be blocked and 
return intermediate value that indicates that function is not finished. 
Then external code call this function again and again until Good or Bad status 
will not be returned. 

Example of non blocking client:
```cpp
#include <ModbusClientPort.h>
//...
void main()
{
    //...
    ModbusClientPort *port = Modbus::createClientPort(Modbus::TCP, &settings, false);
    //...
    while(1)
    {
        s1 = c1.readHoldingRegisters(0, 10, values);
        s2 = c2.readHoldingRegisters(0, 10, values);
        s3 = c3.readHoldingRegisters(0, 10, values);
        doSomeOtherStuffInCurrentThread();
        Modbus::msleep(1);
    }
    //...
}
//...
```

So if user needs to check is function finished he can write:
```cpp
        //...
        s1 = c1.readHoldingRegisters(0, 10, values);
        if (!Modbus::StatusIsProcessing(s1)) {
            // ...
        }
        //...
```

#### Signal/slot mechanism

Library has simplified Qt-like signal/slot mechanism that can use callbacks when some signal is occured.
User can connect function(s) or class method(s) to the predefined signal. 
Callbacks will be called in order which it were connected.

For example `ModbusClientPort` signal/slot mechanism:
```cpp
#include <ModbusClientPort.h>

class Printable 
{
public:
    void printTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
    {
        std::cout << source << " Tx: " << Modbus::bytesToString(buff, size) << '\n';
    }
};

void printRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Rx: " << Modbus::bytesToString(buff, size) << '\n';
}

void main()
{
    //...
    ModbusClientPort *port = Modbus::createClientPort(Modbus::TCP, &settings, false);
    Printable print;
    port->connect(&ModbusClientPort::signalTx, &print, &Printable::printTx);
    port->connect(&ModbusClientPort::signalRx, printRx);
    //...
}
```

### Using with C

To use the library with pure C language user needs to include only one header: `cModbus.h`.
This header includes functions that wraps Modbus interface classes and its methods.
```cpp
#include <cModbus.h>
//...
void printTx(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("%s Tx: %s\n", source, sbytes(buff, size, s, sizeof(s)));
}

void printRx(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("%s Rx: %s\n", source, sbytes(buff, size, s, sizeof(s)));
}

void main()
{
    TcpSettings settings;
    settings.host = "someadr.plc";
    settings.port = 502;
    settings.timeout = 3000;
    const uint8_t unit = 1;
    cModbusClient client = cCliCreate(unit, TCP, &settings, true);
    cModbusClientPort cpo = cCliGetPort(client);
    StatusCode s;
    cCpoConnectTx(cpo, printTx);
    cCpoConnectRx(cpo, printRx);
    while(1)
    {
        s = cReadHoldingRegisters(client, 0, 10, values);
        //...
        msleep(1);
    }
}
//...
```

### Using with Qt

When including `ModbusQt.h` user can use ModbusLib in convinient way in Qt framework.
It has wrapper functions for Qt library to use it together with Qt core objects:
```cpp
#include <ModbusQt.h>
```

## Examples

Examples is located in `examples` folder or root directory.

### `democlient`

`democlient` example demonstrate all implemented functions for client one by one begining
from function with lowest number and then increasing this number with predefined period and
other parameters. 
To see list of available parameters you can print next commands:
```console
$ ./democlient -?
$ ./democlient -help
``` 

### `mbclient`

`mbclient` is a simple example that can work like command-line Modbus Client Tester.
It can use only single function at a time but user can change parameters of every supported function.
To see list of available parameters you can print next commands:
```console
$ ./mbclient -?
$ ./mbclient -help
``` 
Usage example:
```console
$ ./mbclient -func 3 -offset 0 -count 10 -period 500 -n inf
``` 

### `demoserver`

`demoserver` example demonstrate all implemented functions for server.
It uses single block for every type of Modbus memory (0x, 1x, 3x and 4x)
and emulates value change for the first 16 bit register by inceremting it 
by 1 every 1000 milliseconds. So user can run Modbus Client to check
first 16 bit of 000001 (100001) or first register 400001 (300001) changing
every 1 second.
To see list of available parameters you can print next commands:
```console
$ ./demoserver -?
$ ./demoserver -help
``` 

### `mbserver`

`mbserver` is a simple example that can work like command-line Modbus Server Tester.
It implements all function of Modbus library. So remote client can work with server
reading and writting values to it.
To see list of available parameters you can print next commands:
```console
$ ./mbserver -?
$ ./mbserver -help
``` 
Usage example:
```console
$ ./mbserver -c0 256 -c1 256 -c3 16 -c4 16 -type RTU -serial /dev/ttyS0
``` 

## Tests

Unit Tests using googletest library.
Googletest source library must be located in external/googletest

## Documenations

Documentation is located in `docs` directory. Documentation is automatically generated by doxygen.

## Building

### Build using CMake

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
```
>cmake -DMB_QT_ENABLED=ON -DCMAKE_PREFIX_PATH:PATH=C:/Qt/5.15.2/msvc2019_64 -S <path\to\src\ModbusLib> -B .
```

5.  Make binaries (+ debug|release config):
```console
$ cmake --build .
$ cmake --build . --config Debug
$ cmake --build . --config Release
```    
    
6.  Resulting bin files is located in `./bin` directory.

### Build using qmake

1.  Update package list:
```console
$ sudo apt-get update
```

2.  Install main build tools like g++, make etc:
```console
$ sudo apt-get install build-essential
```

3.  Install Qt tools:
```console
$ sudo apt-get install qtbase5-dev qttools5-dev
```

4.  Check for correct instalation:
```console
$ whereis qmake
qmake: /usr/bin/qmake
$ whereis libQt5Core*
libQt5Core.prl: /usr/lib/x86_64-linux-gnu/libQt5Core.prl
libQt5Core.so: /usr/lib/x86_64-linux-gnu/libQt5Core.so
libQt5Core.so.5: /usr/lib/x86_64-linux-gnu/libQt5Core.so.5
libQt5Core.so.5.15: /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.15
libQt5Core.so.5.15.3: /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.15.3
$ whereis libQt5Help*
libQt5Help.prl: /usr/lib/x86_64-linux-gnu/libQt5Help.prl
libQt5Help.so: /usr/lib/x86_64-linux-gnu/libQt5Help.so
libQt5Help.so.5: /usr/lib/x86_64-linux-gnu/libQt5Help.so.5
libQt5Help.so.5.15: /usr/lib/x86_64-linux-gnu/libQt5Help.so.5.15
libQt5Help.so.5.15.3: /usr/lib/x86_64-linux-gnu/libQt5Help.so.5.15.3
```

5.  Install git:
```console
$ sudo apt-get install git
```

6.  Create project directory, move to it and clone repository:
```console
$ cd ~
$ mkdir src
$ cd src
$ git clone https://github.com/serhmarch/ModbusLib.git
```

7.  Create and/or move to directory for build output, e.g. `~/bin/ModbusLib`:
```console
$ cd ~
$ mkdir -p bin/ModbusLib
$ cd bin/ModbusLib
```

8.  Run qmake to create Makefile for build:
```console
$ qmake ~/src/ModbusLib/src/ModbusLib.pro -spec linux-g++
```

9.  To ensure Makefile was created print:
```console
$ ls -l
total 36
-rw-r--r-- 1 march march 35001 May  6 18:41 Makefile
```

10. Finaly to make current set of programs print:
```console
$ make
```

11. After build step move to `<build_folder>/bin` to ensure everything is correct:
```console
$ cd bin
$ pwd
~/bin/ModbusLib/bin
```
