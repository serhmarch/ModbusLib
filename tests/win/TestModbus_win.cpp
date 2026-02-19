#include "../TestModbus.h"

#include <win/Modbus_win.h>

ModbusSocket *createTestSocket()
{
    return new ModbusSocket();
}