#ifndef MODBUSNETPORT_P_H
#define MODBUSNETPORT_P_H

#include "ModbusPort_p.h"
#include "ModbusNetPort.h"

class ModbusNetPortPrivate : public ModbusPortPrivate
{
public:
    static ModbusNetPortPrivate *create(ModbusFramePrivate *f, bool blocking);

public:
    ModbusNetPortPrivate(ModbusFramePrivate *f, bool blocking) :
        ModbusPortPrivate(f, blocking)
    {
        const Modbus::NetDefaults &d = Modbus::NetDefaults::instance();

        settings.host        = d.host;
        settings.port        = d.port;
        settingsBase.timeout = d.timeout;
    }

public: // settings
    inline const String& host() const { return settings.host; }
    inline uint16_t port() const { return settings.port; }

public:
    struct
    {
        String host;
        uint16_t port;
    } settings;

};

#endif // MODBUSNETPORT_P_H
