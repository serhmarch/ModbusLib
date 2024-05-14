#ifndef MODBUSCLIENTPORT_P_H
#define MODBUSCLIENTPORT_P_H

#include "ModbusObject_p.h"

#include "ModbusObject.h"
#include "ModbusPort.h"

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
    ModbusClientPortPrivate(ModbusPort *port)
    {
        this->state = STATE_UNKNOWN;
        this->currentClient = nullptr;
        this->port = port;
        this->repeats = 0;
        this->settings.repeatCount = 1;

        port->setServerMode(false);
    }

public:
    const Char *getName() const { return currentClient->objectName(); }
    inline void setStatus(StatusCode s) { this->lastStatus = s; }

public:
    ModbusPort *port;
    State state;
    ModbusObject *currentClient;
    uint32_t repeats;
    StatusCode lastStatus;

    struct
    {
        uint32_t repeatCount;
    } settings;

};

inline ModbusClientPortPrivate *d_ModbusClientPort(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusClientPortPrivate*>(d_ptr); }

#endif // MODBUSCLIENTPORT_P_H
