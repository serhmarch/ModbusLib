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

        setPortName        (d.portName        );
        setBaudRate        (d.baudRate        );
        setDataBits        (d.dataBits        );
        setStopBits        (d.stopBits        );
        setParity          (d.parity          );
        setFlowControl     (d.flowControl     );
        setTimeoutFirstByte(d.timeoutFirstByte);
        setTimeoutInterByte(d.timeoutInterByte);
    }
};

inline ModbusSerialPortPrivate *d_ModbusSerialPort(ModbusPortPrivate *d_ptr) { return static_cast<ModbusSerialPortPrivate*>(d_ptr); }

#endif // MODBUSSERIALPORT_P_H
