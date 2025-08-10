#include "ModbusSerialPort_p_unix.h"

ModbusSerialPortPrivate *ModbusSerialPortPrivate::create(uint16_t maxBuffSize, bool blocking)
{
    return new ModbusSerialPortPrivateUnix(maxBuffSize, blocking);
}
