#ifndef MODBUSSERVERPORT_P_H
#define MODBUSSERVERPORT_P_H

#include "ModbusObject_p.h"

class ModbusPort;

namespace ModbusServerPortPrivateNS {

enum State
{
    STATE_BEGIN                 = 0,
    STATE_UNKNOWN               = STATE_BEGIN,
    STATE_BEGIN_OPEN            ,
    STATE_WAIT_FOR_OPEN         ,
    STATE_OPENED                ,
    STATE_BEGIN_READ            ,
    STATE_READ                  ,
    STATE_PROCESS_DEVICE        ,
    STATE_WRITE                 ,
    STATE_BEGIN_WRITE           ,
    STATE_WAIT_FOR_CLOSE        ,
    STATE_TIMEOUT               ,
    STATE_CLOSED                ,
    STATE_END                   = STATE_CLOSED
};

} // namespace ModbusServerPortPrivateNS

using namespace ModbusServerPortPrivateNS;

class ModbusServerPortPrivate : public ModbusObjectPrivate
{
public:
    ModbusServerPortPrivate(ModbusInterface *device)
    {
        this->device = device;
        this->state = STATE_UNKNOWN;
        this->cmdClose = false;
        this->timestamp = 0;
        this->context = nullptr;
        this->settings.broadcastEnabled = true;
    }

public: // state
    inline bool isBroadcastEnabled() const { return settings.broadcastEnabled; }
    inline void setBroadcastEnabled(bool enable) { settings.broadcastEnabled = enable; }
    inline void timestampRefresh() { timestamp = timer(); }
    inline bool isStateClosed() const { return state == STATE_CLOSED || state == STATE_TIMEOUT; }
    inline const Char *getName() const { return objectName.data(); }
    inline StatusCode lastErrorStatus() const { return errorStatus; }
    inline const Char *lastErrorText() const { return errorText.data(); }
    inline StatusCode setErrorBase(StatusCode status, const Char *text)
    {
        errorStatus = status;
        errorText = text;
        return status;
    }

public:
    ModbusInterface *device;
    State state;
    bool cmdClose;
    StatusCode errorStatus;
    String errorText;
    Timer timestamp;
    void *context;
    struct
    {
        bool broadcastEnabled;
    } settings;
};

inline ModbusServerPortPrivate *d_ModbusServerPort(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusServerPortPrivate*>(d_ptr); }

#endif // MODBUSSERVERPORT_P_H
