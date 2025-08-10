/*!
 * \file   ModbusRtuPort.h
 * \brief  Contains definition of RTU serial port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSRTUPORT_H
#define MODBUSRTUPORT_H

#include "ModbusPort.h"

/*! \brief Implements RTU version of the Modbus communication protocol.

    \details `ModbusRtuPort` derived from `ModbusSerialPort` and implements `writeBuffer` and `readBuffer`
    for RTU version of Modbus communication protocol.

 */
class MODBUS_EXPORT ModbusRtuPort : public ModbusPort
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusRtuPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusRtuPort` returns `Modbus::RTU`.
    Modbus::ProtocolType type() const override { return Modbus::RTU; }
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) override;
    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

protected:
    using ModbusPort::ModbusPort;
};

#endif // MODBUSRTUPORT_H
