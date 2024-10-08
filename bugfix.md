# 0.2.0

* Fixed bug of LogView flooding by messages when unsuccessful open serial port or bind tcp port.
* Fixed bug when stop polling there was no port close handle.

# 0.3.0

* Fixed 'Blocking'-mode bug for ModbusTcpPort creation
* Fixed bug when write multiple coils/regs always return 0 count in response
* Fixed ModbusClientPort error text is empty when 'Not correct response'
* Improve read/write memory registers/bits functions
* Fixed bug when TCP port give 'Not Correct response' while server close connection

# 0.3.2

* Fixed a bug with an inappropriate Modbus exception code

