#include "ModbusClientPort.h"
#include "ModbusClientPort_p.h"

#include "ModbusPort.h"

ModbusClientPort::ModbusClientPort(ModbusPort *port) :
    ModbusObject(new ModbusClientPortPrivate(port))
{
}

ProtocolType ModbusClientPort::type() const
{
    return d_ModbusClientPort(d_ptr)->port->type();
}

StatusCode ModbusClientPort::close()
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    StatusCode s = d->port->close();
    signalClosed(this->objectName());
    d->currentClient = nullptr;
    d->setPortStatus(s);
    return s;
}

bool ModbusClientPort::isOpen() const
{
    return d_ModbusClientPort(d_ptr)->port->isOpen();
}

uint32_t ModbusClientPort::tries() const
{
    return d_ModbusClientPort(d_ptr)->settings.tries;
}

void ModbusClientPort::setTries(uint32_t v)
{
    if (v > 0)
        d_ModbusClientPort(d_ptr)->settings.tries = v;
}

bool ModbusClientPort::isBroadcastEnabled() const
{
    return d_ModbusClientPort(d_ptr)->isBroadcastEnabled();
}

void ModbusClientPort::setBroadcastEnabled(bool enable)
{
    d_ModbusClientPort(d_ptr)->setBroadcastEnabled(enable);
}

