/*!
 * \file   ModbusUdpPort.h
 * \brief  Header file of class `ModbusUdpPort`.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSUDPPORT_H
#define MODBUSUDPPORT_H

#include "ModbusTcpPort.h"

/*! \brief Class `ModbusUdpPort` implements UDP version of Modbus protocol.

    \details `ModbusPort` contains function to work with UDP-port (connection).

 */

class MODBUS_EXPORT ModbusUdpPort : public ModbusTcpPort
{
public:
    /// \details Constructor of the class.
    ModbusUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::UDP`.
    Modbus::ProtocolType type() const override { return Modbus::UDP; }

protected:
    using ModbusTcpPort::ModbusTcpPort;
};

#endif // MODBUSUDPPORT_H
