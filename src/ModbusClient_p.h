#ifndef MODBUSCLIENT_P_H
#define MODBUSCLIENT_P_H

#include <string>

#include "ModbusObject_p.h"

#include "ModbusClientPort.h"

class ModbusClientPrivate : public ModbusObjectPrivate
{
public:
    uint8_t unit;
    ModbusClientPort *port;
    ModbusClientPort::RequestParams *rp;
    uint8_t buff[MB_VALUE_BUFF_SZ];
    Modbus::StatusCode lastStatus;
    Modbus::StatusCode lastErrorStatus;
    Modbus::String lastErrorText;
};

inline ModbusClientPrivate *d_ModbusClient(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusClientPrivate*>(d_ptr); }

#endif // MODBUSCLIENT_P_H
