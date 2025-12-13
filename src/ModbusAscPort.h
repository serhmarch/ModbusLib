/*!
 * \file   ModbusAscPort.h
 * \brief  Contains definition of ASCII serial port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSASCPORT_H
#define MODBUSASCPORT_H

#include "ModbusPort.h"

/*! \brief Implements ASCII version of the Modbus communication protocol.

    \details `ModbusAscPort` derived from `ModbusSerialPort` and implements ASCII (American Standard Code
    for Information Interchange) framing for Modbus communication over serial lines. ASCII protocol uses
    human-readable hexadecimal characters for data transmission, making it easier to debug and monitor
    with standard serial port tools, though less efficient than RTU.
    
    ASCII protocol characteristics:
    - Hexadecimal encoding - Each data byte encoded as two ASCII hex characters (0-9, A-F)
    - LRC error checking - Uses Longitudinal Redundancy Check (simpler than CRC-16)
    - Visible frame delimiters - Starts with ':' (colon) and ends with CR-LF (carriage return, line feed)
    - Human-readable format - Can be viewed and debugged with any terminal program
    - Standard addressing - Supports 1-247 device addresses (0 for broadcast)
    - Lower bandwidth efficiency - Approximately twice the bytes compared to RTU
    
    The implementation handles:
    - Automatic ASCII hex encoding and decoding
    - LRC calculation and verification
    - Frame delimiter insertion (':' start, CR-LF end)
    - Unit address encoding as two hex characters
    - Character-based error detection
    - Conversion between binary data and ASCII representation
    
    Frame structure:
    - Start character: ':' (0x3A)
    - Address: 2 ASCII hex characters
    - Function: 2 ASCII hex characters
    - Data: N pairs of ASCII hex characters
    - LRC: 2 ASCII hex characters
    - End: CR (0x0D) + LF (0x0A)
 */
class MODBUS_EXPORT ModbusAscPort : public ModbusPort
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusAscPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusAscPort` returns `Modbus::ASC`.
    Modbus::ProtocolType type() const override { return Modbus::ASC; }
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) override;
    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

protected:
    using ModbusPort::ModbusPort;
};

#endif // MODBUSASCPORT_H
