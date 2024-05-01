#include "ModbusClient.h"

namespace Modbus {

Client::Client(uint8_t unit, ClientPort *port)
{
    m_unit = unit;
    m_port = port;
    m_rp = port->createRequestParams(this);
    m_lastStatus      = Status_Uncertain;
    m_lastErrorStatus = Status_Uncertain;

}

Client::~Client()
{
    m_port->deleteRequestParams(m_rp);
}

Modbus::Type Client::type() const
{
    return m_port->type();
}

bool Client::isOpen() const
{
    return m_port->isOpen();
}

Modbus::StatusCode Client::readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff,  fcBytes;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("Modbus::Client::readCoils(offset=%hu, count=%hu): Requested count of coils is too large"), offset, count);
            m_lastErrorText = buff;
            m_port->cancelRequest(m_rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Start coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Start coil offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];     // Quantity of coils - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];     // Quantity of coils - LS BYTE
        // no need break
    case ClientPort::Process:
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

Modbus::StatusCode Client::readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcBytes;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("Modbus::Client::readDiscreteInputs(offset=%hu, count=%hu): Requested count of inputs is too large"), offset, count);
            m_lastErrorText = buff;
            m_port->cancelRequest(m_rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Start input offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Start input offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];     // Quantity of inputs - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];     // Quantity of inputs - LS BYTE
        // no need break
    case ClientPort::Process:
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


Modbus::StatusCode Client::readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcRegs, fcBytes, i;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("Modbus::Client::readHoldingRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            m_lastErrorText = buff;
            m_port->cancelRequest(m_rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        // no need break
    case ClientPort::Process:
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

Modbus::StatusCode Client::readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, fcRegs, fcBytes, i;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("Modbus::Client::readInputRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            m_lastErrorText = buff;
            m_port->cancelRequest(m_rp);
            return Status_BadNotCorrectRequest;
        }
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Start register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Start register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&count)[1];  // Quantity of values - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&count)[0];  // Quantity of values - LS BYTE
        // no need break
    case ClientPort::Process:
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

Modbus::StatusCode Client::writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
{
    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];   // Coil offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];   // Coil offset - LS BYTE
        buff[2] = (value ? 0xFF : 0x00);                    // Value - 0xFF if true, 0x00 if false
        buff[3] = 0x00;                                     // Value - must always be NULL
        // no need break
    case ClientPort::Process:
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

Modbus::StatusCode Client::writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
{
    const uint16_t szBuff = 4;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, outValue;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        buff[0] = reinterpret_cast<uint8_t*>(&offset)[1];    // Register offset - MS BYTE
        buff[1] = reinterpret_cast<uint8_t*>(&offset)[0];    // Register offset - LS BYTE
        buff[2] = reinterpret_cast<uint8_t*>(&value)[1];     // Value - MS BYTE
        buff[3] = reinterpret_cast<uint8_t*>(&value)[0];     // Value - LS BYTE
        // no need break
    case ClientPort::Process:
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

StatusCode Client::readExceptionStatus(uint8_t unit, uint8_t *value)
{
    const uint16_t szBuff = 1;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff;

    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        // no need break
    case ClientPort::Process:
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

Modbus::StatusCode Client::writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    const int szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, outOffset, fcCoils, fcBytes;


    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        if (count > MB_MAX_DISCRETS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("Modbus::Client::writeMultipleCoils(offset=%hu, count=%hu): Requested count of coils is too large"), offset, count);
            m_lastErrorText = buff;
            m_port->cancelRequest(m_rp);
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
    case ClientPort::Process:
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

Modbus::StatusCode Client::writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    const uint16_t szBuff = 300;

    uint8_t buff[szBuff];
    Modbus::StatusCode r;
    uint16_t szOutBuff, i, outOffset, fcRegs;


    ClientPort::RequestStatus status = m_port->getRequestStatus(m_rp);
    switch (status)
    {
    case ClientPort::Enable:
        if (count > MB_MAX_REGISTERS)
        {
            const size_t len = 200;
            Char buff[len];
            snprintf(buff, len, StringLiteral("Modbus::Client::writeMultipleRegisters(offset=%hu, count=%hu): Requested count of registers is too large"), offset, count);
            m_lastErrorText = buff;
            m_port->cancelRequest(m_rp);
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
    case ClientPort::Process:
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

Modbus::StatusCode Client::readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    Modbus::StatusCode r = readCoils(unit, offset, count, m_buff);
    if (r != Status_Good)
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (m_buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}

Modbus::StatusCode Client::readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    Modbus::StatusCode r = readCoils(unit, offset, count, m_buff);
    if (r != Status_Good)
        return r;
    for (int i = 0; i < count; ++i)
        values[i] = (m_buff[i / 8] & static_cast<uint8_t>(1 << (i % 8))) != 0;
    return Status_Good;
}

Modbus::StatusCode Client::writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values)
{
    if (m_port->currentClient() == nullptr)
    {
        for (int i = 0; i < count; i++)
        {
            if (!(i & 7))
                m_buff[i / 8] = 0;
            if (values[i] != 0)
                m_buff[i / 8] |= (1 << (i % 8));
        }
        return writeMultipleCoils(unit, offset, count, m_buff);
    }
    else if (m_port->currentClient() == this)
        return writeMultipleCoils(unit, offset, count, m_buff);
    return Status_Processing;
}

StatusCode Client::request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t * szOutBuff)
{
    Modbus::StatusCode r = m_port->request(unit, func, buff, szInBuff, maxSzBuff, szOutBuff);
    if (StatusIsProcessing(r))
        return r;
    m_lastStatus = r;
    if (StatusIsBad(r))
    {
        m_lastErrorStatus = r;
        m_lastErrorText = m_port->lastErrorText();
    }
    return r;
}

} // namespace Modbus
