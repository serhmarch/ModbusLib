#ifndef MODBUSSERIALPORT_P_H
#define MODBUSSERIALPORT_P_H

#include "ModbusPort_p.h"

class ModbusSerialPortPrivate : public ModbusPortPrivate
{
public:
    static ModbusSerialPortPrivate *create(uint16_t maxBuffSize, bool blocking);

public:
    ModbusSerialPortPrivate(uint16_t maxBuffSize, bool blocking) :
        ModbusPortPrivate(maxBuffSize, blocking)
    {
        const Modbus::SerialDefaults &d = Modbus::SerialDefaults::instance();

        settings.hostOrPortName   = d.portName        ;
        settings.baudRate         = d.baudRate        ;
        settings.dataBits         = d.dataBits        ;
        settings.stopBits         = d.stopBits        ;
        settings.parity           = d.parity          ;
        settings.flowControl      = d.flowControl     ;
        settings.timeout          = d.timeoutFirstByte;
        settings.timeoutInterByte = d.timeoutInterByte;
    }

    inline auto portName        () const { return settings.hostOrPortName  ; }
    inline auto baudRate        () const { return settings.baudRate        ; }
    inline auto dataBits        () const { return settings.dataBits        ; }
    inline auto stopBits        () const { return settings.stopBits        ; }
    inline auto parity          () const { return settings.parity          ; }
    inline auto flowControl     () const { return settings.flowControl     ; }
    inline auto timeoutFirstByte() const { return settings.timeout         ; }
    inline auto timeoutInterByte() const { return settings.timeoutInterByte; }

};

inline ModbusSerialPortPrivate *d_ModbusSerialPort(ModbusPortPrivate *d_ptr) { return static_cast<ModbusSerialPortPrivate*>(d_ptr); }

#endif // MODBUSSERIALPORT_P_H
