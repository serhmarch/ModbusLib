/*!
 * \file   ModbusUdpPort.h
 * \brief  Header file of class `ModbusUdpPort`.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSUDPPORT_H
#define MODBUSUDPPORT_H

#include "ModbusUdpPortBase.h"

/*! \brief Class `ModbusUdpPort` implements UDP version of Modbus protocol.

    \details `ModbusPort` contains function to work with UDP-port (connection).

 */

class MODBUS_EXPORT ModbusUdpPort : public ModbusUdpPortBase
{
public:
    /// \details Constructor of the class.
    ModbusUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::UDP`.
    Modbus::ProtocolType type() const override { return Modbus::UDP; }

public:
    ///  \details Repeat next request parameters (for Modbus UDP transaction Id).
    void setNextRequestRepeated(bool v);

    /// \details Returns `true' if the identifier of each subsequent parcel is automatically incremented by 1, `false' otherwise.
    bool autoIncrement() const;

    /// \details Returns the current transaction identifier.
    uint16_t transactionId() const;
    
    /// \details Sets the transaction identifier for the next request.
    void setTransactionId(uint16_t id);

protected:
    using ModbusUdpPortBase::ModbusUdpPortBase;
};

#endif // MODBUSUDPPORT_H
