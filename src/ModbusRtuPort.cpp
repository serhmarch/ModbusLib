/*
    Modbus

    Created: 2024
    Author: Serhii Marchuk, https://github.com/serhmarch

    Copyright (C) 2024  Serhii Marchuk

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
#include "ModbusRtuPort.h"
#include "ModbusSerialPort_p.h"

ModbusRtuPort::ModbusRtuPort(bool blocking) :
    ModbusPort(ModbusSerialPortPrivate::create(MB_RTU_IO_BUFF_SZ, blocking))
{
}

StatusCode ModbusRtuPort::writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff)
{
    uint16_t crc;
    // 2 is unit and function bytes + 2 bytes CRC16
    if (szInBuff > MB_RTU_IO_BUFF_SZ-(sizeof(crc)+2))
        return this->setError(Status_BadWriteBufferOverflow, StringLiteral("RTU. Write-buffer overflow"));
    d_ptr->buff[0] = unit;
    d_ptr->buff[1] = func;
    memcpy(&d_ptr->buff[2], buff, szInBuff);
    d_ptr->sz = szInBuff + 2;
    crc = crc16(d_ptr->buff, d_ptr->sz);
    d_ptr->buff[d_ptr->sz  ] = reinterpret_cast<uint8_t*>(&crc)[0];
    d_ptr->buff[d_ptr->sz+1] = reinterpret_cast<uint8_t*>(&crc)[1];
    d_ptr->sz += 2;
    return Status_Good;
}

StatusCode ModbusRtuPort::readBuffer(uint8_t& unit, uint8_t& func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    uint16_t crc;
    if (d_ptr->sz < 4) // Note: Unit + Func + 2 bytes CRC
        return this->setError(Status_BadNotCorrectRequest, StringLiteral("RTU. Not correct input. Input data length to small"));

    crc = d_ptr->buff[d_ptr->sz-2] | (d_ptr->buff[d_ptr->sz-1] << 8);
    if (crc16(d_ptr->buff, d_ptr->sz-2) != crc)
        return this->setError(Status_BadCrc, StringLiteral("RTU. Wrong CRC"));

    unit = d_ptr->buff[0];
    func = d_ptr->buff[1];

    d_ptr->sz -= 4;
    if (d_ptr->sz > maxSzBuff)
        return d_ptr->setError(Status_BadReadBufferOverflow, StringLiteral("RTU. Read-buffer overflow"));
    memcpy(buff, &d_ptr->buff[2], d_ptr->sz);
    *szOutBuff = d_ptr->sz;
    return Status_Good;
}
