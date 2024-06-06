/*
    Modbus

    Created: 2024
    Author: Serhii Marchuk, https://github.com/serhmarch

    Copyright (C) 2024  Serhii Marchuk

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
#ifndef MODBUSSERIALPORT_H
#define MODBUSSERIALPORT_H

#include "ModbusPort.h"

class MODBUS_EXPORT ModbusSerialPort : public ModbusPort
{
public:
    struct MODBUS_EXPORT Defaults
    {
        const Modbus::Char       *portName        ;
        const int32_t             baudRate        ;
        const int8_t              dataBits        ;
        const Modbus::Parity      parity          ;
        const Modbus::StopBits    stopBits        ;
        const Modbus::FlowControl flowControl     ;
        const uint32_t            timeoutFirstByte;
        const uint32_t            timeoutInterByte;

        Defaults();
        static const Defaults &instance();
    };

public:
    Modbus::Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;

public: // settings
    const Modbus::Char *portName() const;
    void setPortName(const Modbus::Char *portName);
    int32_t baudRate() const;
    void setBaudRate(int32_t baudRate);
    int8_t dataBits() const;
    void setDataBits(int8_t dataBits);
    Modbus::Parity parity() const;
    void setParity(Modbus::Parity parity);
    Modbus::StopBits stopBits() const;
    void setStopBits(Modbus::StopBits stopBits);
    Modbus::FlowControl flowControl() const;
    void setFlowControl(Modbus::FlowControl flowControl);
    uint32_t timeoutFirstByte() const;
    void setTimeoutFirstByte(uint32_t timeout);
    uint32_t timeoutInterByte() const;
    void setTimeoutInterByte(uint32_t timeout);

public:
    const uint8_t *readBufferData() const override;
    uint16_t readBufferSize() const override;
    const uint8_t *writeBufferData() const override;
    uint16_t writeBufferSize() const override;

protected:
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

protected:
    using ModbusPort::ModbusPort;
};

#endif // MODBUSSERIALPORT_H
