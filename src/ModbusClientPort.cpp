#include "ModbusClientPort.h"
#include "ModbusClientPort_p.h"

#include "ModbusPort.h"

inline ModbusClientPortPrivate *d_cast(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusClientPortPrivate*>(d_ptr); }

ModbusClientPort::ModbusClientPort(ModbusPort *port) :
    ModbusObject(new ModbusClientPortPrivate(port))
{
}

ProtocolType ModbusClientPort::type() const
{
    return d_cast(d_ptr)->port->type();
}

StatusCode ModbusClientPort::close()
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    StatusCode s = d->port->close();
    signalClosed(this->objectName());
    d->currentClient = nullptr;
    d->setPortStatus(s);
    return s;
}

bool ModbusClientPort::isOpen() const
{
    return d_cast(d_ptr)->port->isOpen();
}

uint32_t ModbusClientPort::tries() const
{
    return d_cast(d_ptr)->settings.tries;
}

void ModbusClientPort::setTries(uint32_t v)
{
    if (v > 0)
        d_cast(d_ptr)->settings.tries = v;
}

bool ModbusClientPort::isBroadcastEnabled() const
{
    return d_cast(d_ptr)->isBroadcastEnabled();
}

void ModbusClientPort::setBroadcastEnabled(bool enable)
{
    d_cast(d_ptr)->setBroadcastEnabled(enable);
}

#ifndef MBF_READ_COILS_DISABLE
StatusCode ModbusClientPort::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return readCoils(this, unit, offset, count, values);
}

Modbus::StatusCode ModbusClientPort::readCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff,  fcBytes;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC01. Requested count of coils %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_DISCRETS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Start coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Start coil offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];     // Quantity of coils - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];     // Quantity of coils - LS BYTE
        d->count = count;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_READ_COILS,   // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (!szOutBuff)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC01. No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC01. Incorrect received data size"));
        if (fcBytes != ((d->count + 7) / 8))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC01. 'ByteCount' is not match received one"));
        memcpy(values, &buff[1], fcBytes);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClientPort::readCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    Modbus::StatusCode r = readCoils(client, unit, offset, count, d->buff);
    if (!StatusIsGood(r) || d->isBroadcast())
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (d->buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode ModbusClientPort::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return readDiscreteInputs(this, unit, offset, count, values);
}

Modbus::StatusCode ModbusClientPort::readDiscreteInputs(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcBytes;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC02. Requested count of inputs %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_DISCRETS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];   // Start input offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];   // Start input offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];    // Quantity of inputs - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];    // Quantity of inputs - LS BYTE
        d->count = count;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                     // unit ID
                          MBF_READ_DISCRETE_INPUTS, // modbus function number
                          buff,                     // in buffer
                          4,                        // count of input data bytes
                          buff,                     // out buffer
                          szBuff,                   // maximum size of buffer
                          &szOutBuff);              // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (!szOutBuff)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC02. No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC02. Incorrect received data size"));
        if (fcBytes != ((d->count + 7) / 8))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC02. 'ByteCount' is not match received one"));
        memcpy(values, &buff[1], fcBytes);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClientPort::readDiscreteInputsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    Modbus::StatusCode r = readDiscreteInputs(client, unit, offset, count, d->buff);
    if (!StatusIsGood(r) || d->isBroadcast())
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (d->buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode ModbusClientPort::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return readHoldingRegisters(this, unit, offset, count, values);
}

Modbus::StatusCode ModbusClientPort::readHoldingRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcRegs, fcBytes, i;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC03. Requested count of registers %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_REGISTERS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        d->count = count;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                         // unit ID
                          MBF_READ_HOLDING_REGISTERS,   // modbus function number
                          buff,                         // in buffer
                          4,                            // count of input data bytes
                          buff,                         // out buffer
                          szBuff,                       // maximum size of buffer
                          &szOutBuff);                  // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (!szOutBuff)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC03. No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC03. Incorrect received data size"));
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != d->count)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC03. Count of registers is not match received one"));
        for (i = 0; i < fcRegs; i++)
            values[i] = (buff[i*2+1] << 8) | buff[i*2+2];
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode ModbusClientPort::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return readInputRegisters(this, unit, offset, count, values);
}

Modbus::StatusCode ModbusClientPort::readInputRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcRegs, fcBytes, i;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC04. Requested count of registers %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_REGISTERS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        d->count = count;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                     // unit ID
                          MBF_READ_INPUT_REGISTERS, // modbus function number
                          buff,                     // in buffer
                          4,                        // count of input data bytes
                          buff,                     // out buffer
                          szBuff,                   // maximum size of buffer
                          &szOutBuff);              // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast()) // processing
            RAISE_COMPLETED(r);
        if (!szOutBuff)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC04. No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC04. Incorrect received data size"));
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != d->count)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC04. Count of registers is not match received one"));
        for (i = 0; i < fcRegs; i++)
            values[i] = (buff[i*2+1] << 8) | buff[i*2+2];
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode ModbusClientPort::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
{
    return writeSingleCoil(this, unit, offset, value);
}

