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
#include "ModbusServerResource.h"

ModbusTcpServer::Defaults::Defaults() :
    port   (STANDARD_TCP_PORT),
    timeout(3000)
{
}

const ModbusTcpServer::Defaults &ModbusTcpServer::Defaults::instance()
{
    static const Defaults d;
    return d;
}

uint16_t ModbusTcpServer::port() const
{
    return d_ModbusTcpServer(d_ptr)->tcpPort;
}

void ModbusTcpServer::setPort(uint16_t port)
{
    d_ModbusTcpServer(d_ptr)->tcpPort = port;
}

int ModbusTcpServer::timeout() const
{
    return d_ModbusTcpServer(d_ptr)->timeout;
}

void ModbusTcpServer::setTimeout(int timeout)
{
    d_ModbusTcpServer(d_ptr)->timeout = timeout;
}

void ModbusTcpServer::clearConnections()
{
    ModbusTcpServerPrivate *d = d_ModbusTcpServer(d_ptr);
    for (auto& elem : d->connections)
        delete elem;
    d->connections.clear();

}

StatusCode ModbusTcpServer::process()
{
    ModbusTcpServerPrivate *d = d_ModbusTcpServer(d_ptr);
    StatusCode r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_CLOSED:
            //if (d->cmdClose)
            //    break;
            //d->state = STATE_WAIT_FOR_OPEN;
            // no need break
        case STATE_WAIT_FOR_OPEN:
            if (d->cmdClose)
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            r = open();
            if (r) // if not OK it's mean that an error occured or in process
                return r;
            d->state = STATE_OPENED;
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = close();
            if (r) // if not OK it's mean that an error occured or in process
                return r;
            d->state = STATE_CLOSED;
            clearConnections();
            //setMessage("Finalized");
            break;
        case STATE_OPENED:
            ////setMessage("Initialized. Waiting for connections...");
            d->state = STATE_PROCESS_DEVICE;
            // no need break
        case STATE_PROCESS_DEVICE:
        {
            if (d->cmdClose)
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            // check up new connection
            if (ModbusTcpSocket *s = this->nextPendingConnection())
            {
                ModbusServerPort *c = createTcpPort(s);
                d->connections.push_back(c);
                //connect(c, &ModbusServerPort::signalTx     , this, &ModbusServerPort::signalTx     );
                //connect(c, &ModbusServerPort::signalRx     , this, &ModbusServerPort::signalRx     );
                //connect(c, &ModbusServerPort::signalError  , this, &ModbusServerPort::signalError  );
                //connect(c, &ModbusServerPort::signalMessage, this, &ModbusServerPort::signalMessage);
                //setMessage(QString("New connection from '%1'").arg(c->name()));
            }
            // process current connections
            for (Connections_t::iterator it = d->connections.begin(); it != d->connections.end(); )
            {
                ModbusServerPort *c = *it;
                c->process();
                if (!c->isOpen())
                {
                    //setMessage(QString("Close connection from '%1'").arg(c->name()));
                    it = d->connections.erase(it);
                    delete c;
                    continue;
                }
                it++;
            }
        }
            break;
        default:
            if (d->cmdClose && isOpen())
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
            }
            else if (isOpen())
            {
                d->state = STATE_OPENED;
                fRepeatAgain = true;
            }
            else
                d->state = STATE_CLOSED;
            break;
        }
    }
    while(fRepeatAgain);
    return Status_Processing;
}

ModbusServerPort *ModbusTcpServer::createTcpPort(ModbusTcpSocket *socket)
{
    ModbusTcpPort *tcp = new ModbusTcpPort(socket);
    tcp->connect(&ModbusTcpPort::emitTx, this, &ModbusTcpServer::emitTx);
    tcp->connect(&ModbusTcpPort::emitRx, this, &ModbusTcpServer::emitRx);
    //connect(c, &ModbusServerPort::signalError  , this, &ModbusServerPort::signalError  );
    //connect(c, &ModbusServerPort::signalMessage, this, &ModbusServerPort::signalMessage);
    ModbusServerResource *port = new ModbusServerResource(tcp, device());
    //port->setName(socket->localAddress().toString());
    return port;
}

void ModbusTcpServer::emitTx(const uint8_t *buff, uint16_t size)
{
    this->emitSignal(&ModbusTcpServer::emitTx, buff, size);
}

void ModbusTcpServer::emitRx(const uint8_t *buff, uint16_t size)
{
    this->emitSignal(&ModbusTcpServer::emitRx, buff, size);
}
