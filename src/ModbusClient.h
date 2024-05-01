/*!
 * \file   ModbusClient.h
 * \brief  Header file of Modbus client.
 * 
 * \author march
 * \date   April 2024
 */
#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <string>

#include "ModbusClientPort.h"

namespace Modbus {

/*! \brief The `Modbus::Client` class implements the interface of the client part of the Modbus protocol.

    \details `Modbus::Client` contains a list of Modbus functions that are implemented by the Modbus client program.
    It implements functions for reading and writing different types of Modbus memory that are defined by the specification.
    The operations that return `Modbus::StatusCode` are asynchronous, that is, if the operation is not completed, it returns the intermediate status `Modbus::Status_Processing`,
    and then it must be called until it is successfully completed or returns an error status.

 */
class MODBUS_EXPORT Client : public Interface
{
public:
    /// \details Class constructor.
    /// \param[in] unit The address of the remote Modbus device to which this client is bound.
    /// \param[in] port A pointer to the port object to which this client object belongs.
    Client(uint8_t unit, ClientPort *port);

    /// \details Class destructor.
    ~Client();

public:
    /// \details Returns the type of the Modbus protocol.
    Type type() const;

    /// \details Returns the address of the remote Modbus device to which this client is bound.
    inline uint8_t unit() const { return m_unit; }

    /// \details Sets the address of the remote Modbus device to which this client is bound.
    inline void setUnit(uint8_t unit) { m_unit = unit; }

    /// \details Returns `true` if communication with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Returns a pointer to the port object to which this client object belongs.
    inline ClientPort *port() const { return m_port; }

public:
    /// \details Same as `Modbus::Client::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline StatusCode readCoils(uint16_t offset, uint16_t count, void *values) { return readCoils(m_unit, offset, count, values); }

    /// \details The same as `Modbus::Client::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline StatusCode readDiscreteInputs(uint16_t offset, uint16_t count, void *values) { return readDiscreteInputs(m_unit, offset, count, values); }

    /// \details The same as `Modbus::Client::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode readHoldingRegisters(uint16_t offset, uint16_t count, uint16_t *values) { return readHoldingRegisters(m_unit, offset, count, values); }

    /// \details Same as `Modbus::Client::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode readInputRegisters(uint16_t offset, uint16_t count, uint16_t *values) { return readInputRegisters(m_unit, offset, count, values); }

    /// \details The same as `Modbus::Client::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode writeSingleCoil(uint16_t offset, bool value) { return writeSingleCoil(m_unit, offset, value); }

    /// \details The same as `Modbus::Client::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode writeSingleRegister(uint16_t offset, uint16_t value) { return writeSingleRegister(m_unit, offset, value); }

    /// \details The same as `Modbus::Client::readExceptionStatus(uint8_t unit, uint8_t *status)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode readExceptionStatus(uint8_t *value) { return readExceptionStatus(m_unit, value); }

    /// \details Same as `Modbus::Client::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline StatusCode writeMultipleCoils(uint16_t offset, uint16_t count, const void *values) { return writeMultipleCoils(m_unit, offset, count, values); }

    /// \details Same as `Modbus::Client::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode writeMultipleRegisters(uint16_t offset, uint16_t count, const uint16_t *values) { return writeMultipleRegisters(m_unit, offset, count, values); }

    /// \details The same as `Modbus::Client::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode readCoilsAsBoolArray(uint16_t offset, uint16_t count, bool *values) { return readCoilsAsBoolArray(m_unit, offset, count, values); }

    /// \details The same as `Modbus::Client::readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`, but the address of the remote Modbus device is missing. It is pre-set in the constructor.
    inline StatusCode readDiscreteInputsAsBoolArray(uint16_t offset, uint16_t count, bool *values) { return readDiscreteInputsAsBoolArray(m_unit, offset, count, values); }

    /// \details The same as `Modbus::Client::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)`, but the address of the remote Modbus device is missing. It is preset in the constructor.
    inline StatusCode writeMultipleCoilsAsBoolArray(uint16_t offset, uint16_t count, const bool *values) { return writeMultipleCoilsAsBoolArray(m_unit, offset, count, values); }

public: // Modbus Interface
    StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
    StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
    StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
    StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
    StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value) override;
    StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value) override;
    StatusCode readExceptionStatus(uint8_t unit, uint8_t *value) override;
    StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values) override;
    StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values) override;

public:
    /// \details The same as `Modbus::Client::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    StatusCode readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values);

    /// \details The same as `Modbus::Client::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    StatusCode readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values);

    /// \details The same as `Modbus::Client::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the input buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    StatusCode writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values);

public:
    /// \details Returns the status of the last operation performed.
    inline StatusCode lastStatus() const { return m_lastStatus; }

    /// \details Returns the status of the last error of the performed operation.
    inline StatusCode lastErrorStatus() const { return m_lastErrorStatus; }

private:
    StatusCode request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff);

private:
    uint8_t m_unit;
    ClientPort *m_port;
    ClientPort::RequestParams *m_rp;
    uint8_t m_buff[MB_VALUE_BUFF_SZ];
    StatusCode m_lastStatus;
    StatusCode m_lastErrorStatus;
    String m_lastErrorText;
};

} // namespace Modbus

#endif // MODBUSCLIENT_H
