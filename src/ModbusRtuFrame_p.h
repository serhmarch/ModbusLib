#ifndef MODBUSRTUFRAME_P_H
#define MODBUSRTUFRAME_P_H

#include "ModbusFrame_p.h"

class ModbusRtuFramePrivate : public ModbusFramePrivate
{
public:
    ModbusRtuFramePrivate() : ModbusFramePrivate(MB_RTU_IO_BUFF_SZ)
    {
    }

public:
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) override
    {
        uint16_t crc;
        // 2 is unit and function bytes + 2 bytes CRC16
        if (szInBuff > MB_RTU_IO_BUFF_SZ-(sizeof(crc)+2))
            return this->setError(Status_BadWriteBufferOverflow, StringLiteral("RTU. Write-buffer overflow"));
        this->buff[0] = unit;
        this->buff[1] = func;
        memcpy(&this->buff[2], buff, szInBuff);
        this->sz = szInBuff + 2;
        crc = crc16(this->buff, this->sz);
        this->buff[this->sz  ] = reinterpret_cast<uint8_t*>(&crc)[0];
        this->buff[this->sz+1] = reinterpret_cast<uint8_t*>(&crc)[1];
        this->sz += 2;
        return Status_Good;
    }

    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override
    {
        uint16_t crc;
        if (this->sz < 4) // Note: Unit + Func + 2 bytes CRC
            return this->setError(Status_BadNotCorrectRequest, StringLiteral("RTU. Not correct input. Input data length to small"));

        crc = this->buff[this->sz-2] | (this->buff[this->sz-1] << 8);
        if (crc16(this->buff, this->sz-2) != crc)
            return this->setError(Status_BadCrc, StringLiteral("RTU. Wrong CRC"));

        unit = this->buff[0];
        func = this->buff[1];

        this->sz -= 4;
        if (this->sz > maxSzBuff)
            return this->setError(Status_BadReadBufferOverflow, StringLiteral("RTU. Read-buffer overflow"));
        memcpy(buff, &this->buff[2], this->sz);
        *szOutBuff = this->sz;
        return Status_Good;

    }
};

#endif // MODBUSRTUFRAME_P_H