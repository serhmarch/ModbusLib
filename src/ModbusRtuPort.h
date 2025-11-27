/*!
 * \file   ModbusRtuPort.h
 * \brief  Contains definition of RTU serial port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSRTUPORT_H
#define MODBUSRTUPORT_H

#include "ModbusSerialPort.h"

/*! \brief Implements RTU version of the Modbus communication protocol.

    \details `ModbusRtuPort` derived from `ModbusSerialPort` and implements RTU (Remote Terminal Unit)
    framing for Modbus communication over serial lines (RS-232, RS-485, RS-422). RTU is a compact binary
    protocol that uses CRC-16 for error detection and relies on timing gaps between frames for message
    delimiting.
    
    RTU protocol characteristics:
    - Binary encoding - Data transmitted in raw binary format (more efficient than ASCII)
    - CRC-16 error checking - Appends 16-bit CRC to each frame for data integrity verification
    - Silent interval framing - Uses 3.5 character times of silence to mark frame boundaries
    - Efficient bandwidth usage - Typically 30-40% more efficient than ASCII protocol
    - Standard addressing - Supports 1-247 device addresses (0 for broadcast)
    
    The implementation handles:
    - Automatic CRC-16 calculation and verification
    - Proper timing for inter-character and inter-frame gaps
    - Unit address encoding in the frame header
    - Binary data packing and unpacking
    - Error detection and reporting
    
    Timing requirements:
    - Inter-character timeout - Maximum time between characters within a frame
    - Inter-frame delay - Minimum silent time (3.5 char) before starting new frame
    - These timings are baud-rate dependent and automatically calculated
 */
class MODBUS_EXPORT ModbusRtuPort : public ModbusSerialPort
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusRtuPort(bool blocking = false);

    ///  \details Destructor of the class.
    ~ModbusRtuPort();

public:
    /// \details Returns the Modbus protocol type. For `ModbusAscPort` returns `Modbus::RTU`.
    Modbus::ProtocolType type() const override { return Modbus::RTU; }

protected:
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) override;
    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

protected:
    using ModbusSerialPort::ModbusSerialPort;
};

#endif // MODBUSRTUPORT_H