#ifndef MBF_READ_COILS_DISABLE
Modbus::StatusCode ModbusClientPort::readCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::readCoils(offset=%hu, count=%hu): Requested count of coils is too large"), offset, count);
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
                          buff,             // in buffer
                          4,                // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (!szOutBuff)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        if (fcBytes != ((d->count + 7) / 8))
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'ByteCount' is not match received one"));
        memcpy(values, &buff[1], fcBytes);
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
Modbus::StatusCode ModbusClientPort::readDiscreteInputs(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::readDiscreteInputs(offset=%hu, count=%hu): Requested count of inputs is too large"), offset, count);
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
                          buff,                     // in buffer
                          4,                        // count of input data bytes
                          buff,                     // out buffer
                          szBuff,                   // maximum size of buffer
                          &szOutBuff);              // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (!szOutBuff)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        if (fcBytes != ((d->count + 7) / 8))
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'ByteCount' is not match received one"));
        memcpy(values, &buff[1], fcBytes);
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
Modbus::StatusCode ModbusClientPort::readHoldingRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::readHoldingRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
                          buff,                         // in buffer
                          4,                            // count of input data bytes
                          buff,                         // out buffer
                          szBuff,                       // maximum size of buffer
                          &szOutBuff);                  // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (!szOutBuff)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != d->count)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Count' is not match received one"));
        for (i = 0; i < fcRegs; i++)
            values[i] = (buff[i*2+1] << 8) | buff[i*2+2];
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
Modbus::StatusCode ModbusClientPort::readInputRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::readInputRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
                          buff,                           // in buffer
                          4,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast()) // processing
            return r;
        if (!szOutBuff)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != d->count)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Count' is not match received one"));
        for (i = 0; i < fcRegs; i++)
            values[i] = (buff[i*2+1] << 8) | buff[i*2+2];
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
Modbus::StatusCode ModbusClientPort::writeSingleCoil(ModbusObject *client, uint8_t unit, uint16_t offset, bool value)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
        r = this->request(unit,                           // unit ID
                          MBF_WRITE_SINGLE_COIL,          // modbus function number
                          buff,                           // in buffer
                          4,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (szOutBuff != 4)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));

        outOffset = buff[1] | (buff[0] << 8);
        if (outOffset != d->offset)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Offset' is not match received one"));
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
Modbus::StatusCode ModbusClientPort::writeSingleRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t value)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
        r = this->request(unit,                           // unit ID
                          MBF_WRITE_SINGLE_REGISTER,      // modbus function number
                          buff,                           // in buffer
                          4,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff != 4)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));

        outOffset = buff[1] | (buff[0] << 8);
        if (outOffset != d->offset)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Offset' is not match received one"));
        outValue = buff[3] | (buff[2] << 8);
        if (outValue != d->value)   
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Value' is not match received one"));
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode ModbusClientPort::readExceptionStatus(ModbusObject *client, uint8_t unit, uint8_t *value)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
        r = this->request(unit,                           // unit ID
                          MBF_READ_EXCEPTION_STATUS,      // modbus function number
                          buff,                           // in buffer
                          0,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff != 1)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        *value = buff[0];
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
Modbus::StatusCode ModbusClientPort::diagnostics(ModbusObject *client, uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
                          buff,             // in buffer
                          insize+2,         // count of input data bytes
                          buff,             // out buffer
                          szBuff,           // maximum size of buffer
                          &szOutBuff);      // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff < 2)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));

        outSubfunc = buff[1] | (buff[0] << 8);
        if (outSubfunc != d->subfunc)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Subfunc' is not match received one"));
        *outsize = static_cast<uint8_t>(szOutBuff-2);
        memcpy(outdata, &buff[2], *outsize);
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
Modbus::StatusCode ModbusClientPort::getCommEventCounter(ModbusObject *client, uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
                          buff,                       // in buffer
                          0,                          // count of input data bytes
                          buff,                       // out buffer
                          szBuff,                     // maximum size of buffer
                          &szOutBuff);                // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff != 4)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        *status = buff[1] | (buff[0] << 8);
        *eventCount = buff[3] | (buff[2] << 8);
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
Modbus::StatusCode ModbusClientPort::getCommEventLog(ModbusObject *client, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
                          buff,                   // in buffer
                          0,                      // count of input data bytes
                          buff,                   // out buffer
                          szBuff,                 // maximum size of buffer
                          &szOutBuff);            // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff < 7)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        byteCount = buff[0];
        if (szOutBuff != (byteCount+1))
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'ByteCount' doesn't match with received data size"));
        *status       = buff[2] | (buff[1] << 8);
        *eventCount   = buff[4] | (buff[3] << 8);
        *messageCount = buff[6] | (buff[5] << 8);

        byteCount = byteCount-6;
        if (byteCount > GET_COMM_EVENT_LOG_MAX)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'EventCount' is bigger than 64"));
        *eventBuffSize = byteCount;
        memcpy(eventBuff, &buff[7], byteCount);
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
Modbus::StatusCode ModbusClientPort::writeMultipleCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::writeMultipleCoils(offset=%hu, count=%hu): Requested count of coils is too large"), offset, count);
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
                          buff,                     // in buffer
                          5 + buff[4],              // count of input data bytes
                          buff,                     // out buffer
                          szBuff,                   // maximum size of buffer
                          &szOutBuff);              // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (szOutBuff != 4)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        outOffset = (buff[0] << 8) | buff[1];
        if (outOffset != d->offset)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Offset' is not match received one"));
        outCount = (buff[2] << 8) | buff[3];
        if (outCount != d->count)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Count' is not match received one"));
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
Modbus::StatusCode ModbusClientPort::writeMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::writeMultipleRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
        r = this->request(unit,             // unit ID
                          MBF_WRITE_MULTIPLE_REGISTERS,   // modbus function number
                          buff,                           // in buffer
                          5 + buff[4],                    // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (szOutBuff != 4)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        outOffset = (buff[0] << 8) | buff[1];
        if (outOffset != d->offset)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Offset' is not match received one"));
        outCount = (buff[2] << 8) | buff[3];
        if (outCount != d->count)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Count' is not match received one"));
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
Modbus::StatusCode ModbusClientPort::reportServerID(ModbusObject *client, uint8_t unit, uint8_t *count, uint8_t *data)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
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
                          buff,                 // in buffer
                          0,                    // count of input data bytes
                          buff,                 // out buffer
                          szBuff,               // maximum size of buffer
                          &szOutBuff);          // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff == 0)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        *count = buff[0];
        memcpy(data, &buff[1], *count);
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode ModbusClientPort::maskWriteRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
                          buff,                           // in buffer
                          6,                              // count of input data bytes
                          buff,                           // out buffer
                          szBuff,                         // maximum size of buffer
                          &szOutBuff);                    // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;

        if (szOutBuff != 6)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));

        outOffset  = buff[1] | (buff[0] << 8);
        outAndMask = buff[3] | (buff[2] << 8);
        outOrMask  = buff[5] | (buff[4] << 8);
        if (outOffset != d->offset)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'Offset' is not match received one"));
        if (outAndMask != d->andMask)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'AndMask' is not match received one"));
        if (outOrMask != d->orMask)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'OrMask' is not match received one"));
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode ModbusClientPort::readWriteMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
            snprintf(errbuff, len, StringLiteral("ModbusClientPort::readWriteMultipleRegisters(): Requested count of registers is too large"));
            this->cancelRequest(client);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
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
                          buff,                             // in buffer
                          9 + buff[8],                      // count of input data bytes
                          buff,                             // out buffer
                          szBuff,                           // maximum size of buffer
                          &szOutBuff);                      // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast())
            return r;
        if (!szOutBuff)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("No data was received"));
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != d->count)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Count registers to read is not match received one"));
        for (i = 0; i < fcRegs; i++)
            readValues[i] = (buff[i * 2 + 1] << 8) | buff[i * 2 + 2];
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
Modbus::StatusCode ModbusClientPort::readFIFOQueue(ModbusObject *client, uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
                          buff,                // in buffer
                          2,                   // count of input data bytes
                          buff,                // out buffer
                          szBuff,              // maximum size of buffer
                          &szOutBuff);         // count of output data bytes
        if ((r != Status_Good) || d->isBroadcast()) // processing
            return r;
        if (szOutBuff < 4)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("Incorrect received data size"));
        bytesCount = buff[1] | (buff[0] << 8);
        if (bytesCount != (szOutBuff - 2))
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'ByteCount' doesn't match with received data size"));
        FIFOCount  = buff[3] | (buff[2] << 8);
        if (bytesCount != (FIFOCount + 1) * 2)
            return d->setError(Status_BadNotCorrectResponse, StringLiteral("'ByteCount' doesn't match with 'FIFOCount'"));
        if (FIFOCount > READ_FIFO_QUEUE_MAX)
            return d->setError(Status_BadIllegalDataValue, StringLiteral("'FIFOCount' is bigger than 31"));
        for (i = 0; i < FIFOCount; i++)
            values[i] = buff[i*2+5] | (buff[i*2+4] << 8);
        *count = FIFOCount;
        return d->setGoodStatus();
    default:
        return Status_Processing;
    }
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
Modbus::StatusCode ModbusClientPort::readCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

    Modbus::StatusCode r = readCoils(client, unit, offset, count, d->buff);
    if ((r != Status_Good) || d->isBroadcast())
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (d->buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
Modbus::StatusCode ModbusClientPort::readDiscreteInputsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

    Modbus::StatusCode r = readDiscreteInputs(client, unit, offset, count, d->buff);
    if ((r != Status_Good) || d->isBroadcast())
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (d->buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
Modbus::StatusCode ModbusClientPort::writeMultipleCoilsAsBoolArray(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const bool *values)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);

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
Modbus::StatusCode ModbusClientPort::diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata)
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

