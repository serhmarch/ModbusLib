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
#include "../ModbusServerTCP.h"

#include "../ModbusServerResource.h"
#include "../ModbusPortTCP.h"
#include "ModbusTCP_win.h"

namespace Modbus {

struct ServerTCP::PlatformData
{
    TCPSocket *socket;
};


void ServerTCP::constructorPrivate()
{
    WSADATA data;
    WSAStartup(0x202, &data);

    m_platformData = new PlatformData;
    m_platformData->socket = new TCPSocket;
}

void ServerTCP::destructorPrivate()
{
    delete m_platformData->socket;
    delete m_platformData;
    WSACleanup();
}

StatusCode ServerTCP::open()
{
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (m_state)
        {
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
        {
            if (isOpen())
            {
                m_state = STATE_OPENED;
                return Status_Good;
            }

            m_platformData->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (m_platformData->socket->isInvalid())
            {
                m_state = STATE_CLOSED;
                return Status_BadTcpCreate;
            }

            // Bind the socket
            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any available interface
            serverAddr.sin_port = htons(m_tcpPort); // Port number

            if (m_platformData->socket->bind((sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            {
                m_platformData->socket->close();
                m_state = STATE_CLOSED;
                return Status_BadTcpBind;
            }

            // Listen on the socket
            if (m_platformData->socket->listen(SOMAXCONN) == SOCKET_ERROR)
            {
                m_platformData->socket->close();
                m_state = STATE_CLOSED;
                return Status_BadTcpListen;
            }
            m_platformData->socket->setBlocking(false);
        }
            return Status_Good;
        default:
            if (!isOpen())
            {
                m_state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ServerTCP::close()
{
    if (isOpen())
        m_platformData->socket->close();
    m_cmdClose = true;
    for (ServerPort *c : m_connections)
        c->close();
    switch (m_state)
    {
    case STATE_WAIT_FOR_CLOSE:
        for (ServerPort *c : m_connections)
        {
            c->process();
            if (!c->isStateClosed())
                return Status_Processing;
        }
        break;
    default:
        return Status_Processing;
    }
    return Status_Good;
}

bool ServerTCP::isOpen() const
{
    return m_platformData->socket->isValid();
}

TCPSocket *ServerTCP::nextPendingConnection()
{
    // Accept the incoming connection
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = m_platformData->socket->accept((sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            m_platformData->socket->close();
            m_state = STATE_CLOSED;
        }
        return nullptr;
    }

    TCPSocket *tcp = new TCPSocket(clientSocket);
    return tcp;
}

ServerPort *ServerTCP::createPortTCP(TCPSocket *socket)
{
    PortTCP *tcp = new PortTCP(socket);
    ServerResource *port = new ServerResource(tcp, device());
    //port->setName(socket->localAddress().toString());
    return port;
}

} // namespace Modbus
