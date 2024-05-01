/*
    Modbus

    Created: 2023
    Author: Serhii Marchuk, https://github.com/serhmarch

    Copyright (C) 2023  Serhii Marchuk

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "ModbusPortRTU.h"

namespace Modbus {

PortRTU::PortRTU(bool blocking) : PortSerial(blocking)
{
    c_buffSz = MB_RTU_IO_BUFF_SZ;
    m_buff = new uint8_t[c_buffSz];
}

PortRTU::~PortRTU()
{
    delete m_buff;
}

StatusCode PortRTU::writeBuffer(uint8_t slave, uint8_t func, uint8_t *buff, uint16_t szInBuff)
{
    if (!m_modeServer)
    {
        if (m_block)
            return Status_Processing;
        m_unit = slave;
        m_func = func;
        m_block = true;
    }
    uint16_t crc;
    // 2 is slave and function bytes + 2 bytes CRC16
    if (szInBuff > c_buffSz-(sizeof(crc)+2))
        return setError(Status_BadWriteBufferOverflow, StringLiteral("Write-buffer overflow"));
    m_buff[0] = slave;
    m_buff[1] = func;
    memcpy(&m_buff[2], buff, szInBuff);
    m_sz = szInBuff + 2;
    crc = Modbus::crc16(m_buff, m_sz);
    m_buff[m_sz  ] = reinterpret_cast<uint8_t*>(&crc)[0];
    m_buff[m_sz+1] = reinterpret_cast<uint8_t*>(&crc)[1];
    m_sz += 2;
    return Status_Good;
}

StatusCode PortRTU::readBuffer(uint8_t& slave, uint8_t& func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    uint16_t crc;
    if (m_sz < 5)
        return setError(Status_BadNotCorrectRequest, StringLiteral("Not correct response. Responsed data length to small"));

    crc = m_buff[m_sz-2] | (m_buff[m_sz-1] << 8);
    if (Modbus::crc16(m_buff, m_sz-2) != crc)
        return setError(Status_BadCrc, StringLiteral("Wrong CRC"));

    if (!m_modeServer)
    {
        if (m_buff[0] != m_unit)
            return setError(Status_BadNotCorrectRequest, StringLiteral("Not correct response. Requested unit (slave) address is not equal to responsed"));

        if ((m_buff[1] & MBF_EXCEPTION) == MBF_EXCEPTION)
        {
            StatusCode r = m_buff[2] ? static_cast<StatusCode>(m_buff[2]) : Status_Bad;
            return setError(static_cast<StatusCode>(r), StringLiteral("Returned modbus exception"));
           }
        if (m_buff[1] != m_func)
            return setError(Status_BadNotCorrectRequest, StringLiteral("Not correct response. Requested function is not equal to responsed"));
    }

    slave = m_buff[0];
    func = m_buff[1];

    m_sz -= 4;
    if (m_sz > maxSzBuff)
        return setError(Status_BadReadBufferOverflow, StringLiteral("Read-buffer overflow"));
    memcpy(buff, &m_buff[2], m_sz);
    *szOutBuff = m_sz;
    return Status_Good;
}

} // namespace Modbus
