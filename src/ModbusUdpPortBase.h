/*!
 * \file   ModbusUdpPortBase.h
 * \brief  Header file of class `ModbusUdpPortBase`.
 *
 * \author serhmarch
 * \date   Feb 2026
 */
#ifndef MODBUSUDPPORTBASE_H
#define MODBUSUDPPORTBASE_H

#include "ModbusNetPort.h"

class ModbusSocket;

/*! \brief Class `ModbusUdpPortBase` implements UDP transport of Modbus protocol.

    \details 
 */

class MODBUS_EXPORT ModbusUdpPortBase : public ModbusNetPort
{
public:
    Modbus::Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

protected:
    using ModbusNetPort::ModbusNetPort;
};

#endif // MODBUSUDPPORTBASE_H
