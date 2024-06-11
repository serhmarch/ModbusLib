/*!
 * \file   ModbusRtuPort.h
 * \brief  Contains definition of RTU serial port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSRTUPORT_H
#define MODBUSRTUPORT_H

#include "ModbusSerialPort.h"

class MODBUS_EXPORT ModbusRtuPort : public ModbusSerialPort
{
public:
    ModbusRtuPort(bool blocking = false);
    ~ModbusRtuPort();

public:
    Modbus::ProtocolType type() const override { return Modbus::RTU; }

protected:
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) override;
    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

protected:
    using ModbusSerialPort::ModbusSerialPort;
};

#endif // MODBUSRTUPORT_H