Modbus::StatusCode ModbusClientPort::writeSingleCoil(ModbusObject *client, uint8_t unit, uint16_t offset, bool value)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];   // Coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];   // Coil offset - LS BYTE
        buff[2] = (value ? 0xFF : 0x00);                    // Value - 0xFF if true, 0x00 if false
        buff[3] = 0x00;                                     // Value - must always be NULL
        d->offset = offset;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                           // unit ID
                          MBF_WRITE_SINGLE_COIL,          // modbus function number
                          buff,                           // in buffer
                          4,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC05. Incorrect received data size"));

        outOffset = buff[1] | (buff[0] << 8);
        if (outOffset != d->offset)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC05. Requested offset is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode ModbusClientPort::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
{
    return writeSingleRegister(this, unit, offset, value);
}

Modbus::StatusCode ModbusClientPort::writeSingleRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t value)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, outValue;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&value)[1];     // Value - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&value)[0];     // Value - LS BYTE
        d->offset = offset;
        d->value = value;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                           // unit ID
                          MBF_WRITE_SINGLE_REGISTER,      // modbus function number
                          buff,                           // in buffer
                          4,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC06. Incorrect received data size"));

        outOffset = buff[1] | (buff[0] << 8);
        if (outOffset != d->offset)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC06. Requested offset is not match received one"));
        outValue = buff[3] | (buff[2] << 8);
        if (outValue != d->value)   
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC06. Requested value is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode ModbusClientPort::readExceptionStatus(uint8_t unit, uint8_t *value)
{
    return readExceptionStatus(this, unit, value);
}

StatusCode ModbusClientPort::readExceptionStatus(ModbusObject *client, uint8_t unit, uint8_t *value)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 1;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                      // unit ID
                          MBF_READ_EXCEPTION_STATUS, // modbus function number
                          buff,                      // in buffer
                          0,                         // count of input data bytes
                          buff,                      // out buffer
                          szBuff,                    // maximum size of buffer
                          &szOutBuff);               // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 1)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC07. Incorrect received data size"));
        *value = buff[0];
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnQueryData(uint8_t unit, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata)
{
    return diagnosticsReturnQueryData(this, unit, insize, indata, outsize, outdata);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnQueryData(ModbusObject *client, uint8_t unit, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 255;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;
    uint8_t sz;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (insize > (szBuff - 2))
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC08. ReturnQueryData. Input data size %u is too large (max=%u)"), insize, (uint16_t)(szBuff - 2));
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        d->subfunc = MBF_DIAGNOSTICS_RETURN_QUERY_DATA;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        memcpy(&buff[2], indata, insize);
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          insize+2,         // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff < 2)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnQueryData. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnQueryData. 'Subfunc' is not match received one"));
        sz = static_cast<uint8_t>(szOutBuff-2);
        if (sz > insize)
            sz = insize;
        memcpy(outdata, &buff[2], sz);
        if (outsize)
            *outsize = sz;
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
StatusCode ModbusClientPort::diagnosticsRestartCommunicationsOption(uint8_t unit, bool clearEventLog)
{
    return diagnosticsRestartCommunicationsOption(this, unit, clearEventLog);
}

Modbus::StatusCode ModbusClientPort::diagnosticsRestartCommunicationsOption(ModbusObject *client, uint8_t unit, bool clearEventLog)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = (clearEventLog ? 0xFF : 0x00);              // Restart Option - 0xFF if true, 0x00 if false
        buff[3] = 0x00;                                       // Restart Option - must always be 0x00
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. RestartCommunicationsOption. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. RestartCommunicationsOption. 'Subfunc' is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }

}
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnDiagnosticRegister(uint8_t unit, uint16_t *value)
{
    return diagnosticsReturnDiagnosticRegister(this, unit, value);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnDiagnosticRegister(ModbusObject *client, uint8_t unit, uint16_t *value)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnDiagnosticRegister. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnDiagnosticRegister. 'Subfunc' is not match received one"));

        *value = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
StatusCode ModbusClientPort::diagnosticsChangeAsciiInputDelimiter(uint8_t unit, char delimiter)
{
    return diagnosticsChangeAsciiInputDelimiter(this, unit, delimiter);
}

Modbus::StatusCode ModbusClientPort::diagnosticsChangeAsciiInputDelimiter(ModbusObject *client, uint8_t unit, char delimiter)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = static_cast<uint8_t>(delimiter);            // Delimiter
        buff[3] = 0;                                          // Delimiter - must always be 0x00
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ChangeAsciiInputDelimiter. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ChangeAsciiInputDelimiter. 'Subfunc' is not match received one"));

        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
StatusCode ModbusClientPort::diagnosticsForceListenOnlyMode(uint8_t unit)
{
    return diagnosticsForceListenOnlyMode(this, unit);
}

Modbus::StatusCode ModbusClientPort::diagnosticsForceListenOnlyMode(ModbusObject *client, uint8_t unit)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ForceListenOnlyMode. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ForceListenOnlyMode. 'Subfunc' is not match received one"));

        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
StatusCode ModbusClientPort::diagnosticsClearCountersAndDiagnosticRegister(uint8_t unit)
{
    return diagnosticsClearCountersAndDiagnosticRegister(this, unit);
}

Modbus::StatusCode ModbusClientPort::diagnosticsClearCountersAndDiagnosticRegister(ModbusObject *client, uint8_t unit)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc, outValue;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ClearCountersAndDiagnosticRegister. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ClearCountersAndDiagnosticRegister. 'Subfunc' is not match received one"));

        outValue = buff[3] | (buff[2] << 8);
        if (outValue != 0x0000)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ClearCountersAndDiagnosticRegister. 'Data' is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnBusMessageCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnBusMessageCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnBusMessageCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusMessageCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusMessageCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnBusCommunicationErrorCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnBusCommunicationErrorCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnBusCommunicationErrorCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusCommunicationErrorCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusCommunicationErrorCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnBusExceptionErrorCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnBusExceptionErrorCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnBusExceptionErrorCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusExceptionErrorCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusExceptionErrorCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnServerMessageCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnServerMessageCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnServerMessageCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerMessageCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerMessageCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnServerNoResponseCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnServerNoResponseCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnServerNoResponseCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerNoResponseCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerNoResponseCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnServerNAKCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnServerNAKCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnServerNAKCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerNAKCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerNAKCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnServerBusyCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnServerBusyCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnServerBusyCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerBusyCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnServerBusyCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_CHARACTER_OVERRUN_COUNT_DISABLE
StatusCode ModbusClientPort::diagnosticsReturnBusCharacterOverrunCount(uint8_t unit, uint16_t *count)
{
    return diagnosticsReturnBusCharacterOverrunCount(this, unit, count);
}

Modbus::StatusCode ModbusClientPort::diagnosticsReturnBusCharacterOverrunCount(ModbusObject *client, uint8_t unit, uint16_t *count)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_RETURN_BUS_CHARACTER_OVERRUN_COUNT;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusCharacterOverrunCount. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ReturnBusCharacterOverrunCount. 'Subfunc' is not match received one"));

        *count = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_RETURN_BUS_CHARACTER_OVERRUN_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
StatusCode ModbusClientPort::diagnosticsClearOverrunCounterAndFlag(uint8_t unit)
{
    return diagnosticsClearOverrunCounterAndFlag(this, unit);
}

Modbus::StatusCode ModbusClientPort::diagnosticsClearOverrunCounterAndFlag(ModbusObject *client, uint8_t unit)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc, outValue;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        d->subfunc = MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG;
        buff[0] = reinterpret_cast<uint8_t*>(&d->subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&d->subfunc)[0]; // Sub function - LS BYTE
        buff[2] = 0x00;
        buff[3] = 0x00;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ClearOverrunCounterAndFlag. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ClearOverrunCounterAndFlag. 'Subfunc' is not match received one"));

        outValue = buff[3] | (buff[2] << 8);
        if (outValue != 0x0000)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. ClearOverrunCounterAndFlag. 'Data' is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE

#endif // MBF_DIAGNOSTICS_DISABLE


#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
Modbus::StatusCode ModbusClientPort::getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    return getCommEventCounter(this, unit, status, eventCount);
}

