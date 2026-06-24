#ifndef MODBUSTCPPORT_P_H
#define MODBUSTCPPORT_P_H

#include "ModbusPort_p.h"
#include "ModbusTcpPort.h"

#define MBCLIENTTCP_BUFF_SZ MB_TCP_IO_BUFF_SZ

class ModbusTcpPortPrivate : public ModbusPortPrivate
{
public:
    ModbusTcpPortPrivate(bool blocking) :
        ModbusPortPrivate(blocking)
    {
        const ModbusTcpPort::Defaults &d = ModbusTcpPort::Defaults::instance();

        settings.host                 = d.host   ;
        settings.port                 = d.port   ;
        settingsBase.timeout          = d.timeout;
        settingsBase.timeoutInterByte = 0        ;

        timestamp = 0;
        autoIncrement = true;
        transaction = 0;
        sz = 0;
    }

public: // settings
    inline const String& host() const { return settings.host; }
    inline void setHost(const String& host) { settings.host = host; }
    inline void setHost(String &&host) { settings.host = std::move(host); }
    inline void setHost(const Char *host) { settings.host = host; }

    inline uint16_t port() const { return settings.port; }
    inline void setPort(uint16_t port) { settings.port = port; }

    inline void timestampRefresh() { timestamp = timer(); }
public:
    struct
    {
        String   host;
        uint16_t port;
    } settings;
    Timer timestamp;
    bool autoIncrement;
    uint16_t transaction;
    uint8_t  buff[MBCLIENTTCP_BUFF_SZ];
    uint16_t sz;
};

#endif // MODBUSTCPPORT_P_H
