#ifndef MODBUSPORT_P_H
#define MODBUSPORT_P_H

#include "ModbusObject_p.h"

#include "ModbusFrame_p.h"

class ModbusPort;

namespace ModbusPortPrivateNS {

enum State
{
    STATE_UNKNOWN = 0,
    STATE_WAIT_FOR_OPEN,
    STATE_OPENED,
    STATE_PREPARE_TO_READ,
    STATE_WAIT_FOR_READ,
    STATE_WAIT_FOR_READ_ALL,
    STATE_PREPARE_TO_WRITE,
    STATE_WAIT_FOR_WRITE,
    STATE_WAIT_FOR_WRITE_ALL,
    STATE_WAIT_FOR_CLOSE,
    STATE_CLOSED,
    STATE_END = STATE_CLOSED
};

} // namespace ModbusPortPrivateNS

using namespace Modbus;
using namespace ModbusPortPrivateNS;

class ModbusPortPrivate
{
public:
    ModbusPortPrivate(ModbusFramePrivate *f, bool blocking) :
        frame(f),
        modeBlocking(blocking),
        state(STATE_UNKNOWN),
        changed(false),
        errorStatus(Modbus::Status_Uncertain)
    {
        // this->settingsBase.timeout must be initialized in derived classes
    }

    virtual ~ModbusPortPrivate()
    {
        delete this->frame;
    }

public: //settings
    inline uint32_t timeout() const { return settingsBase.timeout; }
    
public:
    inline bool modeServer() const { return frame->modeServer; }
    inline void setModeServer(bool server) { frame->modeServer = server; }
    inline bool isBlocking() const { return modeBlocking; }
    inline bool isStateClosed() const { return state == STATE_CLOSED; }
    inline bool isChanged() const { return changed; }
    inline void setChanged(bool changed) { this->changed = changed; }
    inline void clearChanged() { setChanged(false); }
    inline uint8_t* buff() const { return frame->buff; }
    inline uint16_t buffSz() const { return frame->sz; }
    inline void setBuffSz(uint16_t sz) { frame->sz = sz; }
    inline void addBuffSz(uint16_t sz) { frame->sz += sz; }
    inline uint16_t buffMaxSz() const { return frame->c_buffSz; }
    inline StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) { return frame->writeBuffer(unit, func, buff, szInBuff); }
    inline StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) { return frame->readBuffer(unit, func, buff, maxSzBuff, szOutBuff); }
    inline StatusCode lastErrorStatus() { return frame->lastErrorStatus(); }
    inline const Char *lastErrorText() { return frame->lastErrorText(); }
    inline StatusCode setError(StatusCode status, const String &text) { return frame->setError(status, text); }
    inline StatusCode setError(StatusCode status, String &&text) { return frame->setError(status, std::move(text)); }

public:
    ModbusFramePrivate *frame;
    State state;
    bool changed;
    bool modeBlocking;
    StatusCode errorStatus;
    String errorText;
    struct
    {
        uint32_t timeout;
    } settingsBase;

};

#endif // MODBUSPORT_P_H
