#include "ModbusPortTCP.h"

namespace Modbus {

PortTCP::Defaults::Defaults() :
    host   (StringLiteral("localhost")),
    port   (STANDARD_TCP_PORT),
    timeout(3000)
{
}

const PortTCP::Defaults &PortTCP::Defaults::instance()
{
    static const Defaults d;
    return d;
}

PortTCP::PortTCP(TCPSocket *socket, bool blocking) : Port(blocking)
{
    m_autoIncrement = true;
    m_host = StringLiteral("127.0.0.1");
    m_port = static_cast<uint16_t>(STANDARD_TCP_PORT);
    m_timeout = 3000;
    m_transaction = 0;
    constructorPrivate(socket);
}

PortTCP::PortTCP(bool synch) : PortTCP(nullptr, synch)
{
}

PortTCP::~PortTCP()
{
    destructorPrivate();
}

void PortTCP::setHost(const String & host)
{
    if (m_host != host)
    {
        m_host = host;
        setChanged(true);
    }
}

void PortTCP::setPort(uint16_t port) 
{ 
    if (m_port != port)
    {
        m_port = port;
        setChanged(true);
    }
}

void PortTCP::setTimeout(uint32_t timeout)
{
    if (m_timeout != timeout)
    {
        m_timeout = timeout;
        setChanged(true);
    }
}

void PortTCP::setNextRequestRepeated(bool v)
{
    m_autoIncrement = !v;
}

StatusCode PortTCP::writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff)
{
    if (!m_modeServer)
    {
        if (m_block)
            return Status_Processing;
        m_transaction += m_autoIncrement;
        m_autoIncrement = true;
        m_unit = unit;
        m_func = func;
        m_block = true;
    } // if (!m_modeServer)
    // 8 = 6(TCP prefix size in bytes) + 2(unit and function bytes)
    if (szInBuff > MBCLIENTTCP_BUFF_SZ - 8)
        return setError(Status_BadWriteBufferOverflow, StringLiteral("TCP. Write-buffer overflow"));
    // standart TCP message prefix
    m_buff[0] = static_cast<uint8_t>(m_transaction >> 8);  // transaction id
    m_buff[1] = static_cast<uint8_t>(m_transaction);       // transaction id
    m_buff[2] = 0;
    m_buff[3] = 0;
    uint16_t cBytes = szInBuff + 2; // quantity of next bytes
    m_buff[4] = reinterpret_cast<uint8_t*>(&cBytes)[1]; // quantity of next bytes (MSB)
    m_buff[5] = reinterpret_cast<uint8_t*>(&cBytes)[0]; // quantity of next bytes (LSB)
    // unit, function, data
    m_buff[6] = unit;
    m_buff[7] = func;
    memcpy(&m_buff[8], buff, szInBuff);
    m_sz = szInBuff + 8;
    return Status_Good;
}

StatusCode PortTCP::readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    if (m_sz < 8)
        return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Not correct response. Responsed data length to small"));

    uint16_t transaction = m_buff[1] | (m_buff[0] << 8);

    if (!((m_buff[2] == 0) && (m_buff[3] == 0)))
        return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Not correct read-buffer's TCP-prefix"));

    uint16_t cBytes = m_buff[5] | (m_buff[4] << 8);
    if (cBytes != (m_sz-6))
        return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Not correct read-buffer's TCP-prefix. Size defined in TCP-prefix is not equal to actual response-size"));
    
    if (m_modeServer)
    {
        m_transaction = transaction;
    }
    else
    {
        if (m_transaction != transaction)
            return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Not correct response. Requested transaction id is not equal to responded"));

        if (m_buff[6] != m_unit)
            return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Not correct response. Requested unit (unit) is not equal to responsed"));

        if ((m_buff[7] & MBF_EXCEPTION) == MBF_EXCEPTION)
        {
            if (m_sz > 8)
            {
                StatusCode r = static_cast<StatusCode>(m_buff[8]); // Returned modbus exception
                return setError(static_cast<StatusCode>(Status_Bad | r), String(StringLiteral("TCP. Returned Modbus-exception with code "))+toString(static_cast<int>(r)));
            }
            else
                return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Exception status missed"));
        }

        if (m_buff[7] != m_func)
            return setError(Status_BadNotCorrectResponse, StringLiteral("TCP. Not correct response. Requested function is not equal to responsed"));
    }
    unit = m_buff[6];
    func = m_buff[7];

    m_sz = m_sz - 8;
    if (m_sz > maxSzBuff)
        m_sz = maxSzBuff;
    memcpy(buff, &m_buff[8], m_sz);
    *szOutBuff = m_sz;
    return Status_Good;
}

} // namespace Modbus 
