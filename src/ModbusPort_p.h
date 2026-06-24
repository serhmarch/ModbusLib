#ifndef MODBUSPORT_P_H
#define MODBUSPORT_P_H

#include "ModbusObject_p.h"

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
    ModbusPortPrivate(bool blocking)
    {
        this->state = STATE_UNKNOWN;
        this->changed = false;
        this->modeServer = false;
        this->modeBlocking = blocking;
        this->errorStatus = Modbus::Status_Uncertain;
        // Note: settingsBase.timeout and settingsBase.timeoutInterByte must be initialized in derived classes
    }

    virtual ~ModbusPortPrivate()
    {
    }

public: //settings
    inline auto timeout() const { return settingsBase.timeout; }
    inline void setTimeout(uint32_t timeout) { settingsBase.timeout = timeout; }

    inline uint32_t timeoutFirstByte() const { return timeout(); }
    inline void setTimeoutFirstByte(uint32_t timeout) { setTimeout(timeout); }

    inline auto timeoutInterByte() const { return settingsBase.timeoutInterByte; }
    inline void setTimeoutInterByte(uint32_t timeout) { settingsBase.timeoutInterByte = timeout; }

public:
    inline bool isServerMode() const { return this->modeServer; }
    inline void setServerMode(bool server) { this->modeServer = server; }
    inline bool isBlocking() const { return modeBlocking; }
    inline bool isNonBlocking() const { return !modeBlocking; }
    inline bool isStateClosed() const { return state == STATE_CLOSED; }
    inline bool isChanged() const { return changed; }
    inline void setChanged(bool changed) { this->changed = changed; }
    inline void clearChanged() { setChanged(false); }
    inline StatusCode lastErrorStatus() { return errorStatus; }
    inline const Char *lastErrorText() { return errorText.data(); }
    inline StatusCode setError(StatusCode status, const String &text) { errorStatus = status; errorText = text; return status; }
    inline StatusCode setError(StatusCode status, String &&text) { errorStatus = status; errorText = text; return status; }

public:
    State state;
    bool changed;
    bool modeServer;
    bool modeBlocking;
    StatusCode errorStatus;
    String errorText;
    struct
    {
        uint32_t timeout;
        uint32_t timeoutInterByte;
    } settingsBase;

};

#endif // MODBUSPORT_P_H
