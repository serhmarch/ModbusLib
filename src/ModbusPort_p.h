#ifndef MODBUSPORT_P_H
#define MODBUSPORT_P_H

#include "ModbusObject_p.h"

class ModbusPort;

namespace ModbusPortPrivateNS {

enum State
{
    STATE_BEGIN = 0,
    STATE_UNKNOWN = STATE_BEGIN,
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

class ModbusPortPrivate : public ModbusObjectPrivate
{
public:
    ModbusPortPrivate(bool blocking)
    {
        this->state = STATE_UNKNOWN;
        this->block = blocking;;
        this->unit = 0;
        this->func = 0;
        this->modeServer = false;
        this->modeSynch = blocking;
        this->clearChanged();
    }

public:
    inline bool isBlocking() const { return block; }
    inline bool isStateClosed() const { return state == STATE_CLOSED; }
    inline void setChanged(bool changed) { this->changed = changed; }
    inline void clearChanged() { setChanged(false); }
    inline StatusCode setError(StatusCode status, const String &text) { lastErrorText = text; return status; }
    inline StatusCode setError(StatusCode status, String &&text) { lastErrorText = text; return status; }

public:
    State state;
    bool block;
    uint8_t unit;
    uint8_t func;
    bool changed;
    bool modeServer;
    bool modeSynch;
    String lastErrorText;
};

inline ModbusPortPrivate *d_ModbusPort(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusPortPrivate*>(d_ptr); }

#endif // MODBUSPORT_P_H
