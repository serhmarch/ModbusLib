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
            const size_t len = 200;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC01. Requested count of coils %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_DISCRETS);
            this->cancelRequest(client);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Start coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Start coil offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];     // Quantity of coils - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];     // Quantity of coils - LS BYTE
        d->count = count;
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_READ_COILS,   // modbus function number
                          buff,             // in-out buffer
                          4,                // count of input data bytes
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
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
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
            this->cancelRequest(client);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];   // Start input offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];   // Start input offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];    // Quantity of inputs - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];    // Quantity of inputs - LS BYTE
        d->count = count;
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                     // unit ID
                          MBF_READ_DISCRETE_INPUTS, // modbus function number
                          buff,                     // in-out buffer
                          4,                        // count of input data bytes
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
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
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
            this->cancelRequest(client);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        d->count = count;
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                         // unit ID
                          MBF_READ_HOLDING_REGISTERS,   // modbus function number
                          buff,                         // in-out buffer
                          4,                            // count of input data bytes
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
            this->cancelRequest(client);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest, errbuff);
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        d->count = count;
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_READ_INPUT_REGISTERS,       // modbus function number
                          buff,                           // in-out buffer
                          4,                              // count of input data bytes
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_WRITE_SINGLE_COIL,          // modbus function number
                          buff,                           // in-out buffer
                          4,                              // count of input data bytes
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_WRITE_SINGLE_REGISTER,      // modbus function number
                          buff,                           // in-out buffer
                          4,                              // count of input data bytes
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                      // unit ID
                          MBF_READ_EXCEPTION_STATUS, // modbus function number
                          buff,                      // in-out buffer
                          0,                         // count of input data bytes
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
Modbus::StatusCode ModbusClientPort::diagnostics(ModbusObject *client, uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outSubfunc;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&subfunc)[1]; // Sub function - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&subfunc)[0]; // Sub function - LS BYTE
        memcpy(&buff[2], indata, insize);
        d->subfunc = subfunc;
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
                          MBF_DIAGNOSTICS,  // modbus function number
                          buff,             // in-out buffer
                          insize+2,         // count of input data bytes
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);

        if (szOutBuff < 2)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse, StringLiteral("FC08. Requested subfunc is not match received one"));
        *outsize = static_cast<uint8_t>(szOutBuff-2);
        memcpy(outdata, &buff[2], *outsize);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                       // unit ID
                          MBF_GET_COMM_EVENT_COUNTER, // modbus function number
                          buff,                       // in-out buffer
                          0,                          // count of input data bytes
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                   // unit ID
                          MBF_GET_COMM_EVENT_LOG, // modbus function number
                          buff,                   // in-out buffer
                          0,                      // count of input data bytes
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
        if (byteCount > GET_COMM_EVENT_LOG_MAX)
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
            snprintf(errbuff, len, StringLiteral("FC01. Requested count of coils %hu is too large (max=%hu)"), count, (uint16_t)MB_MAX_DISCRETS);
            this->cancelRequest(client);
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                     // unit ID
                          MBF_WRITE_MULTIPLE_COILS, // modbus function number
                          buff,                     // in-out buffer
                          5 + buff[4],              // count of input data bytes
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
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
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
            this->cancelRequest(client);
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                         // unit ID
                          MBF_WRITE_MULTIPLE_REGISTERS, // modbus function number
                          buff,                         // in-out buffer
                          5 + buff[4],                  // count of input data bytes
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                 // unit ID
                          MBF_REPORT_SERVER_ID, // modbus function number
                          buff,                 // in-out buffer
                          0,                    // count of input data bytes
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

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                           // unit ID
                          MBF_MASK_WRITE_REGISTER,        // modbus function number
                          buff,                           // in-out buffer
                          6,                              // count of input data bytes
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
            this->cancelRequest(client);
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                             // unit ID
                          MBF_READ_WRITE_MULTIPLE_REGISTERS,// modbus function number
                          buff,                             // in-out buffer
                          9 + buff[8],                      // count of input data bytes
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                // unit ID
                          MBF_READ_FIFO_QUEUE, // modbus function number
                          buff,                // in-out buffer
                          2,                   // count of input data bytes
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
Modbus::StatusCode ModbusClientPort::readDeviceIdentification(ModbusObject *client, uint8_t unit, uint8_t readDevId, uint8_t objectId, uint8_t *data, uint8_t *dataSize)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ModbusClientPort::RequestStatus status = this->getRequestStatus(client);
    switch (status)
    {
    case ModbusClientPort::Enable:
        // Validate Read Device ID code (1=Basic, 2=Regular, 3=Extended, 4=Specific)
        if (readDevId < MB_READ_DEVICE_ID_BASIC || readDevId > MB_READ_DEVICE_ID_SPECIFIC)
        {
            this->cancelRequest(client);
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectRequest,
                StringLiteral("FC43. Invalid Read Device ID code"));
        }
        // Pack 3-byte MEI request: [MEI Type, Read Dev ID Code, Object ID]
        buff[0] = MB_MEI_TYPE_READ_DEVICE_ID;  // MEI Type = 0x0E (Read Device Identification)
        buff[1] = readDevId;                    // Read Device ID code
        buff[2] = objectId;                     // Object ID to start reading from
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,                                    // unit ID
                          MBF_ENCAPSULATED_INTERFACE_TRANSPORT,    // modbus function number (0x2B)
                          buff,                                    // in-out buffer
                          3,                                       // count of input data bytes
                          szBuff,                                  // maximum size of buffer
                          &szOutBuff);                             // count of output data bytes
        if (StatusIsProcessing(r))
            return r;
        if (!StatusIsGood(r) || d->isBroadcast())
            RAISE_COMPLETED(r);
        // Minimum valid response: MEI Type(1) + Read Dev ID(1) + Conformity(1) +
        //                         More Follows(1) + Next Object ID(1) + Num Objects(1) = 6 bytes
        if (szOutBuff < 6)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse,
                StringLiteral("FC43. Incorrect received data size"));
        // Verify MEI type in response matches our request
        if (buff[0] != MB_MEI_TYPE_READ_DEVICE_ID)
            RAISE_ERROR_COMPLETED(Status_BadNotCorrectResponse,
                StringLiteral("FC43. MEI Type mismatch in response"));
        // Copy raw response to caller — caller is responsible for parsing TLV objects
        *dataSize = static_cast<uint8_t>(szOutBuff);
        memcpy(data, buff, szOutBuff);
        RAISE_COMPLETED(Modbus::Status_Good);
    default:
        return Status_Processing;
    }
}
#endif // MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE

#ifndef MBF_READ_COILS_DISABLE
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

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
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

#ifndef MBF_READ_COILS_DISABLE
StatusCode ModbusClientPort::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return readCoils(this, unit, offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode ModbusClientPort::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return readDiscreteInputs(this, unit, offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode ModbusClientPort::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return readHoldingRegisters(this, unit, offset, count, values);
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode ModbusClientPort::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return readInputRegisters(this, unit, offset, count, values);
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode ModbusClientPort::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
{
    return writeSingleCoil(this, unit, offset, value);
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode ModbusClientPort::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
{
    return writeSingleRegister(this, unit, offset, value);
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode ModbusClientPort::readExceptionStatus(uint8_t unit, uint8_t *value)
{
    return readExceptionStatus(this, unit, value);
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
Modbus::StatusCode ModbusClientPort::diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata)
{
    return diagnostics(this, unit, subfunc, insize, indata, outsize, outdata);
}
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
Modbus::StatusCode ModbusClientPort::getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    return getCommEventCounter(this, unit, status, eventCount);
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
Modbus::StatusCode ModbusClientPort::getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    return getCommEventLog(this, unit, status, eventCount, messageCount,eventBuffSize, eventBuff);
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode ModbusClientPort::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    return writeMultipleCoils(this, unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClientPort::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    return writeMultipleRegisters(this, unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
Modbus::StatusCode ModbusClientPort::reportServerID(uint8_t unit, uint8_t *count, uint8_t *data)
{
    return reportServerID(this, unit, count, data);
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode ModbusClientPort::maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    return maskWriteRegister(this, unit, offset, andMask, orMask);
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClientPort::readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    return readWriteMultipleRegisters(this, unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
Modbus::StatusCode ModbusClientPort::readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    return readFIFOQueue(this, unit, fifoadr, count, values);
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
Modbus::StatusCode ModbusClientPort::readDeviceIdentification(uint8_t unit, uint8_t readDevId, uint8_t objectId, uint8_t *data, uint8_t *dataSize)
{
    return readDeviceIdentification(this, unit, readDevId, objectId, data, dataSize);
}
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

StatusCode ModbusClientPort::request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    ModbusClientPortPrivate *d = d_cast(d_ptr);
    while (1)
    {
        if (!d->isWriteBufferBlocked())
        {
            d->unit = unit;
            d->func = func;
            d->lastTries = 0;
            auto r = d->port->writeBuffer(unit, func, buff, szInBuff);
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
            r = d->port->readBuffer(unit, func, buff, maxSzBuff, szOutBuff);
            if (!StatusIsBad(r))
            {
                if (unit != d->unit)
                    RAISE_ERROR(Status_BadNotCorrectResponse, StringLiteral("Not correct response. Requested unit (unit) is not equal to responsed"));

                if ((func & MBF_EXCEPTION) == MBF_EXCEPTION)
                {
                    if (*szOutBuff > 0)
                    {
                        auto errcode = buff[0];
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
            // no need break
        case STATE_CLOSED:
            d->state = STATE_BEGIN_OPEN;
            // no need break
        case STATE_BEGIN_OPEN:
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_OPEN;
            // no need break
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
            // no need break
        case STATE_BEGIN_WRITE:
            d->timestampRefresh();
            if (!d->port->isOpen())
            {
                d->state = STATE_CLOSED;
                RAISE_ERROR(Status_BadPortClosed, StringLiteral("Error: Port is closed when trying to write data"));
            }
            d->state = STATE_WRITE;
            // no need break
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
            // no need break
        case STATE_BEGIN_READ:
            d->timestampRefresh();
            d->state = STATE_READ;
            // no need break
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

