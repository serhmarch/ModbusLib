/*!
 * \file   ModbusSerialPort.h
 * \brief  Contains definition of base serial port class.
 *
 * \author serhmarch
 * \date   May 2024
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
