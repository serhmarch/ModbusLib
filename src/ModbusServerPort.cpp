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

inline ModbusServerPortPrivate *d_cast(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusServerPortPrivate*>(d_ptr); }

ModbusInterface *ModbusServerPort::device() const
{
    return d_cast(d_ptr)->device;
}

void ModbusServerPort::setDevice(ModbusInterface *device)
{
    d_cast(d_ptr)->device = device;
}

bool ModbusServerPort::isTcpServer() const
{
    return false;
}

bool ModbusServerPort::isBroadcastEnabled() const
{
    return d_cast(d_ptr)->isBroadcastEnabled();
}

void ModbusServerPort::setBroadcastEnabled(bool enable)
{
    d_cast(d_ptr)->setBroadcastEnabled(enable);
}

const void *ModbusServerPort::unitMap() const
{
    return d_cast(d_ptr)->unitMap();
}

void ModbusServerPort::setUnitMap(const void *unitmap)
{
    d_cast(d_ptr)->setUnitMap(unitmap);
}

Modbus::String ModbusServerPort::unitMapString() const
{
    if (const void *unitmap = this->unitMap())
        return Modbus::unitMapToString(unitmap);
    return Modbus::String();
}

void ModbusServerPort::setUnitMapString(const Modbus::Char *s)
{
    if (s == nullptr || *s == '\0')
    {
        this->setUnitMap(nullptr);
        return;
    }
    uint8_t unitmap[MB_UNITMAP_SIZE];
    memset(unitmap, 0, MB_UNITMAP_SIZE);
    if (fillUnitMap(s, unitmap))
        this->setUnitMap(unitmap);
}

bool ModbusServerPort::isUnitEnabled(uint8_t unit) const
{
    return d_cast(d_ptr)->isUnitEnabled(unit);
}

void ModbusServerPort::setUnitEnabled(uint8_t unit, bool enable)
{
    d_cast(d_ptr)->setUnitEnabled(unit, enable);
}

void *ModbusServerPort::context() const
{
    return d_cast(d_ptr)->context;
}

void ModbusServerPort::setContext(void *context)
{
    d_cast(d_ptr)->context = context;
}

StatusCode ModbusServerPort::lastStatus() const
{
    return d_cast(d_ptr)->lastStatus;
}

Modbus::Timestamp ModbusServerPort::lastStatusTimestamp() const
{
    return d_cast(d_ptr)->lastStatusTimestamp;
}

Modbus::StatusCode ModbusServerPort::lastErrorStatus() const
{
    return d_cast(d_ptr)->lastErrorStatus;
}

const Char *ModbusServerPort::lastErrorText() const
{
    return d_cast(d_ptr)->lastErrorText.data();
}

bool ModbusServerPort::isStateClosed() const
{
    return d_cast(d_ptr)->isStateClosed();
}

void ModbusServerPort::signalOpened(const Modbus::Char *source)
{
    emitSignal(__func__, &ModbusServerPort::signalOpened, source);
}

void ModbusServerPort::signalClosed(const Modbus::Char *source)
{
    emitSignal(__func__, &ModbusServerPort::signalClosed, source);
}

void ModbusServerPort::signalTx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
{
    emitSignal(__func__, &ModbusServerPort::signalTx, source, buff, size);
}

void ModbusServerPort::signalRx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
{
    emitSignal(__func__, &ModbusServerPort::signalRx, source, buff, size);
}

void ModbusServerPort::signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text)
{
    emitSignal(__func__, &ModbusServerPort::signalError, source, status, text);
}

void ModbusServerPort::signalCompleted(const Modbus::Char *source, Modbus::StatusCode status)
{
    emitSignal(__func__, &ModbusServerPort::signalCompleted, source, status);
}