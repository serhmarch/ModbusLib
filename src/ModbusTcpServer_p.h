#ifndef MODBUSTCPSERVER_P_H
#define MODBUSTCPSERVER_P_H

#include <list>

#include "ModbusTcpServer.h"
#include "ModbusServerPort_p.h"

namespace ModbusTcpServerPrivateNS {

typedef std::list<ModbusServerPort*> Connections_t;

} // namespace ModbusTcpServerPrivateNS

using namespace ModbusTcpServerPrivateNS;

class ModbusTcpServerPrivate : public ModbusServerPortPrivate
{
public:
    static bool getHostService(ModbusSocket *socket, String &host, String &service);

public:
    ModbusTcpServerPrivate(Modbus::ProtocolType type, ModbusInterface *device) :
        ModbusServerPortPrivate(device)
    {
        const ModbusTcpServer::Defaults &d = ModbusTcpServer::Defaults::instance();

        switch(type)
        {
        case Modbus::ASCvTCP:
        case Modbus::RTUvTCP:
            this->type = type;
            break;
        default:
            this->type = Modbus::TCP;
            break;
        }

        this->tcpPort = d.port   ;
        this->timeout = d.timeout;
        this->maxconn = d.maxconn;
    }

public:
    Modbus::ProtocolType type;
    uint16_t tcpPort;
    uint32_t timeout;
    uint32_t maxconn;
    Connections_t connections;
};

inline ModbusTcpServerPrivate *d_ModbusTcpServer(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusTcpServerPrivate*>(d_ptr); }

#endif // MODBUSTCPSERVER_P_H
