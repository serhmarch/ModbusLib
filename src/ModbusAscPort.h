/*!
 * \file   ModbusAscPort.h
 * \brief  Contains definition of ASCII serial port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSASCPORT_H
#define MODBUSASCPORT_H

#include "ModbusSerialPort.h"

class MODBUS_EXPORT ModbusAscPort : public ModbusSerialPort
{
public:
    ModbusAscPort(bool blocking = false);
    ~ModbusAscPort();

public:
    Modbus::ProtocolType type() const override { return Modbus::ASC; }

protected:
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) override;
    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

protected:
    using ModbusSerialPort::ModbusSerialPort;
};

#endif // MODBUSASCPORT_H
