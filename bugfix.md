# 0.2.0

* Fix bug of LogView flooding by messages when unsuccessful open serial port or bind tcp port.
* Fix bug when stop polling there was no port close handle.

# 0.3.0

* Fix 'Blocking'-mode bug for ModbusTcpPort creation
* Fix bug when write multiple coils/regs always return 0 count in response
* Fix ModbusClientPort error text is empty when 'Not correct response'
* Improve read/write memory registers/bits functions
* Fix bug when TCP port give 'Not Correct response' while server close connection
