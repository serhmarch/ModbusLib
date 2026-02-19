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
#include "ModbusNetPort.h"
#include "ModbusNetPort_p.h"

inline ModbusNetPortPrivate *d_cast(ModbusPortPrivate *d_ptr) { return static_cast<ModbusNetPortPrivate*>(d_ptr); }

const Char *ModbusNetPort::host() const
{
    return d_cast(d_ptr)->host().data();
}

void ModbusNetPort::setHost(const Char *host)
{
    ModbusNetPortPrivate *d = d_cast(d_ptr);
    if (d->host() != host)
    {
        d->settings.host = host;
        d->setChanged(true);
    }
}

uint16_t ModbusNetPort::port() const
{
    return d_cast(d_ptr)->port();
}

void ModbusNetPort::setPort(uint16_t port)
{
    ModbusNetPortPrivate *d = d_cast(d_ptr);
    if (d->port() != port)
    {
        d->settings.port = port;
        d->setChanged(true);
    }
}