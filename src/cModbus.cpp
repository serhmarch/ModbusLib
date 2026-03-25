#include "cModbus.h"

#include "Modbus.h"
#include "ModbusPort.h"

#ifndef MB_CLIENT_DISABLE
#include "ModbusClientPort.h"
#include "ModbusClient.h"
#endif // MB_CLIENT_DISABLE

#ifndef MB_SERVER_DISABLE
#include "ModbusServerPort.h"
#include "ModbusTcpServer.h"
#endif // MB_SERVER_DISABLE

class cModbusInterfaceImpl : public ModbusInterface
{
public:
    cModbusInterfaceImpl(cModbusDevice device, cModbusFunctions *functions)
        : m_device(device)
    {
        memcpy(&m_funcs, functions, sizeof(cModbusFunctions));
    }

public:
#ifndef MBF_READ_COILS_DISABLE
    StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        if (m_funcs.readCoils)
            return m_funcs.readCoils(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        if (m_funcs.readDiscreteInputs)
            return m_funcs.readDiscreteInputs(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
    {
        if (m_funcs.readHoldingRegisters)
            return m_funcs.readHoldingRegisters(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
    {
        if (m_funcs.readInputRegisters)
            return m_funcs.readInputRegisters(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
    {
        if (m_funcs.writeSingleCoil)
            return m_funcs.writeSingleCoil(m_device, unit, offset, value);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
    {
        if (m_funcs.writeSingleRegister)
            return m_funcs.writeSingleRegister(m_device, unit, offset, value);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    StatusCode readExceptionStatus(uint8_t unit, uint8_t *status)
    {
        if (m_funcs.readExceptionStatus)
            return m_funcs.readExceptionStatus(m_device, unit, status);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
    StatusCode diagnosticsReturnQueryData(uint8_t unit, const void *indata, uint8_t insize, void *outdata, uint8_t *outsize)
    {
        if (m_funcs.diagnosticsReturnQueryData)
            return m_funcs.diagnosticsReturnQueryData(m_device, unit, indata, insize, outdata, outsize);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
    StatusCode diagnosticsRestartCommunicationsOption(uint8_t unit, bool clearEventLog)
    {
        if (m_funcs.diagnosticsRestartCommunicationsOption)
            return m_funcs.diagnosticsRestartCommunicationsOption(m_device, unit, clearEventLog);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
    StatusCode diagnosticsReturnDiagnosticRegister(uint8_t unit, uint16_t *value)
    {
        if (m_funcs.diagnosticsReturnDiagnosticRegister)
            return m_funcs.diagnosticsReturnDiagnosticRegister(m_device, unit, value);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
    StatusCode diagnosticsChangeAsciiInputDelimiter(uint8_t unit, char delimiter)
    {
        if (m_funcs.diagnosticsChangeAsciiInputDelimiter)
            return m_funcs.diagnosticsChangeAsciiInputDelimiter(m_device, unit, delimiter);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
    StatusCode diagnosticsForceListenOnlyMode(uint8_t unit)
    {
        if (m_funcs.diagnosticsForceListenOnlyMode)
            return m_funcs.diagnosticsForceListenOnlyMode(m_device, unit);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
    StatusCode diagnosticsClearCountersAndDiagnosticRegister(uint8_t unit)
    {
        if (m_funcs.diagnosticsClearCountersAndDiagnosticRegister)
            return m_funcs.diagnosticsClearCountersAndDiagnosticRegister(m_device, unit);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
    StatusCode diagnosticsReturnBusMessageCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnBusMessageCount)
            return m_funcs.diagnosticsReturnBusMessageCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
    StatusCode diagnosticsReturnBusCommunicationErrorCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnBusCommunicationErrorCount)
            return m_funcs.diagnosticsReturnBusCommunicationErrorCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
    StatusCode diagnosticsReturnBusExceptionErrorCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnBusExceptionErrorCount)
            return m_funcs.diagnosticsReturnBusExceptionErrorCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
    StatusCode diagnosticsReturnServerMessageCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnServerMessageCount)
            return m_funcs.diagnosticsReturnServerMessageCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
    StatusCode diagnosticsReturnServerNoResponseCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnServerNoResponseCount)
            return m_funcs.diagnosticsReturnServerNoResponseCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
    StatusCode diagnosticsReturnServerNAKCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnServerNAKCount)
            return m_funcs.diagnosticsReturnServerNAKCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
    StatusCode diagnosticsReturnServerBusyCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnServerBusyCount)
            return m_funcs.diagnosticsReturnServerBusyCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
    StatusCode diagnosticsReturnBusCharacterOverrunCount(uint8_t unit, uint16_t *count)
    {
        if (m_funcs.diagnosticsReturnBusCharacterOverrunCount)
            return m_funcs.diagnosticsReturnBusCharacterOverrunCount(m_device, unit, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    StatusCode diagnosticsClearOverrunCounterAndFlag(uint8_t unit)
    {
        if (m_funcs.diagnosticsClearOverrunCounterAndFlag)
            return m_funcs.diagnosticsClearOverrunCounterAndFlag(m_device, unit);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    StatusCode getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount)
    {
        if (m_funcs.getCommEventCounter)
            return m_funcs.getCommEventCounter(m_device, unit, status, eventCount);
        return Status_BadIllegalFunction;
    }
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    StatusCode getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, void *eventBuff, uint8_t *eventBuffSize)
    {
        if (m_funcs.getCommEventLog)
            return m_funcs.getCommEventLog(m_device, unit, status, eventCount, messageCount, eventBuff, eventBuffSize);
        return Status_BadIllegalFunction;
    }
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
    {
        if (m_funcs.writeMultipleCoils)
            return m_funcs.writeMultipleCoils(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
    {
        if (m_funcs.writeMultipleRegisters)
            return m_funcs.writeMultipleRegisters(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    StatusCode reportServerID(uint8_t unit, void *data, uint8_t *dataSize)
    {
        if (m_funcs.reportServerID)
            return m_funcs.reportServerID(m_device, unit, data, dataSize);
        return Status_BadIllegalFunction;
    }
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_READ_FILE_RECORD_DISABLE
    StatusCode readFileRecord(uint8_t unit, const FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize)
    {
        if (m_funcs.readFileRecord)
            return m_funcs.readFileRecord(m_device, unit, records, recordsCount, outData, outSize);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    StatusCode writeFileRecord(uint8_t unit, const FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize)
    {
        if (m_funcs.writeFileRecord)
            return m_funcs.writeFileRecord(m_device, unit, records, recordsCount, inData, inSize);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_FILE_RECORD_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
    {
        if (m_funcs.maskWriteRegister)
            return m_funcs.maskWriteRegister(m_device, unit, offset, andMask, orMask);
        return Status_BadIllegalFunction;
    }
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
    {
        if (m_funcs.readWriteMultipleRegisters)
            return m_funcs.readWriteMultipleRegisters(m_device, unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *values, uint16_t *count)
    {
        if (m_funcs.readFIFOQueue)
            return m_funcs.readFIFOQueue(m_device, unit, fifoadr, values, count);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
    StatusCode readDeviceIdentification(uint8_t unit, uint8_t readDeviceId, uint8_t objectId, void *data, uint8_t *dataSize, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)
    {
        if (m_funcs.readDeviceIdentification)
            return m_funcs.readDeviceIdentification(m_device, unit, readDeviceId, objectId, data, dataSize, numberOfObjects, conformityLevel, moreFollows, nextObjectId);
        return Status_BadIllegalFunction;
    }
#endif // MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
#endif // MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

private:
    cModbusDevice m_device;
    cModbusFunctions m_funcs;
};

cModbusInterface cCreateModbusDevice(cModbusDevice device, cModbusFunctions *functions)
{
    return new cModbusInterfaceImpl(device, functions);
}

void cDeleteModbusDevice(cModbusInterface dev)
{
    delete static_cast<cModbusInterfaceImpl *>(dev);
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------- ModbusPort ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

cModbusPort cPortCreate(ProtocolType type, const void *settings, bool blocking)
{
    return createPort(type, settings, blocking);
}

void cPortDelete(cModbusPort port)
{
    delete port;
}

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusClientPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

#ifndef MB_CLIENT_DISABLE

cModbusClientPort cCpoCreate(ProtocolType type, const void *settings, bool blocking)
{
    return createClientPort(type, settings, blocking);
}

cModbusClientPort cCpoCreateForPort(cModbusPort port)
{
    return new ModbusClientPort(port);
}

void cCpoDelete(cModbusClientPort clientPort)
{
    delete clientPort;
}

const Char *cCpoGetObjectName(cModbusClientPort clientPort)
{
    return clientPort->objectName();
}

void cCpoSetObjectName(cModbusClientPort clientPort, const Char *name)
{
    clientPort->setObjectName(name);
}

ProtocolType cCpoGetType(cModbusClientPort clientPort)
{
    return clientPort->type();
}

bool cCpoIsOpen(cModbusClientPort clientPort)
{
    return clientPort->isOpen();
}

bool cCpoClose(cModbusClientPort clientPort)
{
    return clientPort->close();
}

uint32_t cCpoGetRepeatCount(cModbusClientPort clientPort)
{
    return clientPort->repeatCount();
}

void cCpoSetRepeatCount(cModbusClientPort clientPort, uint32_t count)
{
    clientPort->setRepeatCount(count);
}

#ifndef MBF_READ_COILS_DISABLE
StatusCode cCpoReadCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return clientPort->readCoils(unit, offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cCpoReadDiscreteInputs(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return clientPort->readDiscreteInputs(unit, offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode cCpoReadHoldingRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return clientPort->readHoldingRegisters(unit, offset, count, values);
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode cCpoReadInputRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return clientPort->readInputRegisters(unit, offset, count, values);
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode cCpoWriteSingleCoil(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, bool value)
{
    return clientPort->writeSingleCoil(unit, offset, value);
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode cCpoWriteSingleRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t value)
{
    return clientPort->writeSingleRegister(unit, offset, value);
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode cCpoReadExceptionStatus(cModbusClientPort clientPort, uint8_t unit, uint8_t *value)
{
    return clientPort->readExceptionStatus(unit, value);
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
StatusCode cCpoDiagnosticsReturnQueryData(cModbusClientPort clientPort, uint8_t unit, const void *indata, uint8_t insize, void *outdata, uint8_t *outsize)
{
    return clientPort->diagnosticsReturnQueryData(unit, indata, insize, outdata, outsize);
}
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
StatusCode cCpoDiagnosticsRestartCommunicationsOption(cModbusClientPort clientPort, uint8_t unit, bool clearEventLog)
{
    return clientPort->diagnosticsRestartCommunicationsOption(unit, clearEventLog);
}
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
StatusCode cCpoDiagnosticsReturnDiagnosticRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t *value)
{
    return clientPort->diagnosticsReturnDiagnosticRegister(unit, value);
}
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
StatusCode cCpoDiagnosticsChangeAsciiInputDelimiter(cModbusClientPort clientPort, uint8_t unit, char delimiter)
{
    return clientPort->diagnosticsChangeAsciiInputDelimiter(unit, delimiter);
}
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
StatusCode cCpoDiagnosticsForceListenOnlyMode(cModbusClientPort clientPort, uint8_t unit)
{
    return clientPort->diagnosticsForceListenOnlyMode(unit);
}
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
StatusCode cCpoDiagnosticsClearCountersAndDiagnosticRegister(cModbusClientPort clientPort, uint8_t unit)
{
    return clientPort->diagnosticsClearCountersAndDiagnosticRegister(unit);
}
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnBusMessageCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnBusMessageCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnBusCommunicationErrorCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnBusCommunicationErrorCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnBusExceptionErrorCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnBusExceptionErrorCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnServerMessageCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnServerMessageCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnServerNoResponseCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnServerNoResponseCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnServerNAKCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnServerNAKCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnServerBusyCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnServerBusyCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
StatusCode cCpoDiagnosticsReturnBusCharacterOverrunCount(cModbusClientPort clientPort, uint8_t unit, uint16_t *count)
{
    return clientPort->diagnosticsReturnBusCharacterOverrunCount(unit, count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
StatusCode cCpoDiagnosticsClearOverrunCounterAndFlag(cModbusClientPort clientPort, uint8_t unit)
{
    return clientPort->diagnosticsClearOverrunCounterAndFlag(unit);
}
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
StatusCode cCpoGetCommEventCounter(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    return clientPort->getCommEventCounter(unit, status, eventCount);
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
StatusCode cCpoGetCommEventLog(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, void *eventBuff, uint8_t *eventBuffSize)
{
    return clientPort->getCommEventLog(unit, status, eventCount, messageCount, eventBuff, eventBuffSize);
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cCpoWriteMultipleCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    return clientPort->writeMultipleCoils(unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cCpoWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    return clientPort->writeMultipleRegisters(unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode cCpoReportServerID(cModbusClientPort clientPort, uint8_t unit, void *data, uint8_t *dataSize)
{
    return clientPort->reportServerID(unit, data, dataSize);
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_READ_FILE_RECORD_DISABLE
StatusCode cCpoReadFileRecord(cModbusClientPort clientPort, uint8_t unit, const FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize)
{
    return clientPort->readFileRecord(unit, records, recordsCount, outData, outSize);
}
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
StatusCode cCpoWriteFileRecord(cModbusClientPort clientPort, uint8_t unit, const FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize)
{
    return clientPort->writeFileRecord(unit, records, recordsCount, inData, inSize);
}
#endif // MBF_WRITE_FILE_RECORD_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode cCpoMaskWriteRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    return clientPort->maskWriteRegister(unit, offset, andMask, orMask);
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cCpoReadWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    return clientPort->readWriteMultipleRegisters(unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
StatusCode cCpoReadFIFOQueue(cModbusClientPort clientPort, uint8_t unit, uint16_t fifoadr, uint16_t *values, uint16_t *count)
{
    return clientPort->readFIFOQueue(unit, fifoadr, values, count);
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
StatusCode cCpoReadDeviceIdentification(cModbusClientPort clientPort, uint8_t unit, uint8_t readDeviceId, uint8_t objectId, void *data, uint8_t *dataSize, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)
{
    return clientPort->readDeviceIdentification(unit, readDeviceId, objectId, data, dataSize, numberOfObjects, conformityLevel, moreFollows, nextObjectId);
}
#endif // MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
#endif // MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

#ifndef MBF_READ_COILS_DISABLE
StatusCode cCpoReadCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    return clientPort->readCoilsAsBoolArray(unit, offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cCpoReadDiscreteInputsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    return clientPort->readDiscreteInputsAsBoolArray(unit, offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cCpoWriteMultipleCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const bool *values)
{
    return clientPort->writeMultipleCoilsAsBoolArray(unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

StatusCode cCpoGetLastStatus(cModbusClientPort clientPort)
{
    return clientPort->lastStatus();
}

StatusCode cCpoGetLastErrorStatus(cModbusClientPort clientPort)
{
    return clientPort->lastErrorStatus();
}

const Char *cCpoGetLastErrorText(cModbusClientPort clientPort)
{
    return clientPort->lastErrorText();
}

void cCpoConnectOpened(cModbusClientPort clientPort, pfSlotOpened funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalOpened, funcPtr);
}

void cCpoConnectClosed(cModbusClientPort clientPort, pfSlotClosed funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalClosed, funcPtr);
}

void cCpoConnectTx(cModbusClientPort clientPort, pfSlotTx funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalTx, funcPtr);
}

void cCpoConnectRx(cModbusClientPort clientPort, pfSlotRx funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalRx, funcPtr);
}

void cCpoConnectError(cModbusClientPort clientPort, pfSlotError funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalError, funcPtr);
}

void cCpoDisconnectFunc(cModbusClientPort clientPort, void *funcPtr)
{
    clientPort->disconnectFunc(funcPtr);
}


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------- ModbusClient ---------------------------------------------
// --------------------------------------------------------------------------------------------------------

cModbusClient cCliCreate(uint8_t unit, ProtocolType type, const void *settings, bool blocking)
{
    ModbusClientPort *clientPort = createClientPort(type, settings, blocking);
    ModbusClient *client = new ModbusClient(unit, clientPort);
    return client;
}

cModbusClient cCliCreateForClientPort(uint8_t unit, cModbusClientPort clientPort)
{
    ModbusClient *client = new ModbusClient(unit, clientPort);
    return client;
}

void cCliDelete(cModbusClient client)
{
    delete client;
}

const Char *cCliGetObjectName(cModbusClient client)
{
    return client->objectName();
}

void cCliSetObjectName(cModbusClient client, const Char *name)
{
    client->setObjectName(name);
}

ProtocolType cCliGetType(cModbusClient client)
{
    return client->type();
}

uint8_t cCliGetUnit(cModbusClient client)
{
    return client->unit();
}

void cCliSetUnit(cModbusClient client, uint8_t unit)
{
    client->setUnit(unit);
}

bool cCliIsOpen(cModbusClient client)
{
    return client->isOpen();
}

cModbusClientPort cCliGetPort(cModbusClient client)
{
    return client->port();
}

#ifndef MBF_READ_COILS_DISABLE
StatusCode cReadCoils(cModbusClient client, uint16_t offset, uint16_t count, void *values)
{
    return client->readCoils(offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cReadDiscreteInputs(cModbusClient client, uint16_t offset, uint16_t count, void *values)
{
    return client->readDiscreteInputs(offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode cReadHoldingRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values)
{
    return client->readHoldingRegisters(offset, count, values);
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode cReadInputRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values)
{
    return client->readInputRegisters(offset, count, values);
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode cWriteSingleCoil(cModbusClient client, uint16_t offset, bool value)
{
    return client->writeSingleCoil(offset, value);
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode cWriteSingleRegister(cModbusClient client, uint16_t offset, uint16_t value)
{
    return client->writeSingleRegister(offset, value);
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode cReadExceptionStatus(cModbusClient client, uint8_t *value)
{
    return client->readExceptionStatus(value);
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
StatusCode cDiagnosticsReturnQueryData(cModbusClient client, const void *indata, uint8_t insize, void *outdata, uint8_t *outsize)
{
    return client->diagnosticsReturnQueryData(indata, insize, outdata, outsize);
}
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
StatusCode cDiagnosticsRestartCommunicationsOption(cModbusClient client, bool clearEventLog)
{
    return client->diagnosticsRestartCommunicationsOption(clearEventLog);
}
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
StatusCode cDiagnosticsReturnDiagnosticRegister(cModbusClient client, uint16_t *value)
{
    return client->diagnosticsReturnDiagnosticRegister(value);
}
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
StatusCode cDiagnosticsChangeAsciiInputDelimiter(cModbusClient client, char delimiter)
{
    return client->diagnosticsChangeAsciiInputDelimiter(delimiter);
}
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
StatusCode cDiagnosticsForceListenOnlyMode(cModbusClient client)
{
    return client->diagnosticsForceListenOnlyMode();
}
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
StatusCode cDiagnosticsClearCountersAndDiagnosticRegister(cModbusClient client)
{
    return client->diagnosticsClearCountersAndDiagnosticRegister();
}
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
StatusCode cDiagnosticsReturnBusMessageCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnBusMessageCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
StatusCode cDiagnosticsReturnBusCommunicationErrorCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnBusCommunicationErrorCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
StatusCode cDiagnosticsReturnBusExceptionErrorCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnBusExceptionErrorCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
StatusCode cDiagnosticsReturnServerMessageCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnServerMessageCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
StatusCode cDiagnosticsReturnServerNoResponseCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnServerNoResponseCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
StatusCode cDiagnosticsReturnServerNAKCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnServerNAKCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
StatusCode cDiagnosticsReturnServerBusyCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnServerBusyCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
StatusCode cDiagnosticsReturnBusCharacterOverrunCount(cModbusClient client, uint16_t *count)
{
    return client->diagnosticsReturnBusCharacterOverrunCount(count);
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
StatusCode cDiagnosticsClearOverrunCounterAndFlag(cModbusClient client)
{
    return client->diagnosticsClearOverrunCounterAndFlag();
}
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
StatusCode cGetCommEventCounter(cModbusClient client, uint16_t *status, uint16_t *eventCount)
{
    return client->getCommEventCounter(status, eventCount);
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
StatusCode cGetCommEventLog(cModbusClient client, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, void *eventBuff, uint8_t *eventBuffSize)
{
    return client->getCommEventLog(status, eventCount, messageCount, eventBuff, eventBuffSize);
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cWriteMultipleCoils(cModbusClient client, uint16_t offset, uint16_t count, const void *values)
{
    return client->writeMultipleCoils(offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cWriteMultipleRegisters(cModbusClient client, uint16_t offset, uint16_t count, const uint16_t *values)
{
    return client->writeMultipleRegisters(offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode cReportServerID(cModbusClient client, void *data, uint8_t *dataSize)
{
    return client->reportServerID(data, dataSize);
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_READ_FILE_RECORD_DISABLE
StatusCode cReadFileRecord(cModbusClient client, const FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize)
{
    return client->readFileRecord(records, recordsCount, outData, outSize);
}
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
StatusCode cWriteFileRecord(cModbusClient client, const FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize)
{
    return client->writeFileRecord(records, recordsCount, inData, inSize);
}
#endif // MBF_WRITE_FILE_RECORD_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode cMaskWriteRegister(cModbusClient client, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    return client->maskWriteRegister(offset, andMask, orMask);
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cReadWriteMultipleRegisters(cModbusClient client, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    return client->readWriteMultipleRegisters(readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
StatusCode cReadFIFOQueue(cModbusClient client, uint16_t fifoadr, uint16_t *values, uint16_t *count)
{
    return client->readFIFOQueue(fifoadr, values, count);
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
StatusCode cReadDeviceIdentification(cModbusClient client, uint8_t readDeviceId, uint8_t objectId, void *data, uint8_t *dataSize, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)
{
    return client->readDeviceIdentification(readDeviceId, objectId, data, dataSize, numberOfObjects, conformityLevel, moreFollows, nextObjectId);
}
#endif // MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
#endif // MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

#ifndef MBF_READ_COILS_DISABLE
StatusCode cReadCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values)
{
    return client->readCoilsAsBoolArray(offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cReadDiscreteInputsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values)
{
    return client->readDiscreteInputsAsBoolArray(offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cWriteMultipleCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, const bool *values)
{
    return client->writeMultipleCoilsAsBoolArray(offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

StatusCode cCliGetLastPortStatus(cModbusClient client)
{
    return client->lastPortStatus();
}

StatusCode cCliGetLastPortErrorStatus(cModbusClient client)
{
    return client->lastPortErrorStatus();
}

const Char *cCliGetLastPortErrorText(cModbusClient client)
{
    return client->lastPortErrorText();
}

#endif // MB_CLIENT_DISABLE


// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusServerPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

#ifndef MB_SERVER_DISABLE

cModbusServerPort cSpoCreate(cModbusInterface device, ProtocolType type, const void *settings, bool blocking)
{
    return createServerPort(device, type, settings, blocking);
}

void cSpoDelete(cModbusServerPort serverPort)
{
    delete serverPort;
}

const Char *cSpoGetObjectName(cModbusServerPort serverPort)
{
    return serverPort->objectName();
}

void cSpoSetObjectName(cModbusServerPort serverPort, const Char *name)
{
    serverPort->setObjectName(name);
}

ProtocolType cSpoGetType(cModbusServerPort serverPort)
{
    return serverPort->type();
}

bool cSpoIsTcpServer(cModbusServerPort serverPort)
{
    return serverPort->isTcpServer();
}

cModbusInterface cSpoGetDevice(cModbusServerPort serverPort)
{
    return serverPort->device();
}

bool cSpoIsOpen(cModbusServerPort serverPort)
{
    return serverPort->isOpen();
}

StatusCode cSpoOpen(cModbusServerPort serverPort)
{
    return serverPort->open();
}

StatusCode cSpoClose(cModbusServerPort serverPort)
{
    return serverPort->close();
}

StatusCode cSpoProcess(cModbusServerPort serverPort)
{
    return serverPort->process();
}

void cSpoConnectOpened(cModbusServerPort serverPort, pfSlotOpened funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalOpened, funcPtr);
}

void cSpoConnectClosed(cModbusServerPort serverPort, pfSlotClosed funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalClosed, funcPtr);
}

void cSpoConnectTx(cModbusServerPort serverPort, pfSlotTx funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalTx, funcPtr);
}

void cSpoConnectRx(cModbusServerPort serverPort, pfSlotRx funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalRx, funcPtr);
}

void cSpoConnectError(cModbusServerPort serverPort, pfSlotError funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalError, funcPtr);
}

void cSpoConnectNewConnection(cModbusServerPort serverPort, pfSlotNewConnection funcPtr)
{
    if (serverPort->isTcpServer())
        static_cast<ModbusTcpServer*>(serverPort)->connect(&ModbusTcpServer::signalNewConnection, funcPtr);
}

void cSpoConnectCloseConnection(cModbusServerPort serverPort, pfSlotCloseConnection funcPtr)
{
    if (serverPort->isTcpServer())
        static_cast<ModbusTcpServer*>(serverPort)->connect(&ModbusTcpServer::signalCloseConnection, funcPtr);
}

void cSpoDisconnectFunc(cModbusServerPort serverPort, void *funcPtr)
{
    serverPort->disconnectFunc(funcPtr);
}

#endif // MB_SERVER_DISABLE
