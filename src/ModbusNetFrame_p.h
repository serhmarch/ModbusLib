#ifndef MODBUSNETFRAME_P_H
#define MODBUSNETFRAME_P_H

#include "ModbusFrame_p.h"

class ModbusNetFramePrivate : public ModbusFramePrivate
{
public:
    ModbusNetFramePrivate() : ModbusFramePrivate(MB_NET_IO_BUFF_SZ),
        autoIncrement(true),
        transaction(0)
    {
    }

public:
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) override
    {
        if (!this->modeServer)
        {
            this->transaction += this->autoIncrement;
            this->autoIncrement = true;
        } // if (!this->modeServer)
        // 8 = 6(TCP prefix size in bytes) + 2(unit and function bytes)
        if (szInBuff > MB_NET_IO_BUFF_SZ - 8)
            return this->setError(Status_BadWriteBufferOverflow, StringLiteral("NET. Write-buffer overflow"));
        // standart TCP message prefix
        this->buff[0] = static_cast<uint8_t>(this->transaction >> 8);  // transaction id
        this->buff[1] = static_cast<uint8_t>(this->transaction);       // transaction id
        this->buff[2] = 0;
        this->buff[3] = 0;
        uint16_t cBytes = szInBuff + 2; // quantity of next bytes
        this->buff[4] = reinterpret_cast<uint8_t*>(&cBytes)[1]; // quantity of next bytes (MSB)
        this->buff[5] = reinterpret_cast<uint8_t*>(&cBytes)[0]; // quantity of next bytes (LSB)
        // unit, function, data
        this->buff[6] = unit;
        this->buff[7] = func;
        memcpy(&this->buff[8], buff, szInBuff);
        this->sz = szInBuff + 8;
        return Status_Good;
    }

    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override
    {
        if (this->sz < 8)
            return this->setError(Status_BadNotCorrectResponse, StringLiteral("NET. Not correct response. Responsed data length to small"));

        uint16_t transaction = this->buff[1] | (this->buff[0] << 8);

        if (!((this->buff[2] == 0) && (this->buff[3] == 0)))
            return this->setError(Status_BadNotCorrectResponse, StringLiteral("NET. Not correct read-buffer's TCP-prefix"));

        uint16_t cBytes = this->buff[5] | (this->buff[4] << 8);
        if (cBytes != (this->sz-6))
            return this->setError(Status_BadNotCorrectResponse, StringLiteral("NET. Not correct read-buffer's TCP-prefix. Size defined in TCP-prefix is not equal to actual response-size"));
        
        if (this->modeServer)
        {
            this->transaction = transaction;
        }
        else
        {
            if (this->transaction != transaction)
                return this->setError(Status_BadNotCorrectResponse, StringLiteral("NET. Not correct response. Requested transaction id is not equal to responded"));
        }
        unit = this->buff[6];
        func = this->buff[7];

        this->sz = this->sz - 8;
        if (this->sz > maxSzBuff)
            this->sz = maxSzBuff;
        memcpy(buff, &this->buff[8], this->sz);
        *szOutBuff = this->sz;
        return Status_Good;
    }

public:
    bool autoIncrement;
    uint16_t transaction;
};

inline ModbusNetFramePrivate *d_net(ModbusFramePrivate *f) { return static_cast<ModbusNetFramePrivate*>(f); }

#endif // MODBUSNETFRAME_P_H