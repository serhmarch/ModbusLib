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
#include "ModbusTcpServer.h"
#include "ModbusTcpServer_p.h"

#include "ModbusTcpPort.h"
#include "ModbusAscOverTcpPort.h"
#include "ModbusRtuOverTcpPort.h"
#include "ModbusServerResource.h"

#define MODBUS_TCPSERVER_OPEN_TIMOUT_ms 1000

inline ModbusTcpServerPrivate *d_cast(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusTcpServerPrivate*>(d_ptr); }

ModbusTcpServer::Defaults::Defaults() :
    ipaddr (StringLiteral("0.0.0.0")),
    port   (STANDARD_TCP_PORT),
    timeout(3000),
    maxconn(10)
{
}

const ModbusTcpServer::Defaults &ModbusTcpServer::Defaults::instance()
{
    static const Defaults d;
    return d;
}

ModbusTcpServer::~ModbusTcpServer()
{
    clearConnections();
}

const Modbus::Char *ModbusTcpServer::ipaddr() const
{
    return d_cast(d_ptr)->ipaddr.data();
}

void ModbusTcpServer::setIpaddr(const Modbus::Char *ipaddr)
{
    d_cast(d_ptr)->ipaddr = ipaddr;
}

uint16_t ModbusTcpServer::port() const
{
    return d_cast(d_ptr)->tcpPort;
}

void ModbusTcpServer::setPort(uint16_t port)
{
    d_cast(d_ptr)->tcpPort = port;
}

uint32_t ModbusTcpServer::timeout() const
{
    return d_cast(d_ptr)->timeout;
}

void ModbusTcpServer::setTimeout(uint32_t timeout)
{
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    d->timeout = timeout;
    for (auto& c : d->connections)
        c->setTimeout(timeout);
}

uint32_t ModbusTcpServer::maxConnections() const
{
    return d_cast(d_ptr)->maxconn;
}

void ModbusTcpServer::setMaxConnections(uint32_t maxconn)
{
    if (maxconn)
        d_cast(d_ptr)->maxconn = maxconn;
    else
        d_cast(d_ptr)->maxconn = 1;
}

ProtocolType ModbusTcpServer::type() const
{
    return d_cast(d_ptr)->type;
}

void ModbusTcpServer::setBroadcastEnabled(bool enable)
{
    ModbusServerPort::setBroadcastEnabled(enable);
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    for (auto& c : d->connections)
        c->setBroadcastEnabled(enable);
}

void ModbusTcpServer::setUnitMap(const void *unitmap)
{
    ModbusServerPort::setUnitMap(unitmap);
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    for (auto& c : d->connections)
        c->setUnitMap(unitmap);
}

void ModbusTcpServer::setUnitEnabled(uint8_t unit, bool enable)
{
    ModbusServerPort::setUnitEnabled(unit, enable);
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    for (auto& c : d->connections)
        c->setUnitEnabled(unit, enable);
}

void ModbusTcpServer::clearConnections()
{
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    for (auto& c : d->connections)
    {
        signalCloseConnection(c->objectName());
        delete c;
    }
    d->connections.clear();
}

StatusCode ModbusTcpServer::process()
{
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    StatusCode r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_CLOSED:
            if (d->cmdClose)
                break;
            d->state = STATE_BEGIN_OPEN;
            MB_FALLTHROUGH
        case STATE_BEGIN_OPEN:
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_OPEN;
            MB_FALLTHROUGH
        case STATE_WAIT_FOR_OPEN:
            if (d->cmdClose)
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            r = open();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))  // an error occured
            {
                signalError(d->getName(), r, d->lastErrorTextData());
                d->state = STATE_TIMEOUT;
                return r;
            }
            d->state = STATE_OPENED;
            signalOpened(d->getName());
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = close();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))  // an error occured
            {
                signalError(d->getName(), r, d->lastErrorTextData());
            }
            d->state = STATE_CLOSED;
            signalClosed(d->getName());
            clearConnections();
            //setMessage("Finalized");
            break;
        case STATE_OPENED:
            ////setMessage("Initialized. Waiting for connections...");
            d->state = STATE_PROCESS_DEVICE;
            MB_FALLTHROUGH
        case STATE_PROCESS_DEVICE:
        {
            if (d->cmdClose)
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            // check up new connection
            if (ModbusSocket *s = this->nextPendingConnection())
            {
                ModbusPort *p = createModbusPort(s);
                p->setTimeout(timeout());
                ModbusServerResource *c = new ModbusServerResource(p, device());
                String host, service;
                if (ModbusTcpServerPrivate::getHostService(s, host, service))
                {
                    String name = host + StringLiteral(":") + service;
                    c->setObjectName(name.data());
                }
                c->connect(&ModbusServerPort::signalTx       , static_cast<ModbusServerPort*>(this), &ModbusTcpServer::signalTx   );
                c->connect(&ModbusServerPort::signalRx       , static_cast<ModbusServerPort*>(this), &ModbusTcpServer::signalRx   );
                c->connect(&ModbusServerPort::signalError    , this, &ModbusTcpServer::setErrorInner    );
                c->connect(&ModbusServerPort::signalCompleted, this, &ModbusTcpServer::setCompletedInner);
                c->setBroadcastEnabled(isBroadcastEnabled());
                c->setUnitMap(unitMap());
                d->connections.push_back(c);
                signalNewConnection(c->objectName());
            }
            // process current connections
            for (Connections_t::iterator it = d->connections.begin(); it != d->connections.end(); )
            {
                ModbusServerPort *c = *it;
                c->process();
                if (!c->isOpen())
                {
                    signalCloseConnection(c->objectName());
                    it = d->connections.erase(it);
                    delete c;
                    continue;
                }
                ++it;
            }
        }
            break;
        case STATE_TIMEOUT:
            if (timer() - d->timestamp < timeout())
                return Status_Processing;
            d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        default:
            if (d->cmdClose && isOpen())
                d->state = STATE_WAIT_FOR_CLOSE;
            else if (isOpen())
                d->state = STATE_OPENED;
            else
                d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while(fRepeatAgain);
    return Status_Processing;
}

ModbusPort *ModbusTcpServer::createModbusPort(ModbusSocket *socket)
{
    switch (d_cast(d_ptr)->type)
    {
    case Modbus::ASCvTCP:
        return new ModbusAscOverTcpPort(socket);
    case Modbus::RTUvTCP:
        return new ModbusRtuOverTcpPort(socket);
    default:
        return new ModbusTcpPort(socket);
    }
}

void ModbusTcpServer::signalNewConnection(const Modbus::Char *source)
{
    this->emitSignal(__func__, &ModbusTcpServer::signalNewConnection, source);
}

void ModbusTcpServer::signalCloseConnection(const Modbus::Char *source)
{
    this->emitSignal(__func__, &ModbusTcpServer::signalCloseConnection, source);
}

void ModbusTcpServer::setErrorInner(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text)
{
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    d->lastStatus = status;
    d->lastErrorStatus = status;
    d->lastErrorText = text;
    d->lastStatusTimestamp = currentTimestamp();
    signalError(source, status, text);
}

void ModbusTcpServer::setCompletedInner(const Modbus::Char *source, Modbus::StatusCode status)
{
    ModbusTcpServerPrivate *d = d_cast(d_ptr);
    d->lastStatus = status;
    if (!StatusIsBad(status)) // Note: timestamp must be updated in setErrorInner function
        d->lastStatusTimestamp = currentTimestamp();
    signalCompleted(source, status);

}

