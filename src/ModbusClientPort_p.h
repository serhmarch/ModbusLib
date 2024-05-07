#ifndef MODBUSCLIENTPORT_P_H
#define MODBUSCLIENTPORT_P_H

#include "ModbusObject_p.h"

class ModbusPort;

namespace ModbusClientPortPrivateNS {

enum State
{
    STATE_BEGIN = 0,
    STATE_UNKNOWN = STATE_BEGIN,
    STATE_WAIT_FOR_OPEN,
    STATE_OPENED,
    STATE_BEGIN_WRITE,
    STATE_WRITE,
    STATE_BEGIN_READ,
    STATE_READ,
    STATE_WAIT_FOR_CLOSE,
    STATE_CLOSED,
    STATE_END = STATE_CLOSED
};

} // namespace ModbusClientPortPrivateNS

using namespace ModbusClientPortPrivateNS;

class ModbusClientPortPrivate : public ModbusObjectPrivate
{
public:
    ModbusPort *port;
    State state;
    void *currentClient;
    uint32_t repeats;
    StatusCode lastStatus;

    struct
    {
        uint32_t repeatCount;
    } settings;

public:
    inline void setStatus(StatusCode s) { this->lastStatus = s; }
};

inline ModbusClientPortPrivate *d_ModbusClientPort(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusClientPortPrivate*>(d_ptr); }

#endif // MODBUSCLIENTPORT_P_H
