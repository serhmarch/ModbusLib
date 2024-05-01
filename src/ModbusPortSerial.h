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
#ifndef MODBUSPORTSERIAL_H
#define MODBUSPORTSERIAL_H

#include "ModbusPort.h"

namespace Modbus {

class MODBUS_EXPORT PortSerial : public Port
{
public:
    struct MODBUS_EXPORT Defaults
    {
        const String      portName        ;
        const int32_t     baudRate        ;
        const int8_t      dataBits        ;
        const Parity      parity          ;
        const StopBits    stopBits        ;
        const FlowControl flowControl     ;
        const uint32_t    timeoutFirstByte;
        const uint32_t    timeoutInterByte;

        Defaults();
        static const Defaults &instance();
    };

public:
    PortSerial(bool blocking = true);
    ~PortSerial();

public:
    Handle handle() const override;
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;

public: // settings
    inline String portName() const { return m_settings.portName; }
    void setPortName(const String &portName);
    inline int32_t baudRate() const { return m_settings.baudRate; }
    void setBaudRate(int32_t baudRate);
    inline int8_t dataBits() const { return m_settings.dataBits; }
    void setDataBits(int8_t dataBits);
    inline Parity parity() const { return m_settings.parity; }
    void setParity(Parity parity);
    inline StopBits stopBits() const { return m_settings.stopBits; }
    void setStopBits(StopBits stopBits);
    inline FlowControl flowControl() const { return m_settings.flowControl; }
    void setFlowControl(FlowControl flowControl);
    inline uint32_t timeoutFirstByte() const { return m_settings.timeoutFirstByte; }
    void setTimeoutFirstByte(uint32_t timeout);
    inline uint32_t timeoutInterByte() const { return m_settings.timeoutInterByte; }
    void setTimeoutInterByte(uint32_t timeout);

protected:
    StatusCode write() override;
    StatusCode read() override;

private:
    void constructorPrivate();
    void destructorPrivate();

protected:
    struct
    {
        String portName;
        int32_t baudRate;
        int8_t dataBits;
        Parity parity;
        StopBits stopBits;
        FlowControl flowControl;
        uint32_t timeoutFirstByte;
        uint32_t timeoutInterByte;
    } m_settings;

protected:
    uint8_t *m_buff;
    uint16_t c_buffSz;
    uint16_t m_sz;

private: // Platform specific data
    struct PlatformData;
    PlatformData *m_platformData;
};

} // namespace Modbus

#endif // MODBUSPORTSERIAL_H
