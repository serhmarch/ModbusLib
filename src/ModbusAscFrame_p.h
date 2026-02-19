#ifndef MODBUSASCFRAME_P_H
#define MODBUSASCFRAME_P_H

#include "ModbusFrame_p.h"

class ModbusAscFramePrivate : public ModbusFramePrivate
{
public:
    ModbusAscFramePrivate() : ModbusFramePrivate(MB_ASC_IO_BUFF_SZ)
    {
    }

public:
    StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) override
    {
        const uint16_t szIBuff = MB_ASC_IO_BUFF_SZ/2;
        uint8_t ibuff[szIBuff];
        // 3 is unit, func and LRC bytes
        if (szInBuff > szIBuff-3)
            return this->setError(Status_BadWriteBufferOverflow, StringLiteral("ASCII. Write-buffer overflow"));
        ibuff[0] = unit;
        ibuff[1] = func;
        memcpy(&ibuff[2], buff, szInBuff);
        ibuff[szInBuff + 2] = lrc(ibuff, szInBuff+2);
        this->sz = bytesToAscii(ibuff, &this->buff[1], szInBuff + 3);
        this->buff[0] = ':' ;  // start ASCII-message character
        this->buff[this->sz+1] = '\r';  // CR
        this->buff[this->sz+2] = '\n';  // LF
        this->sz += 3;
        return Status_Good;
    }

    StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override
    {
        const uint16_t szIBuff = MB_ASC_IO_BUFF_SZ/2;
        uint8_t ibuff[szIBuff];

        if (this->sz < 9) // Note: 9 = 1(':')+2(unit)+2(func)+2(lrc)+1('\r')+1('\n')
            return this->setError(Status_BadNotCorrectRequest, StringLiteral("ASCII. Not correct response. Responsed data length is too small"));

        if (this->buff[0] != ':')
            return this->setError(Status_BadAscMissColon, StringLiteral("ASCII. Missed colon ':' symbol"));

        if ((this->buff[this->sz-2] != '\r') || (this->buff[this->sz-1] != '\n'))
            return this->setError(Status_BadAscMissCrLf, StringLiteral("ASCII. Missed CR-LF ending symbols"));

        if ((this->sz = asciiToBytes(&this->buff[1], ibuff, this->sz-3)) == 0)
            return this->setError(Status_BadAscChar, StringLiteral("ASCII. Bad ASCII symbol"));

        if (lrc(ibuff, this->sz-1) != ibuff[this->sz-1])
            return this->setError(Status_BadLrc, StringLiteral("ASCII. Error LRC"));

        unit = ibuff[0];
        func = ibuff[1];

        this->sz -= 3; // Note: 3 = 1(unit)+1(func)+1(lrc)
        if (this->sz > maxSzBuff)
            return this->setError(Status_BadReadBufferOverflow, StringLiteral("ASCII. Read-buffer overflow"));
        memcpy(buff, &ibuff[2], this->sz);
        *szOutBuff = this->sz;
        return Status_Good;
    }
};

#endif // MODBUSASCFRAME_P_H