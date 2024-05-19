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

#include "ModbusObject.h"

class MODBUS_EXPORT ModbusServerPort : public ModbusObject
{
public:
    ModbusInterface *device() const;

public: // server port interface
    /// \details Returns type of Modbus protocol.
    virtual Modbus::ProtocolType type() const = 0;

    /// \details
    virtual bool isTcpServer() const;

    /// \details
    virtual Modbus::StatusCode open() = 0;

    /// \details Closes port/connection and returns status of the operation.
    virtual Modbus::StatusCode close() = 0;

    /// \details
    virtual bool isOpen() const = 0;

    /// \details
    virtual Modbus::StatusCode process() = 0;

public:
    /// \details
    bool isStateClosed() const;

public: // SIGNALS
    /// \details
    void signalOpened(const Modbus::Char *source);

    /// \details
    void signalClosed(const Modbus::Char *source);

    /// \details Calls each callback of the original packet 'Tx' from the internal list of callbacks, passing them the original array 'buff' and its size 'size'.
    void signalTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Calls each callback of the incoming packet 'Rx' from the internal list of callbacks, passing them the input array 'buff' and its size 'size'.
    void signalRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Calls each callback of the port when error is occured with error's status and text.
    void signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text);

protected:
    using ModbusObject::ModbusObject;
};

#endif // MODBUSSERVERPORT_H

