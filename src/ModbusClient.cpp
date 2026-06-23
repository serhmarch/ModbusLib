#include "ModbusClient.h"
#include "ModbusClient_p.h"

inline ModbusClientPrivate *d_cast(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusClientPrivate*>(d_ptr); }

ModbusClient::ModbusClient(uint8_t unit, ModbusClientPort *port) :
    ModbusObject(new ModbusClientPrivate)
{
    ModbusClientPrivate *d = d_cast(d_ptr);
    d->unit            = unit;
    d->port            = port;
#ifndef MB_CLIENT_REPEAT_DISABLE
    d->innerTries      = 0;
    d->settings.tries  = 1;
#endif // MB_CLIENT_REPEAT_DISABLE
}


Modbus::ProtocolType ModbusClient::type() const
{
    return d_cast(d_ptr)->port->type();
}

uint8_t ModbusClient::unit() const
{
    return d_cast(d_ptr)->unit;
}

void ModbusClient::setUnit(uint8_t unit)
{
    d_cast(d_ptr)->unit = unit;
}

bool ModbusClient::isOpen() const
{
    return d_cast(d_ptr)->port->isOpen();
}

ModbusClientPort *ModbusClient::port() const
{
    return d_cast(d_ptr)->port;
}

uint32_t ModbusClient::tries() const
{
    return d_cast(d_ptr)->settings.tries;
}

void ModbusClient::setTries(uint32_t v)
{
    if (v > 0)
        d_cast(d_ptr)->settings.tries = v;
}


#ifdef MB_CLIENT_REPEAT_DISABLE
#define MB_MODBUS_CLIENT_CALL(func, ...) \
    ModbusClientPrivate *d = d_cast(d_ptr); \
    return d->port->func(this, d->unit, __VA_ARGS__);
#else
#define MB_MODBUS_CLIENT_CALL(func, ...) \
    ModbusClientPrivate *d = d_cast(d_ptr); \
    Modbus::StatusCode s = d->port->func(this, d->unit, __VA_ARGS__); \
    if (Modbus::StatusIsProcessing(s)) \
        return s; \
    if (Modbus::StatusIsBad(s)) \
    { \
        ++d->innerTries; \
        if (d->innerTries < d->settings.tries) \
            return Modbus::Status_Processing; \
    } \
    d->innerTries = 0; \
    return s;
#endif // MB_CLIENT_REPEAT_DISABLE


#ifndef MBF_READ_COILS_DISABLE
StatusCode ModbusClient::readCoils(uint16_t offset, uint16_t count, void *values)
{
    MB_MODBUS_CLIENT_CALL(readCoils, offset, count, values)
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode ModbusClient::readDiscreteInputs(uint16_t offset, uint16_t count, void *values)
{
    MB_MODBUS_CLIENT_CALL(readDiscreteInputs, offset, count, values)
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode ModbusClient::readHoldingRegisters(uint16_t offset, uint16_t count, uint16_t *values)
{
    MB_MODBUS_CLIENT_CALL(readHoldingRegisters, offset, count, values)
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode ModbusClient::readInputRegisters(uint16_t offset, uint16_t count, uint16_t *values)
{
    MB_MODBUS_CLIENT_CALL(readInputRegisters, offset, count, values)
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode ModbusClient::writeSingleCoil(uint16_t offset, bool value)
{
    MB_MODBUS_CLIENT_CALL(writeSingleCoil, offset, value)
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode ModbusClient::writeSingleRegister(uint16_t offset, uint16_t value)
{
    MB_MODBUS_CLIENT_CALL(writeSingleRegister, offset, value)
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode ModbusClient::readExceptionStatus(uint8_t *value)
{
    MB_MODBUS_CLIENT_CALL(readExceptionStatus, value)
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
StatusCode ModbusClient::diagnostics(uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata)
{
    MB_MODBUS_CLIENT_CALL(diagnostics, subfunc, insize, indata, outsize, outdata)
}
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
StatusCode ModbusClient::getCommEventCounter(uint16_t *status, uint16_t *eventCount)
{
    MB_MODBUS_CLIENT_CALL(getCommEventCounter, status, eventCount)
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
StatusCode ModbusClient::getCommEventLog(uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    MB_MODBUS_CLIENT_CALL(getCommEventLog, status, eventCount, messageCount, eventBuffSize, eventBuff)
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode ModbusClient::writeMultipleCoils(uint16_t offset, uint16_t count, const void *values)
{
    MB_MODBUS_CLIENT_CALL(writeMultipleCoils, offset, count, values)
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClient::writeMultipleRegisters(uint16_t offset, uint16_t count, const uint16_t *values)
{
    MB_MODBUS_CLIENT_CALL(writeMultipleRegisters, offset, count, values)
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode ModbusClient::reportServerID(uint8_t *count, uint8_t *data)
{
    MB_MODBUS_CLIENT_CALL(reportServerID, count, data)
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode ModbusClient::maskWriteRegister(uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    MB_MODBUS_CLIENT_CALL(maskWriteRegister, offset, andMask, orMask)
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClient::readWriteMultipleRegisters(uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    MB_MODBUS_CLIENT_CALL(readWriteMultipleRegisters, readOffset, readCount, readValues, writeOffset, writeCount, writeValues)
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
StatusCode ModbusClient::readFIFOQueue(uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    MB_MODBUS_CLIENT_CALL(readFIFOQueue, fifoadr, count, values)
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
StatusCode ModbusClient::readCoilsAsBoolArray(uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPrivate *d = d_cast(d_ptr);
    return d->port->readCoilsAsBoolArray(this, d->unit, offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode ModbusClient::readDiscreteInputsAsBoolArray(uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPrivate *d = d_cast(d_ptr);
    return d->port->readDiscreteInputsAsBoolArray(this, d->unit, offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode ModbusClient::writeMultipleCoilsAsBoolArray(uint16_t offset, uint16_t count, const bool *values)
{
    ModbusClientPrivate *d = d_cast(d_ptr);
    return d->port->writeMultipleCoilsAsBoolArray(this, d->unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

StatusCode ModbusClient::lastPortStatus() const
{
    return d_cast(d_ptr)->port->lastStatus();
}

StatusCode ModbusClient::lastPortErrorStatus() const
{
    return d_cast(d_ptr)->port->lastErrorStatus();
}

const Char *ModbusClient::lastPortErrorText() const
{
    return d_cast(d_ptr)->port->lastErrorText();
}
