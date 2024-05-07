#ifndef MODBUSSERVERRESOURCE_P_H
#define MODBUSSERVERRESOURCE_P_H

#include "ModbusPort.h"
#include "ModbusServerPort_p.h"

#define MBSERVER_SZ_VALUE_BUFF MB_VALUE_BUFF_SZ

class ModbusServerResourcePrivate : public ModbusServerPortPrivate
{
public:
    ModbusServerResourcePrivate(ModbusPort *port, ModbusInterface *device) :
        ModbusServerPortPrivate(device)
    {
        port->setServerMode(true);
        this->port = port;
    }

public:
    ModbusPort *port;
    uint8_t unit;
    uint8_t func;
    uint16_t offset;
    uint16_t count;
    uint8_t valueBuff[MBSERVER_SZ_VALUE_BUFF];

};

inline ModbusServerResourcePrivate *d_ModbusServerResource(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusServerResourcePrivate*>(d_ptr); }

#endif // MODBUSSERVERRESOURCE_P_H
