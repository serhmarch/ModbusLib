/*!
 * \file   ModbusRtuOverUdpPort.h
 * \brief  Contains definition of RTU over UDP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSRTUOVERUDPPORT_H
#define MODBUSRTUOVERUDPPORT_H

#include "ModbusUdpPortBase.h"

/*! \brief Implements RTU over UDP version of the Modbus communication protocol.

    \details `ModbusUdpPortBase` derived from `ModbusSerialPort` and implements `writeBuffer` and `readBuffer`
    for RTU version of Modbus communication protocol.

 */
class MODBUS_EXPORT ModbusRtuOverUdpPort : public ModbusUdpPortBase
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusRtuOverUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusRtuOverUdpPort` returns `Modbus::RTUvUDP`.
    Modbus::ProtocolType type() const override { return Modbus::RTUvUDP; }

protected:
    using ModbusUdpPortBase::ModbusUdpPortBase;
};

#endif // MODBUSRTUOVERUDPPORT_H
