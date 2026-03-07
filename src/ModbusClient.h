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

/*! \brief The `ModbusClient` class is wrapper to simplify usage of `ModbusClientPort`.

    \details The ModbusClient class provides a convenient wrapper around ModbusClientPort that simplifies
    communication with a specific remote Modbus device. It encapsulates the device unit address, eliminating
    the need to specify it with each function call.
    
    Key characteristics:
    - Binds to a specific unit address on construction
    - Wraps all standard Modbus functions without requiring unit parameter
    - Delegates actual communication to the underlying ModbusClientPort
    - Provides access to port connection state and error information
    - Thread-safe when used with proper port synchronization
    - Works with any protocol (TCP, RTU, ASCII) through polymorphic port
    
    This implementation provides:
    - Automatic unit address management for all Modbus function calls
    - Simplified API for applications communicating with single devices
    - Pass-through access to port status and error information
    - Support for all standard Modbus functions (01-24)
    - Optional bool array variants for coil operations
    - Diagnostic and special functions (where enabled)
    
    Usage pattern:
    \code{.cpp}
    // Create and configure the port (settings definition omitted for brevity)
    ModbusClientPort *port = Modbus::createClientPort(Modbus::TCP, &settings, false);
    // Create clients bound to unit addresses
    ModbusClient c1(1, port);
    ModbusClient c2(2, port);
    ModbusClient c3(3, port);

    Modbus::StatusCode s1, s2, s3;
    uint16_t regs1[10], regs2[10], regs3[10];
    
    // Read holding registers without specifying unit address
    while(1)
    {
        s1 = c1.readHoldingRegisters(0, 10, regs1);
        s2 = c2.readHoldingRegisters(0, 10, regs2);
        s3 = c3.readHoldingRegisters(0, 10, regs3);
        Modbus::msleep(1);
    }
    \endcode
    
    The ModbusClient simplifies scenarios where an application communicates with a specific device,
    making code more concise and reducing the chance of addressing errors.

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
    /// \details Same as `ModbusClientPort::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readCoils(uint16_t offset, uint16_t count, void *values);

    /// \details Same as `ModbusClientPort::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readCoilsAsBoolArray(uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Same as `ModbusClientPort::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readDiscreteInputs(uint16_t offset, uint16_t count, void *values);

    /// \details Same as `ModbusClientPort::readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readDiscreteInputsAsBoolArray(uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readHoldingRegisters(uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readInputRegisters(uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    /// \details Same as `ModbusClientPort::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeSingleCoil(uint16_t offset, bool value);
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeSingleRegister(uint16_t offset, uint16_t value);
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    /// \details Same as `ModbusClientPort::readExceptionStatus(uint8_t unit, uint8_t *status)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readExceptionStatus(uint8_t *value);
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnQueryData(uint8_t unit, uint8_t insize, const void *indata, uint8_t maxOutsize, void *outdata, uint8_t *outsize)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnQueryData(uint8_t insize, const void *indata, uint8_t *outsize, void *outdata);
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsRestartCommunicationsOption(uint8_t unit, bool clearEventLog)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsRestartCommunicationsOption(bool clearEventLog);
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnDiagnosticRegister(uint8_t unit, uint16_t *value)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnDiagnosticRegister(uint16_t *value);
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsChangeAsciiInputDelimiter(uint8_t unit, uint16_t *value)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsChangeAsciiInputDelimiter(char delimiter);
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsForceListenOnlyMode(uint8_t unit)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsForceListenOnlyMode();
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsClearCountersAndDiagnosticRegister(uint8_t unit)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsClearCountersAndDiagnosticRegister();
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnBusMessageCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnBusMessageCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnBusCommunicationErrorCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnBusCommunicationErrorCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnBusExceptionErrorCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnBusExceptionErrorCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnServerMessageCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnServerMessageCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnServerNoResponseCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnServerNoResponseCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnServerNAKCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnServerNAKCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnServerBusyCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnServerBusyCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsReturnBusCharacterOverrunCount(uint8_t unit, uint16_t *count)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsReturnBusCharacterOverrunCount(uint16_t *count);
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    /// \details Same as `ModbusClientPort::diagnosticsClearOverrunCounterAndFlag(uint8_t unit)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode diagnosticsClearOverrunCounterAndFlag();
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    /// \details Same as `ModbusClientPort::getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode getCommEventCounter(uint16_t *status, uint16_t *eventCount);
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    /// \details Same as `ModbusClientPort::getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *events)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode getCommEventLog(uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Same as `ModbusClient::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeMultipleCoils(uint16_t offset, uint16_t count, const void *values);

    /// \details Same as `ModbusClient::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeMultipleCoilsAsBoolArray(uint16_t offset, uint16_t count, const bool *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeMultipleRegisters(uint16_t offset, uint16_t count, const uint16_t *values);
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    /// \details Same as `ModbusClientPort::reportServerID(uint8_t unit, uint8_t *count, uint8_t *data)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode reportServerID(uint8_t *count, uint8_t *data);
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_READ_FILE_RECORD_DISABLE
    /// \details Same as `ModbusClientPort::readFileRecord(uint8_t unit, uint8_t recordsCount, const Modbus::FileRecord *records, uint8_t *outSize, void *outData)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readFileRecord(uint8_t recordsCount, const Modbus::FileRecord *records, uint8_t *outSize, void *outData);
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    /// \details Same as `ModbusClientPort::writeFileRecord(uint8_t unit, uint8_t recordsCount, const Modbus::FileRecord *records, const void *inData)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode writeFileRecord(uint8_t recordsCount, const Modbus::FileRecord *records, const void *inData);
#endif // MBF_WRITE_FILE_RECORD_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    /// \details Same as `ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode maskWriteRegister(uint16_t offset, uint16_t andMask, uint16_t orMask);
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Same as `ModbusClientPort::readWriteMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readWriteMultipleRegisters(uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    /// \details Same as `ModbusClientPort::readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readFIFOQueue(uint16_t fifoadr, uint16_t *count, uint16_t *values);
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
    /// \details Same as `ModbusClientPort::readDeviceIdentification(uint8_t unit, uint8_t readDeviceId, uint8_t objectId, uint8_t *dataSize, void *data, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)`,
    /// but the `unit` address of the remote Modbus device is missing. It is preset in the constructor.
    Modbus::StatusCode readDeviceIdentification(uint8_t readDeviceId, uint8_t objectId, uint8_t *dataSize, void *data, uint8_t *numberOfObjects = nullptr, uint8_t *conformityLevel = nullptr, bool *moreFollows = nullptr, uint8_t *nextObjectId = nullptr);
#endif // MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE

#endif // MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

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
