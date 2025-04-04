#ifndef MODBUSSERVERPORT_P_H
#define MODBUSSERVERPORT_P_H

#include "ModbusObject_p.h"

class ModbusPort;

#define MB_UNITS_MAP_SIZE 32

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
        this->settings.unitsmap = nullptr;
    }

    ~ModbusServerPortPrivate() override
    {
        if (settings.unitsmap)
            free(settings.unitsmap);
    }

public: // state
    inline bool isBroadcastEnabled() const { return settings.broadcastEnabled; }
    inline void setBroadcastEnabled(bool enable) { settings.broadcastEnabled = enable; }
    inline const void *unitsMap() const { return settings.unitsmap; }
    inline void setUnitsMap(const void *unitsmap)
    {
        if (settings.unitsmap == unitsmap)
            return;
        if (settings.unitsmap)
            free(settings.unitsmap); 
        if (unitsmap)
        {
            settings.unitsmap = reinterpret_cast<uint8_t*>(malloc(MB_UNITS_MAP_SIZE));
            memcpy(settings.unitsmap, unitsmap, MB_UNITS_MAP_SIZE);
        }
        else
            settings.unitsmap = nullptr;
    }

    inline bool isUnitEnabled(uint8_t unit) const
    {
        if (settings.unitsmap == nullptr)
            return true;
        return ((settings.unitsmap[unit/8]) & (1 << (unit%8))) != 0;
    }

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
        uint8_t *unitsmap;
    } settings;
};

inline ModbusServerPortPrivate *d_ModbusServerPort(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusServerPortPrivate*>(d_ptr); }

#endif // MODBUSSERVERPORT_P_H
