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
#include "ModbusPortSerial.h"

namespace Modbus {

const PortSerial::Defaults &PortSerial::Defaults::instance()
{
    static const Defaults d;
    return d;
}

PortSerial::PortSerial(bool blocking) : Port(blocking)
{
    const Defaults &d = Defaults::instance();

    setPortName(d.portName);
    setBaudRate(d.baudRate);
    setDataBits(d.dataBits);
    setStopBits(d.stopBits);
    setParity(d.parity);
    setFlowControl(d.flowControl);
    setTimeoutFirstByte(d.timeoutFirstByte);
    setTimeoutInterByte(d.timeoutInterByte);

    constructorPrivate();
}

PortSerial::~PortSerial()
{
    destructorPrivate();
}

void PortSerial::setPortName(const String &portName)
{
    if (m_settings.portName != portName)
    {
        m_settings.portName = portName;
        setChanged(true);
    }
}

void PortSerial::setBaudRate(int32_t baudRate)
{
    if (m_settings.baudRate != baudRate)
    {
        m_settings.baudRate = baudRate;
        setChanged(true);
    }
}

void PortSerial::setDataBits(int8_t dataBits)
{
    if (m_settings.dataBits != dataBits)
    {
        m_settings.dataBits = dataBits;
        setChanged(true);
    }
}

void PortSerial::setStopBits(StopBits stopBits)
{
    if (m_settings.stopBits != stopBits)
    {
        m_settings.stopBits = stopBits;
        setChanged(true);
    }
}

void PortSerial::setParity(Parity parity)
{
    if (m_settings.parity != parity)
    {
        m_settings.parity = parity;
        setChanged(true);
    }
}

void PortSerial::setFlowControl(FlowControl flowControl)
{
    if (m_settings.flowControl != flowControl)
    {
        m_settings.flowControl = flowControl;
        setChanged(true);
    }
}

void PortSerial::setTimeoutFirstByte(uint32_t timeout)
{
    if (m_settings.timeoutFirstByte != timeout)
    {
        m_settings.timeoutFirstByte = timeout;
        setChanged(true);
    }
}

void PortSerial::setTimeoutInterByte(uint32_t timeout)
{
    if (m_settings.timeoutInterByte != timeout)
    {
        m_settings.timeoutInterByte = timeout;
        setChanged(true);
    }
}

} // namespace Modbus
