/*!
 * \file   ModbusClientPort.h
 * \brief General file of the algorithm of the client part of the Modbus protocol port.
 *
 * \author march
 * \date   May 2024
 */
#ifndef MODBUSCLIENTPORT_H
#define MODBUSCLIENTPORT_H

#include "ModbusObject.h"

class ModbusPort;

/*! \brief The `ModbusClientPort` class implements the algorithm of the client part of the Modbus communication protocol port.

    \details `ModbusClient` contains a list of Modbus functions that are implemented by the Modbus client program.
    It implements functions for reading and writing various types of Modbus memory defined by the specification.
    The operations that return `Modbus::StatusCode` are asynchronous, that is, if the operation is not completed, it returns the intermediate status `Modbus::Status_Processing`,
    and then it must be called until it is successfully completed or returns an error status.

 */

class MODBUS_EXPORT ModbusClientPort : public ModbusObject, public ModbusInterface
{
public:
    /*! \brief Sets the status of the request for the client.
     */
    enum RequestStatus
    {
        Enable,
        Disable,
        Process
    };

public:
    /// \details Constructor of the class.
    /// \param[in]  port A pointer to the port object to which this client object belongs.
    ModbusClientPort(ModbusPort *port);

public:
    /// \details Returns type of Modbus protocol.
    Modbus::ProtocolType type() const;

    /// \details Returns a pointer to the port object that uses this algorithm.
    ModbusPort *port() const;

    /// \details Closes connection and returns status of the operation.
    Modbus::StatusCode close();

    /// \details Returns `true` if the connection with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Returns the setting of the number of repetitions of the Modbus request if it fails.
    uint32_t repeatCount() const;

    /// \details Sets the number of times a Modbus request is repeated if it fails.
    void setRepeatCount(uint32_t v);

public: // Main interface
    Modbus::StatusCode readCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values);
    Modbus::StatusCode readDiscreteInputs(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values);
    Modbus::StatusCode readHoldingRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
    Modbus::StatusCode readInputRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
    Modbus::StatusCode writeSingleCoil(ModbusObject *client, uint8_t unit, uint16_t offset, bool value);
    Modbus::StatusCode writeSingleRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t value);
    Modbus::StatusCode readExceptionStatus(ModbusObject *client, uint8_t unit, uint8_t *value);
    Modbus::StatusCode writeMultipleCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const void *values);
    Modbus::StatusCode writeMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
    Modbus::StatusCode readCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values);
    Modbus::StatusCode readDiscreteInputsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values);
    Modbus::StatusCode writeMultipleCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const bool *values);

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
    inline Modbus::StatusCode readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values) { return readCoilsAsBoolArray(this, unit, offset, count, values); }

    /// \details The same as `ModbusClient::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    inline Modbus::StatusCode readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values) { return readDiscreteInputsAsBoolArray(this, unit, offset, count, values); }

    /// \details The same as `ModbusClient::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the input buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    inline Modbus::StatusCode writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values) { return writeMultipleCoilsAsBoolArray(this, unit, offset, count, values); }

public:
    /// \details Returns the status of the last operation performed.
    Modbus::StatusCode lastStatus() const;

     /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastErrorStatus() const;

   /// \details Returns the text of the last error of the performed operation.
    const Modbus::Char *lastErrorText() const;

public:
    /// \details Returns a pointer to the client object whose request is currently being processed by the algorithm.
    const ModbusObject *currentClient() const;

    /// \details Deletes the request structure `*rp` for the client.\n
    /// The client usually calls this function to determine whether its request is pending/finished/blocked.
    RequestStatus getRequestStatus(ModbusObject *client);

    /// \details Cancels the previous request specified by the `*rp` pointer for the client.
    void cancelRequest(ModbusObject *client);

public: // SIGNALS
    /// \details
    void signalOpened(const Modbus::Char *source);

    /// \details
    void signalClosed(const Modbus::Char *source);

    /// \details Calls each callback of the original packet 'Tx' from the internal list of callbacks, passing them the original array 'buff' and its size 'size'.
    void signalTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Calls each callback of the incoming packet 'Rx' from the internal list of callbacks, passing them the input array 'buff' and its size 'size'.
    void signalRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Calls each callback of the port when error is occured with error's status and text.
    void signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text);

private:
    Modbus::StatusCode request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff);
    Modbus::StatusCode process();
    friend class ModbusClient;
};

#endif // MODBUSCLIENTPORT_H
