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
#include "ModbusServerPort.h"
#include "ModbusServerPort_p.h"

ModbusInterface *ModbusServerPort::device() const
{
    return d_ModbusServerPort(d_ptr)->device;
}

bool ModbusServerPort::isStateClosed() const
{
    return d_ModbusServerPort(d_ptr)->isStateClosed();
}
