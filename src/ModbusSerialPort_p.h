#ifndef MODBUSSERIALPORT_P_H
#define MODBUSSERIALPORT_P_H

#include "ModbusPort_p.h"
#include "ModbusSerialPort.h"

class ModbusSerialPortPrivate : public ModbusPortPrivate
{
public:
    static ModbusSerialPortPrivate *create(bool blocking);

public:
    ModbusSerialPortPrivate(bool blocking) :
        ModbusPortPrivate(blocking)
    {
        const ModbusSerialPort::Defaults &d = ModbusSerialPort::Defaults::instance();

        settings.portName         = d.portName;
        settings.baudRate         = d.baudRate;
        settings.dataBits         = d.dataBits;
        settings.stopBits         = d.stopBits;
        settings.parity           = d.parity  ;
        settings.flowControl      = d.flowControl;
        settingsBase.timeout      = d.timeoutFirstByte;
        settings.timeoutInterByte = d.timeoutInterByte;
    }

public: // settings
    inline const String &portName() const { return settings.portName; }
    inline void setPortName(const String& portName) { settings.portName = portName; }
    inline void setPortName(String &&portName) { settings.portName = std::move(portName); }
    inline void setPortName(const Char *portName) { settings.portName = portName; }

    inline int32_t baudRate() const { return settings.baudRate; }
    inline void setBaudRate(int32_t baudRate) { settings.baudRate = baudRate; }

    inline int8_t dataBits() const { return settings.dataBits; }
    inline void setDataBits(int8_t dataBits) { settings.dataBits = dataBits; }

    inline Parity parity() const { return settings.parity; }
    inline void setParity(Modbus::Parity parity) { settings.parity = parity; }

    inline StopBits stopBits() const { return settings.stopBits; }
    inline void setStopBits(Modbus::StopBits stopBits) { settings.stopBits = stopBits; }
    
    inline FlowControl flowControl() const { return settings.flowControl; }
    inline void setFlowControl(Modbus::FlowControl flowControl) { settings.flowControl = flowControl; }\

    inline uint32_t timeoutFirstByte() const { return settingsBase.timeout; }
    inline void setTimeoutFirstByte(uint32_t timeout) { settingsBase.timeout = timeout; }

    inline uint32_t timeoutInterByte() const { return settings.timeoutInterByte; }
    inline void setTimeoutInterByte(uint32_t timeout) { settings.timeoutInterByte = timeout; }
    
public:
    struct
    {
        String portName;
        int32_t baudRate;
        int8_t dataBits;
        Parity parity;
        StopBits stopBits;
        FlowControl flowControl;
        uint32_t timeoutInterByte;
    } settings;

    uint8_t *buff;
    uint16_t c_buffSz;
    uint16_t sz;

};

#endif // MODBUSSERIALPORT_P_H