Modbus::StatusCode ModbusClientPort::getCommEventCounter(ModbusObject *client, uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ModbusClientPort::RequestStatus rstatus = this->getRequestStatus(client);
    switch (rstatus)
    {
    case ModbusClientPort::Enable:
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                       // unit ID
                          MBF_GET_COMM_EVENT_COUNTER, // modbus function number
                          buff,                       // in buffer
                          0,                          // count of input data bytes
                          buff,                       // out buffer
                          szBuff,                     // maximum size of buffer
                          &szOutBuff);                // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC11. Incorrect received data size"));
        *status = buff[1] | (buff[0] << 8);
        *eventCount = buff[3] | (buff[2] << 8);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
Modbus::StatusCode ModbusClientPort::getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    return getCommEventLog(this, unit, status, eventCount, messageCount, eventBuffSize, eventBuff);
}

Modbus::StatusCode ModbusClientPort::getCommEventLog(ModbusObject *client, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff], byteCount;
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ModbusClientPort::RequestStatus rstatus = this->getRequestStatus(client);
    switch (rstatus)
    {
    case ModbusClientPort::Enable:
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                   // unit ID
                          MBF_GET_COMM_EVENT_LOG, // modbus function number
                          buff,                   // in buffer
                          0,                      // count of input data bytes
                          buff,                   // out buffer
                          szBuff,                 // maximum size of buffer
                          &szOutBuff);            // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff < 7)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC12. Incorrect received data size"));
        byteCount = buff[0];
        if (szOutBuff != (byteCount+1))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC12. 'ByteCount' doesn't match with received data size"));
        *status       = buff[2] | (buff[1] << 8);
        *eventCount   = buff[4] | (buff[3] << 8);
        *messageCount = buff[6] | (buff[5] << 8);

        byteCount = byteCount-6;
        if (byteCount > MB_GET_COMM_EVENT_LOG_MAX)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC12. 'EventCount' is bigger than 64"));
        *eventBuffSize = byteCount;
        memcpy(eventBuff, &buff[7], byteCount);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode ModbusClientPort::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    return writeMultipleCoils(this, unit, offset, count, values);
}

