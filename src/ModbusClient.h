/*!
 * \file   ModbusClient.h
 * \brief  Header file of Modbus client.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include "ModbusObject.h"

class ModbusClientPort;

/*! \brief The `ModbusClient` class implements the interface of the client part of the Modbus protocol.

    \details `ModbusClient` contains a list of Modbus functions that are implemented by the Modbus client program.
    It implements functions for reading and writing different types of Modbus memory that are defined by the specification.
    The operations that return `Modbus::StatusCode` are asynchronous, that is, if the operation is not completed, it returns the intermediate status `Modbus::Status_Processing`,
    and then it must be called until it is successfully completed or returns an error status.

 */

class MODBUS_EXPORT ModbusClient : public ModbusObject
{
public:
    /// \details Class constructor.
    /// \param[in] unit The address of the remote Modbus device to which this client is bound.
    /// \param[in] port A pointer to the port object to which this client object belongs.
    ModbusClient(uint8_t unit, ModbusClientPort *port);

public:
    /// \details Returns the type of the Modbus protocol.
    Modbus::ProtocolType type() const;

    /// \details Returns the address of the remote Modbus device to which this client is bound.
    uint8_t unit() const;

    /// \details Sets the address of the remote Modbus device to which this client is bound.
    void setUnit(uint8_t unit);

    /// \details Returns `true` if communication with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Returns a pointer to the port object to which this client object belongs.
    ModbusClientPort *port() const;

public:

#ifndef MBF_READ_COILS_DISABLE
    /// \details Same as `ModbusInterface::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readCoils(uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Same as `ModbusInterface::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readDiscreteInputs(uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    /// \details Same as `ModbusInterface::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readHoldingRegisters(uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    /// \details Same as `ModbusInterface::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readInputRegisters(uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    /// \details Same as `ModbusInterface::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeSingleCoil(uint16_t offset, bool value);
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    /// \details Same as `ModbusInterface::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeSingleRegister(uint16_t offset, uint16_t value);
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    /// \details Same as `ModbusInterface::readExceptionStatus(uint8_t unit, uint8_t *status)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readExceptionStatus(uint8_t *value);
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    /// \details Same as `ModbusClientPort::diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnostics(uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata);
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    /// \details Same as `ModbusClientPort::getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode getCommEventCounter(uint16_t *status, uint16_t *eventCount);
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    /// \details Same as `ModbusClientPort::getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *events)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode getCommEventLog(uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Same as `ModbusInterface::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeMultipleCoils(uint16_t offset, uint16_t count, const void *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Same as `ModbusInterface::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeMultipleRegisters(uint16_t offset, uint16_t count, const uint16_t *values);
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    /// \details Same as `ModbusClientPort::reportServerID(uint8_t unit, uint8_t *count, uint8_t *data)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode reportServerID(uint8_t *count, uint8_t *data);
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode maskWriteRegister(uint16_t offset, uint16_t andMask, uint16_t orMask);
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readWriteMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readWriteMultipleRegisters(uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    /// \details Same as `ModbusClientPort::readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readFIFOQueue(uint16_t fifoadr, uint16_t *count, uint16_t *values);
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
    /// \details Same as `ModbusClientPort::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readCoilsAsBoolArray(uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Same as `ModbusClientPort::readWriteMultipleRegisters(uint8_t unit, uint16_t offset, readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readDiscreteInputsAsBoolArray(uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)`, but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeMultipleCoilsAsBoolArray(uint16_t offset, uint16_t count, const bool *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

public:
    /// \details Returns the status of the last operation performed.
    Modbus::StatusCode lastPortStatus() const;

    /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastPortErrorStatus() const;

    /// \details Returns text repr of the last error of the performed operation.
    const Modbus::Char *lastPortErrorText() const;

protected:
    /// \cond
    using ModbusObject::ModbusObject;
    /// \endcond
};

#endif // MODBUSCLIENT_H
