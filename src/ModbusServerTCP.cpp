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
#include "ModbusServerTCP.h"

namespace Modbus {

ServerTCP::Defaults::Defaults() :
    port   (STANDARD_TCP_PORT),
    timeout(3000)
{
}

const ServerTCP::Defaults &ServerTCP::Defaults::instance()
{
    static const Defaults d;
    return d;
}

ServerTCP::ServerTCP(Interface *device) :
    ServerPort(device)
{
    m_tcpPort = STANDARD_TCP_PORT;
    m_timeout = 3000;
    constructorPrivate();
}

ServerTCP::~ServerTCP()
{
    destructorPrivate();
}

StatusCode ServerTCP::process()
{
    StatusCode r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (m_state)
        {
        case STATE_CLOSED:
            //if (m_cmdClose)
            //    break;
            //m_state = STATE_WAIT_FOR_OPEN;
            // no need break
        case STATE_WAIT_FOR_OPEN:
            if (m_cmdClose)
            {
                m_state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            r = open();
            if (r) // if not OK it's mean that an error occured or in process
                return r;
            m_state = STATE_OPENED;
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = close();
            if (r) // if not OK it's mean that an error occured or in process
                return r;
            m_state = STATE_CLOSED;
            clearConnections();
            //setMessage("Finalized");
            break;
        case STATE_OPENED:
            ////setMessage("Initialized. Waiting for connections...");
            m_state = STATE_PROCESS_DEVICE;
            // no need break
        case STATE_PROCESS_DEVICE:
        {
            if (m_cmdClose)
            {
                m_state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            // check up new connection
            if (TCPSocket *s = this->nextPendingConnection())
            {
                ServerPort *c = createPortTCP(s);
                m_connections.push_back(c);
                //connect(c, &ServerPort::signalTx     , this, &ServerPort::signalTx     );
                //connect(c, &ServerPort::signalRx     , this, &ServerPort::signalRx     );
                //connect(c, &ServerPort::signalError  , this, &ServerPort::signalError  );
                //connect(c, &ServerPort::signalMessage, this, &ServerPort::signalMessage);
                //setMessage(QString("New connection from '%1'").arg(c->name()));
            }
            // process current connections
            for (Connections_t::iterator it = m_connections.begin(); it != m_connections.end(); )
            {
                ServerPort *c = *it;
                c->process();
                if (!c->isOpen())
                {
                    //setMessage(QString("Close connection from '%1'").arg(c->name()));
                    it = m_connections.erase(it);
                    delete c;
                    continue;
                }
                it++;
            }
        }
            break;
        default:
            if (m_cmdClose && isOpen())
            {
                m_state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
            }
            else if (isOpen())
            {
                m_state = STATE_OPENED;
                fRepeatAgain = true;
            }
            else
                m_state = STATE_CLOSED;
            break;
        }
    }
    while(fRepeatAgain);
    return Status_Processing;
}

void ServerTCP::clearConnections()
{
    for (auto& elem : m_connections)
        delete elem;
    m_connections.clear();

}

} // namespace Modbus