Modbus::StatusCode ModbusClientPort::writeMultipleCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const int szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, outCount, fcBytes;


    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC15. Requested count of coils %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_DISCRETS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start coil offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of coils - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of coils - LS BYTE
        fcBytes = (count + 7) / 8;
        buff[4] = static_cast<uint8_t>(fcBytes);      // Quantity of next bytes
        memcpy(&buff[5], values, fcBytes);
        d->offset = offset;
        d->count = count;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                     // unit ID
                          MBF_WRITE_MULTIPLE_COILS, // modbus function number
                          buff,                     // in buffer
                          5 + buff[4],              // count of input data bytes
                          buff,                     // out buffer
                          szBuff,                   // maximum size of buffer
                          &szOutBuff);              // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC15. Incorrect received data size"));
        outOffset = (buff[0] << 8) | buff[1];
        if (outOffset != d->offset)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC15. Requested offset is not match received one"));
        outCount = (buff[2] << 8) | buff[3];
        if (outCount != d->count)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC15. Requested count is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClientPort::writeMultipleCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const bool *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    if (this->currentClient() == nullptr)
    {
        for (int i = 0; i < count; i++)
        {
            if (!(i & 7))
                d->buff[i / 8] = 0;
            if (values[i] != 0)
                d->buff[i / 8] |= (1 << (i % 8));
        }
        return writeMultipleCoils(client, unit, offset, count, d->buff);
    }
    else if (this->currentClient() == client)
        return writeMultipleCoils(client, unit, offset, count, d->buff);
    return Status_Processing;
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    return writeMultipleRegisters(this, unit, offset, count, values);
}

Modbus::StatusCode ModbusClientPort::writeMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, i, outOffset, outCount;


    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC16. Requested count of registers %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_REGISTERS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];   // start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];   // start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];    // quantity of registers - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];    // quantity of registers - LS BYTE
        buff[4] = static_cast<uint8_t>(count * 2);          // quantity of next bytes

        for (i = 0; i < count; i++)
        {
            buff[5 + i * 2] = reinterpret_cast<const uint8_t*>(&values[i])[1];
            buff[6 + i * 2] = reinterpret_cast<const uint8_t*>(&values[i])[0];
        }
        d->offset = offset;
        d->count = count;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                         // unit ID
                          MBF_WRITE_MULTIPLE_REGISTERS, // modbus function number
                          buff,                         // in buffer
                          5 + buff[4],                  // count of input data bytes
                          buff,                         // out buffer
                          szBuff,                       // maximum size of buffer
                          &szOutBuff);                  // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (szOutBuff != 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC16. Incorrect received data size"));
        outOffset = (buff[0] << 8) | buff[1];
        if (outOffset != d->offset)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC16. Requested offset is not match received one"));
        outCount = (buff[2] << 8) | buff[3];
        if (outCount != d->count)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC16. Requested count is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode ModbusClientPort::reportServerID(uint8_t unit, uint8_t *count, uint8_t *data)
{
    return reportServerID(this, unit, count, data);
}

Modbus::StatusCode ModbusClientPort::reportServerID(ModbusObject *client, uint8_t unit, uint8_t *count, uint8_t *data)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    uint8_t byteCount;
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ModbusClientPort::RequestStatus rstatus = this->getRequestStatus(client);
    switch (rstatus)
    {
    case ModbusClientPort::Enable:
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                 // unit ID
                          MBF_REPORT_SERVER_ID, // modbus function number
                          buff,                 // in buffer
                          0,                    // count of input data bytes
                          buff,                 // out buffer
                          szBuff,               // maximum size of buffer
                          &szOutBuff);          // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff == 0)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC17. Incorrect received data size"));
        byteCount = buff[0];
        if (szOutBuff != (byteCount+1))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC17. 'ByteCount' parameter doesn't match actual data size"));
        *count = byteCount;
        memcpy(data, &buff[1], byteCount);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_READ_FILE_RECORD_DISABLE
StatusCode ModbusClientPort::readFileRecord(uint8_t unit, const FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize)
{
    return readFileRecord(this, unit, records, recordsCount, outData, outSize);
}

StatusCode ModbusClientPort::readFileRecord(ModbusObject *client, uint8_t unit, const FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;
    uint8_t byteCount = 0;
    int i, opos;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (recordsCount > MB_FILE_RECORD_MAX) // floor(255 / 7)
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC20. Requested records count %u is too large (max=%u)"), recordsCount, MB_FILE_RECORD_MAX);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }

        byteCount = static_cast<uint8_t>(recordsCount * 7);
        buff[0] = byteCount;
        for (i = 0; i < recordsCount; ++i)
        {
            if (records[i].recordLength > MB_FILE_RECORD_BUFF_SZ)
            {
                const size_t len = 100;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC20. Requested record buffer len is too large (max=%u)"), MB_FILE_RECORD_BUFF_SZ);
                RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
            }
            buff[1 + i * 7] = 0x06; // Reference Type
            buff[2 + i * 7] = reinterpret_cast<const uint8_t*>(&records[i].fileNumber)[1];
            buff[3 + i * 7] = reinterpret_cast<const uint8_t*>(&records[i].fileNumber)[0];
            buff[4 + i * 7] = reinterpret_cast<const uint8_t*>(&records[i].recordNumber)[1];
            buff[5 + i * 7] = reinterpret_cast<const uint8_t*>(&records[i].recordNumber)[0];
            buff[6 + i * 7] = reinterpret_cast<const uint8_t*>(&records[i].recordLength)[1];
            buff[7 + i * 7] = reinterpret_cast<const uint8_t*>(&records[i].recordLength)[0];
        }
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                 // unit ID
                          MBF_READ_FILE_RECORD, // modbus function number
                          buff,                 // in buffer
                          static_cast<uint16_t>(buff[0] + 1), // count of input data bytes
                          buff,                 // out buffer
                          szBuff,               // maximum size of buffer
                          &szOutBuff);          // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff == 0)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC20. Incorrect received data size"));

        byteCount = buff[0];
        if (szOutBuff != (static_cast<uint16_t>(byteCount) + 1))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC20. 'ByteCount' parameter doesn't match actual data size"));
        
        opos = 0;
        for (i = 1; i < byteCount; )
        {
            uint8_t fileRespLength = buff[i];
            if ((fileRespLength < 1) || (fileRespLength > MB_FILE_RECORD_BUFF_SZ))
            {
                const size_t len = 120;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC20. Incorrect record data length: %hhu"), fileRespLength);
                RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, errbuff);
            }

            uint8_t refType = buff[i+1];
            if (refType != 0x06)
            {
                const size_t len = 150;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC20. Incorrect Reference Type %u in record %u (expected 0x06)"), refType, (i-1)/7);
                RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, errbuff);
            }

            const uint8_t cRegs = (fileRespLength-1)/2;
            for (uint8_t j = 0; j < cRegs; ++j)
            {
                reinterpret_cast<uint8_t*>(outData)[opos  ] = buff[i + 3 + j*2];
                reinterpret_cast<uint8_t*>(outData)[opos+1] = buff[i + 2 + j*2];
                opos += 2;
            }
            i += fileRespLength+1;
        }
        
        if (outSize)
            *outSize = opos;
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
StatusCode ModbusClientPort::writeFileRecord(uint8_t unit, const FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize)
{
    return writeFileRecord(this, unit, records, recordsCount, inData, inSize);
}

