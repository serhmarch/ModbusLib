/*!
 * \file   Modbus.h
 * \brief  Contains general definitions of the Modbus protocol.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUS_H
#define MODBUS_H

#include <string>
#include <list>

#include "ModbusGlobal.h"

class ModbusPort;
class ModbusClientPort;
class ModbusServerPort;

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- Modbus interface -------------------------------------------
// --------------------------------------------------------------------------------------------------------

class ModbusInterface
{
public:
    virtual Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values) = 0;
    virtual Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values) = 0;
    virtual Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) = 0;
    virtual Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) = 0;
    virtual Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value) = 0;
    virtual Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value) = 0;
    virtual Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *status) = 0;
    virtual Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values) = 0;
    virtual Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values) = 0;
};

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- Modbus namespace -------------------------------------------
// --------------------------------------------------------------------------------------------------------                 

/// Main Modbus namespace. Contains classes, functions and constants to work with Modbus-protocol.
namespace Modbus {

typedef std::string String;

template <class T>
using List = std::list<T>;

/// \details Convert interger value to Modbus::String
/// \returns Returns new Modbus::String value
inline String toModbusString(int val) { return std::to_string(val); }

/// \details Make string representation of bytes array and separate bytes by space
MODBUS_EXPORT String bytesToString(const uint8_t* buff, uint32_t count);

/// \details Make string representation of ASCII array and separate bytes by space
MODBUS_EXPORT String asciiToString(const uint8_t* buff, uint32_t count);

/// \details Return list of names of available serial ports
MODBUS_EXPORT List<String> availableSerialPorts();

/// \details
MODBUS_EXPORT ModbusPort *createPort(ProtocolType type, const void *settings, bool blocking);

/// \details 
MODBUS_EXPORT ModbusClientPort *createClientPort(ProtocolType type, const void *settings, bool blocking);

/// \details 
MODBUS_EXPORT ModbusServerPort *createServerPort(ModbusInterface *device, ProtocolType type, const void *settings, bool blocking);

} //namespace Modbus

#endif // MODBUS_H
