#ifndef MODBUSCLIENTPORT_P_H
#define MODBUSCLIENTPORT_P_H

#include "ModbusObject_p.h"

#include "ModbusObject.h"
#include "ModbusPort.h"

namespace ModbusClientPortPrivateNS {

enum State
{
    STATE_UNKNOWN               = 0,
    STATE_BEGIN_OPEN            ,
    STATE_WAIT_FOR_OPEN         ,
    STATE_OPENED                ,
    STATE_BEGIN_WRITE           ,
    STATE_WRITE                 ,
    STATE_BEGIN_READ            ,
    STATE_READ                  ,
    STATE_WAIT_FOR_CLOSE        ,
    STATE_TIMEOUT               ,
    STATE_CLOSED                ,
    STATE_END = STATE_CLOSED
};

} // namespace ModbusClientPortPrivateNS

using namespace ModbusClientPortPrivateNS;

class ModbusClientPortPrivate : public ModbusObjectPrivate
{
public:
    ModbusClientPortPrivate(ModbusPort *port)
    {
        this->state = STATE_UNKNOWN;
        this->unit = 0;
        this->func = 0;
        this->offset = 0;
        this->count = 0;
        this->orMask = 0;
        this->block = false;;
        this->currentClient = nullptr;
        this->port = port;
        this->repeats = 0;
        this->lastStatus = Modbus::Status_Uncertain;
        this->lastErrorStatus = Modbus::Status_Uncertain;
        this->lastTries = 0;
        this->isLastPortError = true;
        this->timestamp = 0;
        this->lastStatusTimestamp = 0;
        this->settings.tries = 1;
        this->settings.broadcastEnabled = true;

        port->setServerMode(false);
    }

    ~ModbusClientPortPrivate()
    {
        delete this->port;
    }

public:
    inline bool isBroadcastEnabled() const { return settings.broadcastEnabled; }
    inline void setBroadcastEnabled(bool enable) { settings.broadcastEnabled = enable; }
    inline bool isBroadcast() const { return (unit == 0) && isBroadcastEnabled(); }
    inline void timestampRefresh() { timestamp = timer(); }
    inline bool isStateClosed() const { return state == STATE_CLOSED || state == STATE_TIMEOUT; }
    inline bool isWriteBufferBlocked() const  { return block; }
    inline void blockWriteBuffer() { block = true; }
    inline void freeWriteBuffer() { block = false; }
    inline const Char *getName() const { return currentClient->objectName(); }

    inline StatusCode setPortError(StatusCode status)
    {
        lastStatus = status;
        lastErrorStatus = status;
        isLastPortError = true;
        lastStatusTimestamp = currentTimestamp();
        return status;
    }

    inline StatusCode setPortStatus(StatusCode status)
    {
        if (StatusIsBad(status))
            return setPortError(status);
        lastStatus = status;
        lastStatusTimestamp = currentTimestamp();
        return status;
    }

    inline StatusCode setError(StatusCode status, const String &text)
    {
        lastStatus      = status;
        lastErrorStatus = status;
        lastErrorText   = text;
        isLastPortError = false;
        lastStatusTimestamp = currentTimestamp();
        return status;
    }

    inline StatusCode setError(StatusCode status, String &&text)
    {
        lastStatus      = status;
        lastErrorStatus = status;
        lastErrorText   = text;
        isLastPortError = false;
        lastStatusTimestamp = currentTimestamp();
        return status;
    }

    inline StatusCode setGoodStatus()
    {
        lastStatus = Modbus::Status_Good;
        return lastStatus;
    }

public:
    ModbusPort *port;
    State state;
    uint8_t unit;
    uint8_t func;
    union
    {
    uint16_t offset;
    uint16_t subfunc;
    };
    union
    {
    uint16_t count;
    uint16_t value;
    uint16_t andMask;
    };
    union
    {
    uint16_t orMask;
    };
    bool block;
    ModbusObject *currentClient;
    uint32_t repeats;
    uint8_t buff[MB_VALUE_BUFF_SZ];
    StatusCode lastStatus;
    StatusCode lastErrorStatus;
    String lastErrorText;
    uint32_t lastTries;
    bool isLastPortError;
    Timer timestamp;
    Timestamp lastStatusTimestamp;

    struct
    {
        uint32_t tries;
        bool broadcastEnabled;
    } settings;

};

#define SET_ERROR(status, text) { d->setError(status, text); signalError(d->getName(), status, text); }
#define RAISE_ERROR(status, text) { SET_ERROR(status, text) return status; }
#define SET_COMPLETED(status) { d->lastStatus = status; signalCompleted(d->getName(), status); d->currentClient = nullptr; }
#define RAISE_COMPLETED(status) { SET_COMPLETED(status) return status; }
#define RAISE_ERROR_COMPLETED(status, text) { SET_ERROR(status, text) SET_COMPLETED(status) return status; }

#define SET_PORT_ERROR(status) { d->setPortError(status); signalError(d->getName(), status, d->port->lastErrorText()); }
#define RAISE_PORT_ERROR(status) { SET_PORT_ERROR(status) return status; }
#define RAISE_PORT_ERROR_COMPLETED(status) { SET_PORT_ERROR(status) SET_COMPLETED(status) return status; }

#endif // MODBUSCLIENTPORT_P_H