StatusCode ModbusClientPort::writeFileRecord(ModbusObject *client, uint8_t unit, const FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;
    int c, ipos;
    uint16_t pos;
    uint8_t i;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (recordsCount > MB_FILE_RECORD_MAX)
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC21. Requested records count %u is too large (max=%u)"), recordsCount, MB_FILE_RECORD_MAX);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        pos = 1;
        c = 0;
        ipos = 0;
        for (i = 0; i < recordsCount; ++i)
        {
            uint16_t len = records[i].recordLength;
            uint16_t bufflen = 7+len*2;
            if ((c+bufflen) > MB_FILE_RECORD_BUFF_SZ)
            {
                const size_t len = 100;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC21. Requested record buffer len is too large (max=%u)"), MB_FILE_RECORD_BUFF_SZ);
                RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
            }
            buff[pos++] = 0x06; // Reference Type
            buff[pos++] = reinterpret_cast<const uint8_t*>(&records[i].fileNumber)[1];
            buff[pos++] = reinterpret_cast<const uint8_t*>(&records[i].fileNumber)[0];
            buff[pos++] = reinterpret_cast<const uint8_t*>(&records[i].recordNumber)[1];
            buff[pos++] = reinterpret_cast<const uint8_t*>(&records[i].recordNumber)[0];
            buff[pos++] = reinterpret_cast<const uint8_t*>(&records[i].recordLength)[1];
            buff[pos++] = reinterpret_cast<const uint8_t*>(&records[i].recordLength)[0];
            for (uint16_t j = 0; j < len; ++j, ipos += 2)
            {
                buff[pos++] = reinterpret_cast<const uint8_t*>(inData)[ipos+1];
                buff[pos++] = reinterpret_cast<const uint8_t*>(inData)[ipos  ];
            }
            c += bufflen;
        }
        buff[0] = static_cast<uint8_t>(c);
        if (inSize)
            *inSize = ipos;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                  // unit ID
                          MBF_WRITE_FILE_RECORD, // modbus function number
                          buff,                  // in buffer
                          static_cast<uint16_t>(buff[0] + 1), // count of input data bytes
                          buff,                  // out buffer
                          szBuff,                // maximum size of buffer
                          &szOutBuff);           // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff == 0)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC21. Incorrect received data size"));

        if (szOutBuff != static_cast<uint16_t>(buff[0] + 1))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC21. 'ByteCount' parameter doesn't match actual data size"));

        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_FILE_RECORD_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode ModbusClientPort::maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    return maskWriteRegister(this, unit, offset, andMask, orMask);
}

StatusCode ModbusClientPort::maskWriteRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 6;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, outAndMask, outOrMask;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&andMask)[1];   // AndMask - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&andMask)[0];   // AndMask - LS BYTE
        buff[4] = reinterpret_cast<uint8_t*>(&orMask)[1];    // OrMask - MS BYTE
        buff[5] = reinterpret_cast<uint8_t*>(&orMask)[0];    // OrMask - LS BYTE
        d->offset = offset;
        d->andMask = andMask;
        d->orMask = orMask;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                           // unit ID
                          MBF_MASK_WRITE_REGISTER,        // modbus function number
                          buff,                           // in buffer
                          6,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff != 6)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC22. Incorrect received data size"));

        outOffset  = buff[1] | (buff[0] << 8);
        outAndMask = buff[3] | (buff[2] << 8);
        outOrMask  = buff[5] | (buff[4] << 8);
        if (outOffset != d->offset)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC22. Requested offset is not match received one"));
        if (outAndMask != d->andMask)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC22. Requested 'AndMask' is not match received one"));
        if (outOrMask != d->orMask)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC22. Requested 'OrMask' is not match received one"));
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClientPort::readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    return readWriteMultipleRegisters(this, unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
}

