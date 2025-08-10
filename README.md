# ModbusLib

## Overview

ModbusLib is a free, open-source Modbus library written in C++.
It implements client and server functions for TCP, RTU and ASCII versions of Modbus Protocol.
It has interface for plain C language (implements in cModbus.h header file).
Also it has optional wrapper to use with Qt (implements in ModbusQt.h header file).

Library implements such Modbus functions as:
* `1`  (`0x01`) - `READ_COILS`
* `2`  (`0x02`) - `READ_DISCRETE_INPUTS`
* `3`  (`0x03`) - `READ_HOLDING_REGISTERS`
* `4`  (`0x04`) - `READ_INPUT_REGISTERS`
* `5`  (`0x05`) - `WRITE_SINGLE_COIL`
* `6`  (`0x06`) - `WRITE_SINGLE_REGISTER`
* `7`  (`0x07`) - `READ_EXCEPTION_STATUS`
* `8`  (`0x08`) - `DIAGNOSTICS` (since v0.4)
* `11` (`0x0B`) - `GET_COMM_EVENT_COUNTER` (since v0.4)
* `12` (`0x0C`) - `GET_COMM_EVENT_LOG` (since v0.4)
* `15` (`0x0F`) - `WRITE_MULTIPLE_COILS`
* `16` (`0x10`) - `WRITE_MULTIPLE_REGISTERS`
* `17` (`0x11`) - `REPORT_SERVER_ID`(since v0.4)
* `22` (`0x16`) - `MASK_WRITE_REGISTER` (since v0.3)
* `23` (`0x17`) - `READ_WRITE_MULTIPLE_REGISTERS` (since v0.3)
* `24` (`0x18`) - `READ_FIFO_QUEUE` (since v0.4)

## Using Library

### Common usage (C++)

Library was written in C++ and it is the main language to use it (also C support included). 
To start using this library you must include `ModbusClientPort.h` (`ModbusClient.h`) or 
`ModbusServerPort.h` header files (of course after add include path to the compiler).
This header directly or indirectly include `Modbus.h` main header file.
`Modbus.h` header file contains declarations of main data types, functions and class interfaces
to work with the library.

### Client

`ModbusClientPort` implements Modbus interface directly and can be used very simple:
```cpp
#include <ModbusClientPort.h>
//...
void main()
{
    Modbus::NetworkSettings settings;
    settings.host = "someadr.plc";
    settings.port = Modbus::STANDARD_TCP_PORT;
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
#include <ModbusClient.h>
//...
void main()
{
    //...
    ModbusClientPort *port = Modbus::createClientPort(Modbus::TCP, &settings, true);
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

### Server

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
            memcpy(values, &mem4x[offset], count*sizeof(uint16_t));
            return Modbus::Status_Good;
        }
        return Modbus::Status_BadIllegalDataAddress;
    }
};

void main()
{
    MyModbusDevice device;
    Modbus::NetworkSettings settings;
    settings.port = Modbus::STANDARD_TCP_PORT;
    settings.timeout = 3000;
    settings.maxconn = 10;
    ModbusServerPort *port = Modbus::createServerPort(&device, Modbus::TCP, &settings, false);
    int c = 0;
    while (1)
    {
        port->process();
        Modbus::msleep(1);
        if (c % 1000 == 0)
            setValue(0, getValue(0)+1);
        ++c;
    }
}
//...
```

In this example `MyModbusDevice` ModbusInterface class was created.
It implements only single function: `readHoldingRegisters` (`0x03`).
All other functions will return `Modbus::Status_BadIllegalFunction` by default.

This example creates Modbus TCP server that process connections and increment 
first 4x register by 1 every second. This example uses non blocking mode.

### Signal/slot mechanism

Library has simplified Qt-like signal/slot mechanism that can use callbacks when some signal is occured.
User can connect function(s) or class method(s) to the predefined signal. 
Callbacks will be called in the order in which they were connected.

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
```c
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
    NetworkSettings settings;
    settings.host = "someadr.plc";
    settings.port = STANDARD_TCP_PORT;
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

## Build and install using CMake

1.  Build Tools

    Previously you need to install c++ compiler kit, git and cmake itself (qt tools if needed).
    Then set PATH env variable to find compliler, cmake, git etc.

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

7. **Install the Project**

To install ModbusLib, use CMake's install target after building:

```console
$ cmake --install .
```

By default, this installs the library and headers to the build directory. You can change the installation directory by setting the `CMAKE_INSTALL_PREFIX` variable:

```console
$ cmake -DCMAKE_INSTALL_PREFIX=/your/custom/path -P ...
```

After installation, you can use `find_package` in your own CMake project to locate and link against modbus:

```cmake
find_package(modbus REQUIRED)
```

