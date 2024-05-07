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
#include "ModbusSerialPort.h"
#include "ModbusSerialPort_p.h"

const ModbusSerialPort::Defaults &ModbusSerialPort::Defaults::instance()
{
    static const Defaults d;
    return d;
}

const Char *ModbusSerialPort::portName() const
{
    return d_ModbusSerialPort(d_ptr)->settings.portName.data();
}

void ModbusSerialPort::setPortName(const Char *portName)
{
    if (d_ModbusSerialPort(d_ptr)->settings.portName != portName)
    {
        d_ModbusSerialPort(d_ptr)->settings.portName = portName;
        setChanged(true);
    }
}

int32_t ModbusSerialPort::baudRate() const
{
    return d_ModbusSerialPort(d_ptr)->settings.baudRate;
}

void ModbusSerialPort::setBaudRate(int32_t baudRate)
{
    if (d_ModbusSerialPort(d_ptr)->settings.baudRate != baudRate)
    {
        d_ModbusSerialPort(d_ptr)->settings.baudRate = baudRate;
        setChanged(true);
    }
}

int8_t ModbusSerialPort::dataBits() const
{
    return d_ModbusSerialPort(d_ptr)->settings.dataBits;
}

void ModbusSerialPort::setDataBits(int8_t dataBits)
{
    if (d_ModbusSerialPort(d_ptr)->settings.dataBits != dataBits)
    {
        d_ModbusSerialPort(d_ptr)->settings.dataBits = dataBits;
        setChanged(true);
    }
}

Parity ModbusSerialPort::parity() const
{
    return d_ModbusSerialPort(d_ptr)->settings.parity;
}

void ModbusSerialPort::setStopBits(StopBits stopBits)
{
    if (d_ModbusSerialPort(d_ptr)->settings.stopBits != stopBits)
    {
        d_ModbusSerialPort(d_ptr)->settings.stopBits = stopBits;
        setChanged(true);
    }
}

FlowControl ModbusSerialPort::flowControl() const
{
    return d_ModbusSerialPort(d_ptr)->settings.flowControl;
}

void ModbusSerialPort::setParity(Parity parity)
{
    if (d_ModbusSerialPort(d_ptr)->settings.parity != parity)
    {
        d_ModbusSerialPort(d_ptr)->settings.parity = parity;
        setChanged(true);
    }
}

StopBits ModbusSerialPort::stopBits() const
{
    return d_ModbusSerialPort(d_ptr)->settings.stopBits;
}

void ModbusSerialPort::setFlowControl(FlowControl flowControl)
{
    if (d_ModbusSerialPort(d_ptr)->settings.flowControl != flowControl)
    {
        d_ModbusSerialPort(d_ptr)->settings.flowControl = flowControl;
        setChanged(true);
    }
}

uint32_t ModbusSerialPort::timeoutFirstByte() const
{
    return d_ModbusSerialPort(d_ptr)->settings.timeoutFirstByte;
}

void ModbusSerialPort::setTimeoutFirstByte(uint32_t timeout)
{
    if (d_ModbusSerialPort(d_ptr)->settings.timeoutFirstByte != timeout)
    {
        d_ModbusSerialPort(d_ptr)->settings.timeoutFirstByte = timeout;
        setChanged(true);
    }
}

uint32_t ModbusSerialPort::timeoutInterByte() const
{
    return d_ModbusSerialPort(d_ptr)->settings.timeoutInterByte;
}

void ModbusSerialPort::setTimeoutInterByte(uint32_t timeout)
{
    if (d_ModbusSerialPort(d_ptr)->settings.timeoutInterByte != timeout)
    {
        d_ModbusSerialPort(d_ptr)->settings.timeoutInterByte = timeout;
        setChanged(true);
    }
}
