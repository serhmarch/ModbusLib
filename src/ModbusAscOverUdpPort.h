/*!
 * \file   ModbusAscOverUdpPort.h
 * \brief  Contains definition of ASC over UDP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSASCOVERUDPPORT_H
#define MODBUSASCOVERUDPPORT_H

#include "ModbusAscPort.h"

/*! \brief Implements ASC over UDP version of the Modbus communication protocol.

    \details `ModbusAscPort` derived from `ModbusSerialPort` and implements `writeBuffer` and `readBuffer`
    for ASC version of Modbus communication protocol.

 */
class MODBUS_EXPORT ModbusAscOverUdpPort : public ModbusAscPort
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusAscOverUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusAscOverUdpPort` returns `Modbus::ASCvUDP`.
    Modbus::ProtocolType type() const override { return Modbus::ASCvUDP; }

protected:
    using ModbusAscPort::ModbusAscPort;
};

#endif // MODBUSASCOVERUDPPORT_H
