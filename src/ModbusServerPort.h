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
#ifndef MODBUSSERVERPORT_H
#define MODBUSSERVERPORT_H

#include "Modbus.h"

namespace Modbus {

class MODBUS_EXPORT ServerPort
{
public:
    enum State
    {
        STATE_BEGIN                 = 0,
        STATE_UNKNOWN               = STATE_BEGIN,
        STATE_WAIT_FOR_OPEN         ,
        STATE_OPENED                ,
        STATE_BEGIN_READ            ,
        STATE_READ                  ,
        STATE_PROCESS_DEVICE        ,
        STATE_WRITE                 ,
        STATE_BEGIN_WRITE           ,
        STATE_WAIT_FOR_CLOSE        ,
        STATE_CLOSED                ,
        STATE_END                   = STATE_CLOSED
    };
    
public:
    explicit ServerPort(Interface *device);
    virtual ~ServerPort();

public:
    inline Interface *device() const { return m_device; }

public: // server port interface
    virtual Type type() const = 0;
    virtual StatusCode open() = 0;
    virtual StatusCode close() = 0;
    virtual bool isOpen() const = 0;
    virtual StatusCode process() = 0;

    // state
    inline State state() const { return m_state; }
    inline bool isStateClosed() const { return m_state == STATE_CLOSED; }

protected:
    State m_state;
    bool m_cmdClose;
    Interface *m_device;
};

} // namespace Modbus

#endif // MODBUSSERVERPORT_H