StatusCode ModbusClientPort::readWriteMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, i, fcBytes, fcRegs;


    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if ((readCount > MB_MAX_REGISTERS) || (writeCount > MB_MAX_REGISTERS))
        {
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC23. Requested count of registers (read=%hu, write=%hu) is too large (max=%hu)"), readCount, writeCount, (uint16_t)MB_MAX_REGISTERS);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&readOffset)[1];   // read starting offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&readOffset)[0];   // read starting offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&readCount)[1];    // quantity to read - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&readCount)[0];    // quantity to read - LS BYTE
        buff[4] = reinterpret_cast<uint8_t*>(&writeOffset)[1];  // write starting offset - MS BYTE
        buff[5] = reinterpret_cast<uint8_t*>(&writeOffset)[0];  // write starting offset - LS BYTE
        buff[6] = reinterpret_cast<uint8_t*>(&writeCount)[1];   // quantity to write - MS BYTE
        buff[7] = reinterpret_cast<uint8_t*>(&writeCount)[0];   // quantity to write - LS BYTE
        buff[8] = static_cast<uint8_t>(writeCount * 2);         // quantity of next bytes

        for (i = 0; i < writeCount; i++)
        {
            buff[ 9 + i * 2] = reinterpret_cast<const uint8_t*>(&writeValues[i])[1];
            buff[10 + i * 2] = reinterpret_cast<const uint8_t*>(&writeValues[i])[0];
        }
        d->count = readCount;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                             // unit ID
                          MBF_READ_WRITE_MULTIPLE_REGISTERS,// modbus function number
                          buff,                             // in buffer
                          9 + buff[8],                      // count of input data bytes
                          buff,                             // out buffer
                          szBuff,                           // maximum size of buffer
                          &szOutBuff);                      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (!szOutBuff)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC23. No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC23. Incorrect received data size"));
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != d->count)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC23. Count registers to read is not match received one"));
        for (i = 0; i < fcRegs; i++)
            readValues[i] = (buff[i * 2 + 1] << 8) | buff[i * 2 + 2];
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
Modbus::StatusCode ModbusClientPort::readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    return readFIFOQueue(this, unit, fifoadr, count, values);
}

Modbus::StatusCode ModbusClientPort::readFIFOQueue(ModbusObject *client, uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, bytesCount, FIFOCount, i;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&fifoadr)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&fifoadr)[0]; // Start register offset - LS BYTE
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                // unit ID
                          MBF_READ_FIFO_QUEUE, // modbus function number
                          buff,                // in buffer
                          2,                   // count of input data bytes
                          buff,                // out buffer
                          szBuff,              // maximum size of buffer
                          &szOutBuff);         // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        if (szOutBuff < 4)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC24. Incorrect received data size"));
        bytesCount = buff[1] | (buff[0] << 8);
        if (bytesCount != (szOutBuff - 2))
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC24. 'ByteCount' parameter doesn't match actual data size"));
        FIFOCount  = buff[3] | (buff[2] << 8);
        if (bytesCount != (FIFOCount + 1) * 2)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC24. 'ByteCount' parameter doesn't match 'FIFOCount'"));
        if (FIFOCount > MB_READ_FIFO_QUEUE_MAX)
            RAISE_ERROR_COMPLETED(Status_BadIllegalDataValue, StringLiteral("FC24. 'FIFOCount' is bigger than 31"));
        for (i = 0; i < FIFOCount; i++)
            values[i] = buff[i*2+5] | (buff[i*2+4] << 8);
        *count = FIFOCount;
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
Modbus::StatusCode ModbusClientPort::readDeviceIdentification(uint8_t unit, uint8_t readDevId, uint8_t objectId, uint8_t *dataSize, void *data, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)
{
    return readDeviceIdentification(this, unit, readDevId, objectId, dataSize, data, numberOfObjects, conformityLevel, moreFollows, nextObjectId);
}

Modbus::StatusCode ModbusClientPort::readDeviceIdentification(ModbusObject *client, uint8_t unit, uint8_t readDeviceId, uint8_t objectId, uint8_t *dataSize, void *data, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;
    uint8_t sz;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        // Validate Read Device ID code (1=Basic, 2=Regular, 3=Extended, 4=Specific)
        if (readDeviceId < MB_MEI_READ_DEVICE_ID_BASIC || readDeviceId > MB_MEI_READ_DEVICE_ID_SPECIFIC)
        {
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, StringLiteral("FC43. Invalid Read Device ID code"));
        }
        // Pack 3-byte MEI request: [MEI Type, Read Dev ID Code, Object ID]
        buff[0] = MBF_MEI_READ_DEVICE_ID;  // MEI Type = 0x0E (Read Device Identification)
        buff[1] = readDeviceId;                // Read Device ID code
        buff[2] = objectId;                    // Object ID to start reading from
        d->readDeviceIdCode = readDeviceId;
        d->readDeviceIdObjectId = objectId;
        MB_FALLTHROUGH
    case ModbusClientPort::Process:
        r = this->request(unit,                                 // unit ID
                          MBF_ENCAPSULATED_INTERFACE_TRANSPORT, // modbus function number (0x2B)
                          buff,                                 // in buffer
                          3,                                    // count of input data bytes
                          buff,                                 // out buffer
                          szBuff,                               // maximum size of buffer
                          &szOutBuff);                          // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        // Minimum valid response: MEI Type(1) + Read Dev ID(1) + Conformity(1) +
        //                         More Follows(1) + Next Object ID(1) + Num Objects(1) = 6 bytes
        if (szOutBuff < 6)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC43. Incorrect received data size"));
        // Verify MEI type in response matches our request
        if (buff[0] != MBF_MEI_READ_DEVICE_ID)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC43. MEI Type mismatch in response"));
        // Verify Read Device ID code in response matches our request
        if (buff[1] != d->readDeviceIdCode)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC43. Read Device ID code mismatch in response"));
        if (conformityLevel)
            *conformityLevel = buff[2]; // Conformity Level
        if (moreFollows)
            *moreFollows = buff[3] != 0; // More Follows flag
        if (nextObjectId)
            *nextObjectId = buff[4]; // Next Object ID
        if (numberOfObjects)
            *numberOfObjects = buff[5]; // Number of objects returned
        sz = static_cast<uint8_t>(szOutBuff-6); // Size of data returned (excluding header)
        *dataSize = sz;
        memcpy(data, &buff[6], sz); // Object values
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE

