> Description of all fixed bug is located in `bugfix.md`

# 0.2.0

* Fixed some bugs
* Add some functions to extend Qt support

# 0.3.0

* Add functions 'toBaudRate'
* Add 'MASK_WRITE_REGISTER'-function
* Add 'READ_WRITE_MULTIPLEREGISTER'-function
* Add 'tries'-setting for ModbusQt
* Fixed bugs

# 0.3.1

* Improve string repr for Parity, StopBits and FlowControl serial port settings

# 0.3.2

* Fixed a bug (see bugfix.md)

# 0.3.3

* Fixed bugs (see bugfix.md)

# 0.3.4

* Minor changes, fixed bug (see bugfix.md)

# 0.3.5

* Extended information messages about error

# 0.3.6

* Fixed crash bug on Linux when working with serial port (see bugfix.md)

# 0.3.7

* Extended `Modbus::availableSerialPorts()` function on Linux

# 0.3.8

* Added 'Timestamp' data and functions to work with
* Fixed a bug (see bugfix.md)

# 0.3.10

* Added 'ModbusClientPort::setPort' method

# 0.4.0

* Added functions:
    * 8  (0x08) - DIAGNOSTICS
    * 11 (0x0B) - GET_COMM_EVENT_COUNTER
    * 12 (0x0C) - GET_COMM_EVENT_LOG
    * 17 (0x11) - REPORT_SERVER_ID
    * 24 (0x18) - READ_FIFO_QUEUE
* Added options for client/server/c-support disable
* Added options for Modbus function disabling from lib
* Added option for dynamic/static lib
* Added some string convertion functions
* Added 'signalOpened' generation for ModbusServerResource class

# 0.4.1

* Added support for broadcast
* Updated docs

# 0.4.2

* Fixed 'Parity'-setting swap for Win
* Added `unitmap` byte array for ModbusServerPort to prefilter unit addresses

# 0.4.3

* Fixed TCP Server delay while connecting
* Added max connections setting for TCP server
* Added Modbus::Address class
* Added setConsoleColor()-function
* Added string converting functions for serial port settings
* Updated docs

# 0.4.4

* Fixed `ModbusTcpServer::setMaxConnection`
* Fixed demo
* Added `ModbusClientPort::lastTries` statistic
* If serial port `timeoutInterByte = 0` that means no need to wait next bytes
* Fixed issue with `writeCount` registers for `ReadWriteMultipleRegisters` Modbus func
* Fixed issue with blocking TCP sockets

# 0.4.5

* Minor internal improvements

# 0.4.6

* Fixed memory leak according to the #6 bug report
* Change types of in/out buffers of `diagnostics` (0x08) function to `void*`
