#include "../TestModbus.h"

#include <unix/Modbus_unix.h>

ModbusSocket *createTestSocket()
{
    return new ModbusSocket();
}