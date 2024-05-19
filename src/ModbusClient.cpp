#include "ModbusClient.h"
#include "ModbusClient_p.h"

ModbusClient::ModbusClient(uint8_t unit, ModbusClientPort *port) :
    ModbusObject(new ModbusClientPrivate)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);
    d->unit            = unit;
    d->port            = port;
    d->rp              = port->createRequestParams(this);
    d->lastStatus      = Status_Uncertain;
    d->lastErrorStatus = Status_Uncertain;

}

ModbusClient::~ModbusClient()
{
    d_ModbusClient(d_ptr)->port->deleteRequestParams(d_ModbusClient(d_ptr)->rp);
}

Modbus::ProtocolType ModbusClient::type() const
{
    return d_ModbusClient(d_ptr)->port->type();
}

uint8_t ModbusClient::unit() const
{
    return d_ModbusClient(d_ptr)->unit;
}

void ModbusClient::setUnit(uint8_t unit)
{
    d_ModbusClient(d_ptr)->unit = unit;
}

bool ModbusClient::isOpen() const
{
    return d_ModbusClient(d_ptr)->port->isOpen();
}

ModbusClientPort *ModbusClient::port() const
{
    return d_ModbusClient(d_ptr)->port;
}

Modbus::StatusCode ModbusClient::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff,  fcBytes;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("ModbusClient::readCoils(offset=%hu, count=%hu): Requested count of coils is too large"), offset, count);
            d->lastErrorText = buff;
            d->port->cancelRequest(d->rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Start coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Start coil offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];     // Quantity of coils - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];     // Quantity of coils - LS BYTE
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_READ_COILS,                 // modbus function number
            buff,                           // in-out buffer
            4,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;
        if (!szOutBuff)
            return Status_BadNotCorrectResponse;
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return Status_BadNotCorrectResponse;
        if (fcBytes != ((count + 7) / 8))
            return Status_BadNotCorrectResponse;
        memcpy(values, &buff[1], fcBytes);
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcBytes;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("ModbusClient::readDiscreteInputs(offset=%hu, count=%hu): Requested count of inputs is too large"), offset, count);
            d->lastErrorText = buff;
            d->port->cancelRequest(d->rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Start input offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Start input offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];     // Quantity of inputs - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];     // Quantity of inputs - LS BYTE
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_READ_DISCRETE_INPUTS,       // modbus function number
            buff,                           // in-out buffer
            4,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;
        if (!szOutBuff)
            return Status_BadNotCorrectResponse;
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return Status_BadNotCorrectResponse;
        if (fcBytes != ((count + 7) / 8))
            return Status_BadNotCorrectResponse;
        memcpy(values, &buff[1], fcBytes);
        return Status_Good;
    default:
        return Status_Processing;
    }
}


Modbus::StatusCode ModbusClient::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcRegs, fcBytes, i;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("ModbusClient::readHoldingRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            d->lastErrorText = buff;
            d->port->cancelRequest(d->rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_READ_HOLDING_REGISTERS,     // modbus function number
            buff,                           // in-out buffer
            4,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;
        if (!szOutBuff)
            return Status_BadNotCorrectResponse;
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return Status_BadNotCorrectResponse;
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != count)
            return Status_BadNotCorrectResponse;
        for (i = 0; i < fcRegs; i++)
            values[i] = (buff[i * 2 + 1] << 8) | buff[i * 2 + 2];
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcRegs, fcBytes, i;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("ModbusClient::readInputRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            d->lastErrorText = buff;
            d->port->cancelRequest(d->rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_READ_INPUT_REGISTERS,       // modbus function number
            buff,                           // in-out buffer
            4,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good) // processing
            return r;
        if (!szOutBuff)
            return Status_BadNotCorrectResponse;
        fcBytes = buff[0];  // count of bytes received
        if (fcBytes != szOutBuff - 1)
            return Status_BadNotCorrectResponse;
        fcRegs = fcBytes / sizeof(uint16_t); // count values received
        if (fcRegs != count)
            return Status_BadNotCorrectResponse;
        for (i = 0; i < fcRegs; i++)
            values[i] = (buff[i * 2 + 1] << 8) | buff[i * 2 + 2];
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];   // Coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];   // Coil offset - LS BYTE
        buff[2] = (value ? 0xFF : 0x00);                    // Value - 0xFF if true, 0x00 if false
        buff[3] = 0x00;                                     // Value - must always be NULL
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_WRITE_SINGLE_COIL,          // modbus function number
            buff,                           // in-out buffer
            4,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;
        if (szOutBuff != 4)
            return Status_BadNotCorrectResponse;

        outOffset = buff[1] | (buff[0] << 8);
        if (outOffset != offset)
            return Status_BadNotCorrectResponse;
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, outValue;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&value)[1];     // Value - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&value)[0];     // Value - LS BYTE
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_WRITE_SINGLE_REGISTER,      // modbus function number
            buff,                           // in-out buffer
            4,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;

        if (szOutBuff != 4)
            return Status_BadNotCorrectResponse;

        outOffset = buff[1] | (buff[0] << 8);
        outValue = buff[3] | (buff[2] << 8);
        if (!(outOffset == offset) && (outValue == value))
            return Status_BadNotCorrectResponse;
        return Status_Good;
    default:
        return Status_Processing;
    }
}

