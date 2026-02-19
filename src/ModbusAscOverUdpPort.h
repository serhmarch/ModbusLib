/*!
 * \file   ModbusAscOverUdpPort.h
 * \brief  Contains definition of ASC over UDP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSASCOVERUDPPORT_H
#define MODBUSASCOVERUDPPORT_H

#include "ModbusUdpPortBase.h"

/*! \brief Implements ASC over UDP version of the Modbus communication protocol.

    \details `ModbusUdpPortBase` derived from `ModbusSerialPort` and implements `writeBuffer` and `readBuffer`
    for ASC version of Modbus communication protocol.

 */
class MODBUS_EXPORT ModbusAscOverUdpPort : public ModbusUdpPortBase
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusAscOverUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusAscOverUdpPort` returns `Modbus::ASCvUDP`.
    Modbus::ProtocolType type() const override { return Modbus::ASCvUDP; }

protected:
    using ModbusUdpPortBase::ModbusUdpPortBase;
};

#endif // MODBUSASCOVERUDPPORT_H