ModbusPort *ModbusClientPort::port() const
{
    return d_ModbusClientPort(d_ptr)->port;
}

void ModbusClientPort::setPort(ModbusPort *port)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
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
    return d_ModbusClientPort(d_ptr)->lastStatus;
}

Modbus::Timestamp ModbusClientPort::lastStatusTimestamp() const
{
    return d_ModbusClientPort(d_ptr)->lastStatusTimestamp;
}

Modbus::StatusCode ModbusClientPort::lastErrorStatus() const
{
    return d_ModbusClientPort(d_ptr)->port->lastErrorStatus();
}

const Char *ModbusClientPort::lastErrorText() const
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    if (d->isLastPortError)
        return d->port->lastErrorText();
    else
        return d->lastErrorText.data();
}

uint32_t ModbusClientPort::lastTries() const
{
    return d_ModbusClientPort(d_ptr)->lastTries;
}

const ModbusObject *ModbusClientPort::currentClient() const
{
    return d_ModbusClientPort(d_ptr)->currentClient;
}

ModbusClientPort::RequestStatus ModbusClientPort::getRequestStatus(ModbusObject *client)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
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
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
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

StatusCode ModbusClientPort::rawRequest(const void *inBuff, uint16_t szInBuff, void *outBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    RequestStatus rs = getRequestStatus(this);
    if (rs == Disable)
        return Status_Processing;
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    if (!d->isWriteBufferBlocked())
    {
        // TODO: set `d->unit = 0` and find reason of the crash
        d->unit = 1; // Note: prevent broadcast false recognition
        d->func = 0;
        d->lastTries = 0;
        StatusCode s = d->port->writeRawBuffer(inBuff, szInBuff);
        if (StatusIsBad(s))
            return s;
        d->blockWriteBuffer();
    }
    StatusCode r = process();
    if (StatusIsProcessing(r))
        return r;
    d->lastTries = ++d->repeats;
    if (StatusIsBad(r) && (d->repeats < d->settings.tries))
    {
        d->port->setNextRequestRepeated(true);
        return Status_Processing;
    }
    d->freeWriteBuffer();
    d->repeats = 0;
    d->currentClient = nullptr;
    if (StatusIsBad(r))
        return r;
    if (!d->isBroadcast())
    {
        r = d->port->readRawBuffer(outBuff, maxSzBuff, szOutBuff);
    }
    return d->setPortStatus(r);
}