StatusCode ModbusClient::readExceptionStatus(uint8_t unit, uint8_t *value)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 1;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_READ_EXCEPTION_STATUS,      // modbus function number
            buff,                           // in-out buffer
            0,                              // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;

        if (szOutBuff != 1)
            return Status_BadNotCorrectResponse;
        *value = buff[0];
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const int szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, fcCoils, fcBytes;


    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("ModbusClient::writeMultipleCoils(offset=%hu, count=%hu): Requested count of coils is too large"), offset, count);
            d->lastErrorText = buff;
            d->port->cancelRequest(d->rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start coil offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of coils - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of coils - LS BYTE
        fcBytes = (count + 7) / 8;
        buff[4] = static_cast<uint8_t>(fcBytes);      // Quantity of next bytes
        memcpy(&buff[5], values, fcBytes);
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_WRITE_MULTIPLE_COILS,       // modbus function number
            buff,                           // in-out buffer
            5 + buff[4],                    // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;
        if (szOutBuff != 4)
            return Status_BadNotCorrectResponse;
        outOffset = (buff[0] << 8) | buff[1];
        if (outOffset != offset)
            return Status_BadNotCorrectResponse;
        fcCoils = (buff[2] << 8) | buff[3];
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, i, outOffset, fcRegs;


    ModbusClientPort::RequestStatus status = d->port->getRequestStatus(d->rp);
    switch (status)
    {
    case ModbusClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("ModbusClient::writeMultipleRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            d->lastErrorText = buff;
            d->port->cancelRequest(d->rp);
            return Status_BadNotCorrectRequest;
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
        // no need break
    case ModbusClientPort::Process:
        r = this->request(unit,             // unit ID
            MBF_WRITE_MULTIPLE_REGISTERS,   // modbus function number
            buff,                           // in-out buffer
            5 + buff[4],                    // count of input data bytes
            szBuff,                         // maximum size of buffer
            &szOutBuff);                    // count of output data bytes
        if (r != Status_Good)
            return r;
        if (szOutBuff != 4)
            return Status_BadNotCorrectResponse;
        outOffset = (buff[0] << 8) | buff[1];
        if (outOffset != offset)
            return Status_BadNotCorrectResponse;
        fcRegs = (buff[2] << 8) | buff[3];
        return Status_Good;
    default:
        return Status_Processing;
    }
}

Modbus::StatusCode ModbusClient::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    Modbus::StatusCode r = readCoils(unit, offset, count, d->buff);
    if (r != Status_Good)
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (d->buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}

Modbus::StatusCode ModbusClient::readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    Modbus::StatusCode r = readCoils(unit, offset, count, d->buff);
    if (r != Status_Good)
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (d->buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}

Modbus::StatusCode ModbusClient::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    if (d->port->currentClient() == nullptr)
    {
        for (int i = 0; i < count; i++)
        {
            if (!(i & 7))
                d->buff[i / 8] = 0;
            if (values[i] != 0)
                d->buff[i / 8] |= (1 << (i % 8));
        }
        return writeMultipleCoils(unit, offset, count, d->buff);
    }
    else if (d->port->currentClient() == this)
        return writeMultipleCoils(unit, offset, count, d->buff);
    return Status_Processing;
}

StatusCode ModbusClient::lastStatus() const
{
    return d_ModbusClient(d_ptr)->lastStatus;
}

StatusCode ModbusClient::lastErrorStatus() const
{
    return d_ModbusClient(d_ptr)->lastErrorStatus;
}

const Char *ModbusClient::lastErrorText() const
{
    return d_ModbusClient(d_ptr)->lastErrorText.data();
}

StatusCode ModbusClient::request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t * szOutBuff)
{
    ModbusClientPrivate *d = d_ModbusClient(d_ptr);

    Modbus::StatusCode r = d->port->request(unit, func, buff, szInBuff, maxSzBuff, szOutBuff);
    if (StatusIsProcessing(r))
        return r;
    d->lastStatus = r;
    if (StatusIsBad(r))
    {
        d->lastErrorStatus = r;
        d->lastErrorText = d->port->lastErrorText();
    }
    return r;
}
