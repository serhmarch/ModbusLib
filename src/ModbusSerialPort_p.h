#ifndef MODBUSSERIALPORT_P_H
#define MODBUSSERIALPORT_P_H

#include "ModbusPort_p.h"
#include "ModbusSerialPort.h"

class ModbusSerialPortPrivate : public ModbusPortPrivate
{
public:
    static ModbusSerialPortPrivate *create(ModbusFramePrivate *f, bool blocking);

public:
    ModbusSerialPortPrivate(ModbusFramePrivate *f, bool blocking) :
        ModbusPortPrivate(f, blocking)
    {
        const Modbus::SerialDefaults &d = Modbus::SerialDefaults::instance();

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
    inline int32_t baudRate() const { return settings.baudRate; }
    inline int8_t dataBits() const { return settings.dataBits; }
    inline Parity parity() const { return settings.parity; }
    inline StopBits stopBits() const { return settings.stopBits; }
    inline FlowControl flowControl() const { return settings.flowControl; }
    inline uint32_t timeoutFirstByte() const { return settingsBase.timeout; }
    inline uint32_t timeoutInterByte() const { return settings.timeoutInterByte; }
    
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
};

#endif // MODBUSSERIALPORT_P_H
