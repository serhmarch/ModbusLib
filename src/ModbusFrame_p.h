#ifndef MODBUSFRAME_P_H
#define MODBUSFRAME_P_H

#include "Modbus.h"

using namespace Modbus;

class ModbusFramePrivate
{
public:
    ModbusFramePrivate(uint16_t maxBuffSize) :
        c_buffSz(maxBuffSize),
        buff(new uint8_t[maxBuffSize]),
        sz(0),
        modeServer(false),
        errorStatus(Status_Uncertain)
    {
    }

    ~ModbusFramePrivate()
    {
        delete[] this->buff;
    }

public:
    inline StatusCode lastErrorStatus() { return errorStatus; }
    inline const Char *lastErrorText() { return errorText.data(); }
    inline StatusCode setError(StatusCode status, const String &text) { errorStatus = status; errorText = text; return status; }
    inline StatusCode setError(StatusCode status, String &&text) { errorStatus = status; errorText = text; return status; }

public:
    virtual Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) = 0;
    virtual Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) = 0;

public:
    // buffer
    const uint16_t c_buffSz;
    uint8_t *buff;
    uint16_t sz;
    // mode
    bool modeServer;
    // error
    StatusCode errorStatus;
    String errorText;
};

#endif // MODBUSFRAME_P_H
