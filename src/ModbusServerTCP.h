/*
    Modbus

    Created: 2023
    Author: Serhii Marchuk, https://github.com/serhmarch

    Copyright (C) 2023  Serhii Marchuk

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef MODBUSSERVERTCP_H
#define MODBUSSERVERTCP_H

#include <list>

#include "ModbusServerPort.h"

namespace Modbus {

class TCPSocket;

class MODBUS_EXPORT ServerTCP : public ServerPort
{
public:
    struct MODBUS_EXPORT Defaults
    {
        const uint16_t port   ;
        const uint32_t timeout;

        Defaults();
        static const Defaults &instance();
    };

public:
    ServerTCP(Interface *device);
    ~ServerTCP();

public:
    Modbus::Type type() const override{ return Modbus::TCP; }
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;
    StatusCode process() override;
    
public:
    virtual ServerPort *createPortTCP(TCPSocket *socket);
    
public:
    inline uint16_t port() const { return m_tcpPort; }
    inline void setPort(uint16_t port) { m_tcpPort = port; }
    inline int timeout() const { return m_timeout; }
    inline void setTimeout(int timeout) { m_timeout = timeout; }

private:
    TCPSocket *nextPendingConnection();
    void clearConnections();

private:
    typedef std::list<ServerPort*> Connections_t;
    uint16_t m_tcpPort;
    uint32_t m_timeout;
    Connections_t m_connections;

private:
    void constructorPrivate();
    void destructorPrivate();

private: // Platform specific data
    struct PlatformData;
    PlatformData *m_platformData;
};

} // namespace Modbus

#endif // MODBUSSERVERTCP_H