StatusCode ModbusClientPort::request(uint8_t unit, uint8_t func, const uint8_t *inBuff, uint16_t szInBuff, uint8_t *outBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    if (!d->isWriteBufferBlocked())
    {
        d->unit = unit;
        d->func = func;
        d->lastTries = 0;
        StatusCode s = d->port->writeBuffer(unit, func, inBuff, szInBuff);
        if (StatusIsBad(s))
            return s;
        d->blockWriteBuffer();
    }
    StatusCode r = process();
    if (StatusIsProcessing(r))
        return r;
    d->lastTries = ++d->repeats;
    if (StatusIsBad(r) && (d->repeats < d->settings.tries))
    {
        d->port->setNextRequestRepeated(true);
        return Status_Processing;
    }
    d->freeWriteBuffer();
    d->repeats = 0;
    d->currentClient = nullptr;
    if (StatusIsBad(r))
        return r;
    if (!d->isBroadcast())
    {
        r = d->port->readBuffer(unit, func, outBuff, maxSzBuff, szOutBuff);
        if (StatusIsGood(r))
        {
            if (unit != d->unit)
                return d->setError(Status_BadNotCorrectResponse, StringLiteral("Not correct response. Requested unit (unit) is not equal to responsed"));

            if ((func & MBF_EXCEPTION) == MBF_EXCEPTION)
            {
                if (*szOutBuff > 0)
                {
                    r = static_cast<StatusCode>(outBuff[0]); // Returned modbus exception
                    return d->setError(static_cast<StatusCode>(Status_Bad | r), String(StringLiteral("Returned Modbus-exception with code "))+toModbusString(static_cast<int>(r)));
                }
                else
                    return d->setError(Status_BadNotCorrectResponse, StringLiteral("Exception status missed"));
            }

            if (func != d->func)
                return d->setError(Status_BadNotCorrectResponse, StringLiteral("Not correct response. Requested function is not equal to responsed"));
        }
    }
    return d->setPortStatus(r);
}

StatusCode ModbusClientPort::process()
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
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
            d->setPortStatus(r);
            if (StatusIsBad(r)) // an error occured
            {
                signalError(d->getName(), r, d->port->lastErrorText());
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
            //fRepeatAgain = true;
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
                fRepeatAgain = true;
                break;
            }
            d->state = STATE_WRITE;
            // no need break
        case STATE_WRITE:
            r = d->port->write();
            if (StatusIsProcessing(r))
                return r;
            d->setPortStatus(r);
            if (StatusIsBad(r)) // an error occured
            {
                signalError(d->getName(), r, d->port->lastErrorText());
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
                if (!d->port->isOpen())
                {
                    d->state = STATE_CLOSED;
                    signalClosed(this->objectName());
                    return Status_Uncertain;
                }
                signalRx(d->getName(), d->port->readBufferData(), d->port->readBufferSize());
                d->state = STATE_OPENED;
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

