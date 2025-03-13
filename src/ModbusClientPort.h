/*!
 * \file   ModbusClientPort.h
 * \brief  General file of the algorithm of the client part of the Modbus protocol port.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSCLIENTPORT_H
#define MODBUSCLIENTPORT_H

#include "ModbusObject.h"

class ModbusPort;

/*! \brief The `ModbusClientPort` class implements the algorithm of the client part of the Modbus communication protocol port.

    \details `ModbusClient` contains a list of Modbus functions that are implemented by the Modbus client program.
    It implements functions for reading and writing various types of Modbus memory defined by the specification.
    In the non blocking mode if the operation is not completed it returns the intermediate status `Modbus::Status_Processing`,
    and then it must be called until it is successfully completed or returns an error status.

    `ModbusClientPort` has number of Modbus functions with interface like `readCoils(ModbusObject *client, ...)`. 
    Several clients can automatically share a current `ModbusClientPort` resource. The first one to access the port seizes 
    the resource until the operation with the remote device is completed. Then the first client will release the resource and 
    the next client in the queue will capture it, and so on in a circle.

```cpp
#include <ModbusClient.h>
//...
void main()
{
    //...
    ModbusClientPort *port = Modbus::createClientPort(Modbus::TCP, &settings, false);
    ModbusClient c1(1, port);
    ModbusClient c2(2, port);
    ModbusClient c3(3, port);
    Modbus::StatusCode s1, s2, s3;
    //...
    while(1)
    {
        s1 = c1.readHoldingRegisters(0, 10, values);
        s2 = c2.readHoldingRegisters(0, 10, values);
        s3 = c3.readHoldingRegisters(0, 10, values);
        doSomeOtherStuffInCurrentThread();
        Modbus::msleep(1);
    }
    //...
}
//...
```

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
    /// \param[in]  port A pointer to the port object which belongs to this client object.
    /// Lifecycle of the `port` object is managed by this `ModbusClientPort`-object
    ModbusClientPort(ModbusPort *port);

public:
    /// \details Returns type of Modbus protocol.
    Modbus::ProtocolType type() const;

    /// \details Returns a pointer to the port object that is used by this algorithm.
    ModbusPort *port() const;

    /// \details Set new port object for current client port control. Previous port object is deleted.
    void setPort(ModbusPort *port);

    /// \details Closes connection and returns status of the operation.
    Modbus::StatusCode close();

    /// \details Returns `true` if the connection with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Returns the setting of the number of tries of the Modbus request if it fails.
    uint32_t tries() const;

    /// \details Sets the number of tries a Modbus request is repeated if it fails.
    void setTries(uint32_t v);

    /// \details Same as `tries()`. Used for backward compatibility.
    inline uint32_t repeatCount() const { return tries(); }

    /// \details Same as `setTries()`. Used for backward compatibility.
    inline void setRepeatCount(uint32_t v) { setTries(v); }

    /// \details Returns `true` if broadcast mode for `0` unit address is enabled, `false` otherwise.
    /// Broadcast mode for `0` unit address is required by Modbus protocol so it is enabled by default
    bool isBroadcastEnabled() const;

    /// \details Enables broadcast mode for `0` unit address. It is enabled by default.
    /// \sa `isBroadcastEnabled()`
    void setBroadcastEnabled(bool enable);

public: // Main interface

#ifndef MBF_READ_COILS_DISABLE
    /// \details Same as `ModbusClientPort::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Same as `ModbusClientPort::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readDiscreteInputs(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readHoldingRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readInputRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    /// \details Same as `ModbusClientPort::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode writeSingleCoil(ModbusObject *client, uint8_t unit, uint16_t offset, bool value);
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode writeSingleRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t value);
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    /// \details Same as `ModbusClientPort::readExceptionStatus(uint8_t unit, uint8_t *status)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readExceptionStatus(ModbusObject *client, uint8_t unit, uint8_t *value);
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    /// \details Same as `ModbusClientPort::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode diagnostics(ModbusObject *client, uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata);
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    /// \details Same as `ModbusClientPort::getCommEventCounter(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode getCommEventCounter(ModbusObject *client, uint8_t unit, uint16_t *status, uint16_t *eventCount);
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    /// \details Same as `ModbusClientPort::getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *events)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode getCommEventLog(ModbusObject *client, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode writeMultipleCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const void *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode writeMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    /// \details Same as `ModbusClientPort::reportServerID(uint8_t unit, uint8_t *count, uint8_t *data)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode reportServerID(ModbusObject *client, uint8_t unit, uint8_t *count, uint8_t *data);
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode maskWriteRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask);
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readWriteMultipleRegisters(uint8_t unit, uint16_t offset, readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readWriteMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    /// \details Same as `ModbusClientPort::readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readFIFOQueue(ModbusObject *client, uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values);
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
    /// \details Same as `ModbusClientPort::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Same as `ModbusClientPort::readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode readDiscreteInputsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)` but has `client` as first parameter to seize current `ModbusClientPort` resource.
    Modbus::StatusCode writeMultipleCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const bool *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

public: // Modbus Interface

#ifndef MBF_READ_COILS_DISABLE
    Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value) override;
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value) override;
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *value) override;
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    Modbus::StatusCode diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata) override;
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    Modbus::StatusCode getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount) override;
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    Modbus::StatusCode getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff) override;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values) override;
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values) override;
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    Modbus::StatusCode reportServerID(uint8_t unit, uint8_t *count, uint8_t *data) override;
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    Modbus::StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask) override;
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    Modbus::StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues) override;
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    Modbus::StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values) override;
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
    /// \details Same as `ModbusClientPort::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    inline Modbus::StatusCode readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values) { return readCoilsAsBoolArray(this, unit, offset, count, values); }
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Same as `ModbusClientPort::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`, but the output buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    inline Modbus::StatusCode readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values) { return readDiscreteInputsAsBoolArray(this, unit, offset, count, values); }
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`, but the input buffer of values `values` is an array, where each discrete value is located in a separate element of the array of type `bool`.
    inline Modbus::StatusCode writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values) { return writeMultipleCoilsAsBoolArray(this, unit, offset, count, values); }
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

public:
    /// \details Returns the status of the last operation performed.
    Modbus::StatusCode lastStatus() const;

    /// \details Returns the timestamp of the last operation performed.
    Modbus::Timestamp lastStatusTimestamp() const;

    /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastErrorStatus() const;

    /// \details Returns the text of the last error of the performed operation.
    const Modbus::Char *lastErrorText() const;

public:
    /// \details Returns a pointer to the client object whose request is currently being processed by the current port.
    const ModbusObject *currentClient() const;

    /// \details Returns status the current request for `client`.\n
    /// The client usually calls this function to determine whether its request is pending/finished/blocked.
    /// If function returns `Enable`, `client` has just became current and can make request to the port, 
    /// `Process` - current `client` is already processing,
    /// `Disable` - other client owns the port.
    RequestStatus getRequestStatus(ModbusObject *client);

    /// \details Cancels the previous request specified by the `*rp` pointer for the client.
    void cancelRequest(ModbusObject *client);

public: // SIGNALS
    /// \details Calls each callback of the port when the port is opened. `source` - current port's name
    void signalOpened(const Modbus::Char *source);

    /// \details Calls each callback of the port when the port is closed. `source` - current port's name
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