#endif // MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

ModbusPort *ModbusClientPort::port() const
{
    return d_cast(d_ptr)->port;
}

void ModbusClientPort::setPort(ModbusPort *port)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    if (port != d->port)
    {
        ModbusPort *old = d->port;
        old->close();
        d->currentClient = nullptr;
        d->state = STATE_UNKNOWN;
        d->port = port;
        delete old;
    }
}

StatusCode ModbusClientPort::lastStatus() const
{
    return d_cast(d_ptr)->lastStatus;
}

Modbus::Timestamp ModbusClientPort::lastStatusTimestamp() const
{
    return d_cast(d_ptr)->lastStatusTimestamp;
}

Modbus::StatusCode ModbusClientPort::lastErrorStatus() const
{
    return d_cast(d_ptr)->lastErrorStatus;
}

const Char *ModbusClientPort::lastErrorText() const
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    if (d->isLastPortError)
        return d->port->lastErrorText();
    else
        return d->lastErrorText.data();
}

uint32_t ModbusClientPort::lastTries() const
{
    return d_cast(d_ptr)->lastTries;
}

const ModbusObject *ModbusClientPort::currentClient() const
{
    return d_cast(d_ptr)->currentClient;
}

ModbusClientPort::RequestStatus ModbusClientPort::getRequestStatus(ModbusObject *client)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    if (d->currentClient)
    {
        if (d->currentClient == client)
            return Process;
        return Disable;
    }
    else
    {
        d->currentClient = client;
        return Enable;
    }
}

void ModbusClientPort::cancelRequest(ModbusObject *client)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    if (d->currentClient == client)
        d->currentClient = nullptr;
}

void ModbusClientPort::signalOpened(const Modbus::Char *source)
{
    emitSignal(__func__, &ModbusClientPort::signalOpened, source);
}

void ModbusClientPort::signalClosed(const Modbus::Char *source)
{
    emitSignal(__func__, &ModbusClientPort::signalClosed, source);
}

void ModbusClientPort::signalTx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
{
    emitSignal(__func__, &ModbusClientPort::signalTx, source, buff, size);
}

void ModbusClientPort::signalRx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
{
    emitSignal(__func__, &ModbusClientPort::signalRx, source, buff, size);
}

void ModbusClientPort::signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text)
{
    emitSignal(__func__, &ModbusClientPort::signalError, source, status, text);
}

void ModbusClientPort::signalCompleted(const Modbus::Char *source, Modbus::StatusCode status)
{
    emitSignal(__func__, &ModbusClientPort::signalCompleted, source, status);
}

StatusCode ModbusClientPort::rawRequest(const void *inBuff, uint16_t szInBuff, void *outBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    RequestStatus rs = getRequestStatus(this);
    if (rs == Disable)
        return Status_Processing;
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    while (1)
    {
        if (!d->isWriteBufferBlocked())
        {
            // TODO: set `d->unit = 0` and find reason of the crash
            d->unit = 1; // Note: prevent broadcast false recognition
            d->func = 0;
            d->lastTries = 0;
            StatusCode s = d->port->writeRawBuffer(inBuff, szInBuff);
            if (StatusIsBad(s))
            {
                SET_PORT_ERROR(s);
                RAISE_COMPLETED(s);
            }
            d->blockWriteBuffer();
        }
        StatusCode r = process();
        if (StatusIsProcessing(r))
            return r;
        d->lastTries = ++d->repeats;
        if (StatusIsBad(r) && (d->repeats < d->settings.tries))
        {
            d->port->setNextRequestRepeated(true);
            if (d->port->isNonBlocking())
                return Status_Processing;
            continue;
        }
        d->freeWriteBuffer();
        d->repeats = 0;
        //d->currentClient = nullptr;
        if (StatusIsBad(r))
            RAISE_COMPLETED(r);
        if (!d->isBroadcast())
        {
            r = d->port->readRawBuffer(outBuff, maxSzBuff, szOutBuff);
            if (StatusIsBad(r))
                SET_PORT_ERROR(r);
        }
        RAISE_COMPLETED(r);
    }
}

