#ifndef MODBUSTCPSERVER_P_UNIX_H
#define MODBUSTCPSERVER_P_UNIX_H

#include "../ModbusTcpServer_p.h"
#include "ModbusTCP_unix.h"

namespace Modbus {

namespace ModbusTcpServerPrivateNS {

typedef std::list<ModbusServerPort*> Connections_t;

} // namespace ModbusTcpServerPrivateNS

using namespace ModbusTcpServerPrivateNS;

class ModbusTcpServerPrivateUnix : public ModbusTcpServerPrivate
{
public:
    ModbusTcpServerPrivateUnix(ModbusInterface *device) :
        ModbusTcpServerPrivate(device)
    {
        this->socket = new ModbusTcpSocket;
    }

    ~ModbusTcpServerPrivateUnix()
    {
        delete this->socket;
    }

public:
    ModbusTcpSocket *socket;
};

inline ModbusTcpServerPrivateUnix *d_unix(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusTcpServerPrivateUnix*>(d_ptr); }

} // namespace Modbus

#endif // MODBUSTCPSERVER_P_WIN_H
