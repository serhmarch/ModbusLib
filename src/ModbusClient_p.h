#ifndef MODBUSCLIENT_P_H
#define MODBUSCLIENT_P_H

#include <string>

#include <string.h>
#include <cstring>

#include "ModbusObject_p.h"

#include "ModbusClientPort.h"

class ModbusClientPrivate : public ModbusObjectPrivate
{
public:
    uint8_t unit;
    ModbusClientPort *port;
#ifndef MB_CLIENT_REPEAT_DISABLE
    uint32_t innerTries;
    struct
    {
        uint32_t tries;
    } settings;
#endif // MB_CLIENT_REPEAT_DISABLE
};

#endif // MODBUSCLIENT_P_H