StatusCode ModbusClientPort::request(uint8_t unit, uint8_t func, const uint8_t *inBuff, uint16_t szInBuff, uint8_t *outBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    while (1)
    {
        if (!d->isWriteBufferBlocked())
        {
            d->unit = unit;
            d->func = func;
            d->lastTries = 0;
            auto r = d->port->writeBuffer(unit, func, inBuff, szInBuff);
            if (StatusIsBad(r))
                RAISE_PORT_ERROR(r);
            d->blockWriteBuffer();

        }
        StatusCode r = process();
        if (StatusIsProcessing(r))
            return r;
        d->lastTries = ++d->repeats;
        if (StatusIsBad(r) && (d->repeats < d->settings.tries))
        {
            d->port->setNextRequestRepeated(true);
            if (d->port->isNonBlocking())
                return Status_Processing;
            continue;
        }
        d->freeWriteBuffer();
        d->repeats = 0;
        //d->currentClient = nullptr;
        if (StatusIsBad(r))
            return r;
        if (!d->isBroadcast())
        {
            r = d->port->readBuffer(unit, func, outBuff, maxSzBuff, szOutBuff);
            if (!StatusIsBad(r))
            {
                if (unit != d->unit)
                    RAISE_ERROR(Status_BadNotCorrectResponse, StringLiteral("Not correct response. Requested unit (unit) is not equal to responsed"));

                if ((func & MBF_EXCEPTION) == MBF_EXCEPTION)
                {
                    if (*szOutBuff > 0)
                    {
                        auto errcode = outBuff[0];
                        const size_t len = 62;
                        Char errbuff[len];
                        snprintf(errbuff, len, StringLiteral("Returned Modbus-exception with code 0x%hhX"), errcode);
                        r = static_cast<StatusCode>(Status_Bad | errcode);
                        RAISE_ERROR(r, errbuff);
                    }
                    else
                        RAISE_ERROR(Status_BadNotCorrectResponse, StringLiteral("Returned Modbus-exception but code missed"));
                }

                if (func != d->func)
                    RAISE_ERROR(Status_BadNotCorrectResponse, StringLiteral("Not correct response. Requested function is not equal to responsed"));
                return r;
            }
            RAISE_PORT_ERROR(r);
        }
        return r;
    }
}

StatusCode ModbusClientPort::process()
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    StatusCode r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_UNKNOWN:
            if (d->port->isOpen())
            {
                d->state = STATE_OPENED;
                fRepeatAgain = true;
                break;
            }
            d->state = STATE_CLOSED;
            MB_FALLTHROUGH
        case STATE_CLOSED:
            d->state = STATE_BEGIN_OPEN;
            MB_FALLTHROUGH
        case STATE_BEGIN_OPEN:
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_OPEN;
            MB_FALLTHROUGH
        case STATE_WAIT_FOR_OPEN:
            r = d->port->open();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r)) // an error occured
            {
                SET_PORT_ERROR(r);
                d->state = STATE_TIMEOUT;
                return r;
            }
            d->state = STATE_OPENED;
            signalOpened(this->objectName());
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = close();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))
            {
                signalError(d->getName(), r, d->port->lastErrorText());
                return r;
            }
            d->state = STATE_CLOSED;
            // Note: Function can not return Status_Processing in Blocking mode
            //fRepeatAgain = d->port->isBlocking(); 
            break;
        case STATE_OPENED:
            if (d->port->isChanged())
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            // send data to server
            d->state = STATE_BEGIN_WRITE;
            MB_FALLTHROUGH
        case STATE_BEGIN_WRITE:
            d->timestampRefresh();
            if (!d->port->isOpen())
            {
                d->state = STATE_CLOSED;
                RAISE_ERROR(Status_BadPortClosed, StringLiteral("Error: Port is closed when trying to write data"));
            }
            d->state = STATE_WRITE;
            MB_FALLTHROUGH
        case STATE_WRITE:
            r = d->port->write();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r)) // an error occured
            {
                SET_PORT_ERROR(r);
                d->state = STATE_TIMEOUT;
                return r;
            }
            else
                signalTx(d->getName(), d->port->writeBufferData(), d->port->writeBufferSize());
            if (d->isBroadcast())
            {
                d->state = STATE_OPENED;
                return r;
            }
            d->state = STATE_BEGIN_READ;
            MB_FALLTHROUGH
        case STATE_BEGIN_READ:
            d->timestampRefresh();
            d->state = STATE_READ;
            MB_FALLTHROUGH
        case STATE_READ:
            r = d->port->read();
            if (StatusIsProcessing(r))
                return r;
            d->setPortStatus(r);
            if (StatusIsBad(r))
            {
                signalError(d->getName(), r, d->port->lastErrorText());
                d->state = STATE_TIMEOUT;
            }
            else
            {
                auto szRead = d->port->readBufferSize();
                if (szRead > 0)
                    signalRx(d->getName(), d->port->readBufferData(), szRead);
                if (d->port->isOpen())
                    d->state = STATE_OPENED;
                else
                {
                    d->state = STATE_CLOSED;
                    signalClosed(this->objectName());
                    //return Status_Uncertain;
                }
            }
            return r;
        case STATE_TIMEOUT:
        {
            uint32_t t = timer() - d->timestamp;
            if (t < d->port->timeout())
            {
                if (d->port->isBlocking())
                    msleep(d->port->timeout() - t);
                else
                    return Status_Processing;
            }
            d->state = STATE_UNKNOWN;
            fRepeatAgain = true;
        }
            break;
        default:
            if (d->port->isOpen())
                d->state = STATE_OPENED;
            else
                d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

