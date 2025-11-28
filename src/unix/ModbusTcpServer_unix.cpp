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
#include "../ModbusTcpServer.h"

#include "ModbusTcpServer_p_unix.h"

ModbusTcpServer::ModbusTcpServer(ModbusInterface *device) :
    ModbusServerPort(new ModbusTcpServerPrivateUnix(device))
{
}

StatusCode ModbusTcpServer::open()
{
    ModbusTcpServerPrivateUnix *d = d_unix(d_ptr);
    d->cmdClose = false;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
        {
            if (isOpen())
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }

            d->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (d->socket->isInvalid())
            {
                d->state = STATE_CLOSED;
                return d->setErrorBase(Status_BadTcpCreate, (StringLiteral("TCP. Socket creation error for port '") + toModbusString(d->tcpPort) +
                                                             StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                             StringLiteral(". ") + getLastErrorText()).data());
            }

            // This prevents the "Address already in use" error that typically occurs when trying to
            // bind to a recently closed socket.
            int opt = 1;
            setsockopt(d->socket->socket(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            // Bind the socket
            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = inet_addr(d->ipaddr.data());
            if (serverAddr.sin_addr.s_addr == INADDR_NONE)
                serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any available interface
            serverAddr.sin_port = htons(d->tcpPort); // Port number

            if (d->socket->bind((sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            {
                d->socket->close();
                d->state = STATE_CLOSED;
                return d->setErrorBase(Status_BadTcpBind, (StringLiteral("TCP. Bind error for port '") + toModbusString(d->tcpPort) +
                                                           StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                           StringLiteral(". ") + getLastErrorText()).data());
            }

            // Listen on the socket
            //if (d->socket->listen(SOMAXCONN) == SOCKET_ERROR)
            if (d->socket->listen(d->maxconn) == SOCKET_ERROR)
            {
                d->socket->close();
                d->state = STATE_CLOSED;
                return d->setErrorBase(Status_BadTcpListen, (StringLiteral("TCP. Listen error for port '") + toModbusString(d->tcpPort) +
                                                             StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                             StringLiteral(". ") + getLastErrorText()).data());
            }
            d->socket->setBlocking(false);
        }
            return Status_Good;
        default:
            if (!isOpen())
            {
                d->state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusTcpServer::close()
{
    ModbusTcpServerPrivateUnix *d = d_unix(d_ptr);
    if (isOpen())
        d->socket->close();
    d->cmdClose = true;
    for (ModbusServerPort *c : d->connections)
        c->close();
    switch (d->state)
    {
    case STATE_WAIT_FOR_CLOSE:
        for (ModbusServerPort *c : d->connections)
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

bool ModbusTcpServer::isOpen() const
{
    return d_unix(d_ptr)->socket->isValid();
}

ModbusTcpSocket *ModbusTcpServer::nextPendingConnection()
{
    ModbusTcpServerPrivateUnix *d = d_unix(d_ptr);
    // Accept the incoming connection
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = d->socket->accept((sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET)
    {
        if (errno != EWOULDBLOCK)
        {
            d->socket->close();
            d->state = STATE_CLOSED;
        }
        return nullptr;
    }
    if (d->connections.size() >= d->maxconn)
    {
        ::close(clientSocket);
        return nullptr;
    }

    ModbusTcpSocket *tcp = new ModbusTcpSocket(clientSocket);
    return tcp;
}

bool ModbusTcpServerPrivate::getHostService(ModbusTcpSocket *socket, String &host, String &service)
{
    sockaddr_storage clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = socket->socket();
    if (getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize) == 0)
    {
        char ipStr[INET6_ADDRSTRLEN] = {};
        uint16_t port = 0;

        if (clientAddr.ss_family == AF_INET)
        {
            sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(&clientAddr);
            inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, sizeof(ipStr));
            port = ntohs(ipv4->sin_port);
            host = ipStr;
            service = std::to_string(port);
            return true;
        }
        else if (clientAddr.ss_family == AF_INET6)
        {
            sockaddr_in6* ipv6 = reinterpret_cast<sockaddr_in6*>(&clientAddr);
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ipStr, sizeof(ipStr));
            port = ntohs(ipv6->sin6_port);
            host = ipStr;
            service = std::to_string(port);
            return true;
        }
    }
    return false;
}

