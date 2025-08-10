#include "ModbusSerialPort_p_win.h"

ModbusSerialPortPrivate *ModbusSerialPortPrivate::create(uint16_t maxBuffSize, bool blocking)
{
    return new ModbusSerialPortPrivateWin(maxBuffSize, blocking);
}
