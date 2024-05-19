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

class MODBUS_EXPORT ModbusClient : public ModbusObject, public ModbusInterface
{
public:
    /// \details Class constructor.
    /// \param[in] unit The address of the remote Modbus device to which this client is bound.
    /// \param[in] port A pointer to the port object to which this client object belongs.
    ModbusClient(uint8_t unit, ModbusClientPort *port);

    /// \details Class destructor.
    ~ModbusClient();

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
    /// \details Same as `ModbusClient::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline Modbus::StatusCode readCoils(uint16_t offset, uint16_t count, void *values) { return readCoils(unit(), offset, count, values); }

    /// \details The same as `ModbusClient::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline Modbus::StatusCode readDiscreteInputs(uint16_t offset, uint16_t count, void *values) { return readDiscreteInputs(unit(), offset, count, values); }

    /// \details The same as `ModbusClient::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode readHoldingRegisters(uint16_t offset, uint16_t count, uint16_t *values) { return readHoldingRegisters(unit(), offset, count, values); }

    /// \details Same as `ModbusClient::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode readInputRegisters(uint16_t offset, uint16_t count, uint16_t *values) { return readInputRegisters(unit(), offset, count, values); }

    /// \details The same as `ModbusClient::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode writeSingleCoil(uint16_t offset, bool value) { return writeSingleCoil(unit(), offset, value); }

    /// \details The same as `ModbusClient::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode writeSingleRegister(uint16_t offset, uint16_t value) { return writeSingleRegister(unit(), offset, value); }

    /// \details The same as `ModbusClient::readExceptionStatus(uint8_t unit, uint8_t *status)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode readExceptionStatus(uint8_t *value) { return readExceptionStatus(unit(), value); }

    /// \details Same as `ModbusClient::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline Modbus::StatusCode writeMultipleCoils(uint16_t offset, uint16_t count, const void *values) { return writeMultipleCoils(unit(), offset, count, values); }

    /// \details Same as `ModbusClient::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode writeMultipleRegisters(uint16_t offset, uint16_t count, const uint16_t *values) { return writeMultipleRegisters(unit(), offset, count, values); }

    /// \details The same as `ModbusClient::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode readCoilsAsBoolArray(uint16_t offset, uint16_t count, bool *values) { return readCoilsAsBoolArray(unit(), offset, count, values); }

    /// \details The same as `ModbusClient::readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline Modbus::StatusCode readDiscreteInputsAsBoolArray(uint16_t offset, uint16_t count, bool *values) { return readDiscreteInputsAsBoolArray(unit(), offset, count, values); }

    /// \details The same as `ModbusClient::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline Modbus::StatusCode writeMultipleCoilsAsBoolArray(uint16_t offset, uint16_t count, const bool *values) { return writeMultipleCoilsAsBoolArray(unit(), offset, count, values); }

public: // Modbus Interface
    Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
    Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
    Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
    Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value) override;
    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value) override;
    Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *value) override;
    Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values) override;
    Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values) override;

public:
    /// \details The same as `ModbusClient::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    Modbus::StatusCode readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values);

    /// \details The same as `ModbusClient::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    Modbus::StatusCode readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values);

    /// \details The same as `ModbusClient::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the input buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    Modbus::StatusCode writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values);

public:
    /// \details Returns the status of the last operation performed.
    Modbus::StatusCode lastStatus() const;

    /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastErrorStatus() const;

    /// \details Returns text repr of the last error of the performed operation.
    const Modbus::Char *lastErrorText() const;

protected:
    Modbus::StatusCode request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff);

protected:
    using ModbusObject::ModbusObject;
};

#endif // MODBUSCLIENT_H
