/*!
 * \file   ModbusTcpPortBase.h
 * \brief  Header file of class `ModbusTcpPortBase`.
 *
 * \author serhmarch
 * \date   Feb 2026
 */
#ifndef MODBUSTCPPORTBASE_H
#define MODBUSTCPPORTBASE_H

#include "ModbusNetPort.h"

class ModbusSocket;

/*! \brief Class `ModbusTcpPortBase` implements TCP transport of Modbus protocol.

    \details 
 */

class MODBUS_EXPORT ModbusTcpPortBase : public ModbusNetPort
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

#endif // MODBUSTCPPORTBASE_H
