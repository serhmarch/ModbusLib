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

/*! \brief Main interface of Modbus communication protocol.

    \details `ModbusInterface` constains list of functions that ModbusLib is supported.
    There are such functions as:
    * 1  (0x01) - `READ_COILS`
    * 2  (0x02) - `READ_DISCRETE_INPUTS`
    * 3  (0x03) - `READ_HOLDING_REGISTERS`
    * 4  (0x04) - `READ_INPUT_REGISTERS`
    * 5  (0x05) - `WRITE_SINGLE_COIL`
    * 6  (0x06) - `WRITE_SINGLE_REGISTER`
    * 7  (0x07) - `READ_EXCEPTION_STATUS`
    * 8  (0x08) - `DIAGNOSTICS`
    * 11 (0x0B) - `GET_COMM_EVENT_COUNTER`
    * 12 (0x0C) - `GET_COMM_EVENT_LOG`
    * 15 (0x0F) - `WRITE_MULTIPLE_COILS`
    * 16 (0x10) - `WRITE_MULTIPLE_REGISTERS`
    * 17 (0x11) - `REPORT_SERVER_ID`
    * 22 (0x16) - `MASK_WRITE_REGISTER`
    * 23 (0x17) - `READ_WRITE_MULTIPLE_REGISTERS`
    * 24 (0x18) - `READ_FIFO_QUEUE`

    Default implementation of every Modbus function returns `Modbus::Status_BadIllegalFunction`.
 */
class MODBUS_EXPORT ModbusInterface
{
public:

#ifndef MBF_READ_COILS_DISABLE
    /// \details Function for read discrete outputs (coils, 0x bits).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  count   Count of coils (bits).
    /// \param[out] values  Pointer to the output buffer (bit array) where the read values are stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    /// \details Function for read digital inputs (1x bits).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  count   Count of inputs (bits).
    /// \param[out] values  Pointer to the output buffer (bit array) where the read values are stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    /// \details Function for read holding (output) 16-bit registers (4x regs).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  count   Count of registers.
    /// \param[out] values  Pointer to the output buffer (bit array) where the read values are stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    /// \details Function for read input 16-bit registers (3x regs).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  count   Count of registers.
    /// \param[out] values  Pointer to the output buffer (bit array) where the read values are stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    /// \details Function for write one separate discrete output (0x coil).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  value   Boolean value to be set.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value);
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    /// \details Function for write one separate 16-bit holding register (4x).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  value   16-bit unsigned integer value to be set.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value);
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    /// \details Function to read ExceptionStatus.
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[out] status  Pointer to the byte (bit array) where the exception status is stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *status);
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    /// \details Function provides a series of tests for checking the communication system between a client device and a server,
    /// or for checking various internal error conditions within a server.
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  subfunc Address of the remote Modbus device.
    /// \param[in]  insize  Size of the input buffer (in bytes).
    /// \param[in]  indata  Pointer to the buffer where the input (request) data is stored.
    /// \param[out] outsize Size of the buffer (in bytes) where the output data is stored.
    /// \param[out] outdata Pointer to the buffer where the output data is stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata);
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    /// \details Function is used to get a status word and an event count from the remote device's communication event counter.
    /// \param[in]  unit        Address of the remote Modbus device.
    /// \param[out] status      Returned status word.
    /// \param[out] eventCount  Returned event counter.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount);
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    /// \details Function is used to get a status word and an event count from the remote device's communication event counter.
    /// \param[in]  unit          Address of the remote Modbus device.
    /// \param[out] status        Returned status word.
    /// \param[out] eventCount    Returned event counter.
    /// \param[out] messageCount  Returned message counter.
    /// \param[out] eventBuffSize Size of the buffer where the output events (bytes) is stored.
    /// \param[out] eventBuff     Pointer to the buffer where the output events (bytes) is stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    /// \details Function is used to modify the contents of a specified holding register using a
    /// combination of an AND mask, an OR mask, and the register's current contents.
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  count   Count of coils.
    /// \param[in]  values  Pointer to the input buffer (bit array) which values must be written.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details Function for write holding (output) 16-bit registers (4x regs).
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  count   Count of registers.
    /// \param[in]  values  Pointer to the input buffer which values must be written.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    /// \details Function to read the description of the type, the current status, and other information specific to a remote device.
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  count   Count of bytes returned.
    /// \param[in]  data    Pointer to the output buffer where the read data are stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode reportServerID(uint8_t unit, uint8_t *count, uint8_t *data);
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    /// \details Function is used to modify the contents of a specified holding register using a
    /// combination of an AND mask, an OR mask, and the register's current contents.
    /// The function’s algorithm is:
    /// `Result = (Current Contents AND And_Mask) OR (Or_Mask AND (NOT And_Mask))`
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  offset  Starting offset (0-based).
    /// \param[in]  andMask 16-bit unsigned integer value AND mask.
    /// \param[in]  orMask  16-bit unsigned integer value OR mask.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask);
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    /// \details This function code performs a combination of one read operation and one write operation in a single MODBUS transaction.
    /// \param[in]  unit        Address of the remote Modbus device.
    /// \param[in]  readOffset  Starting offset for read(0-based).
    /// \param[in]  readCount   Count of registers to read.
    /// \param[out] readValues  Pointer to the output buffer which values must be read.
    /// \param[in]  writeOffset Starting offset for write(0-based).
    /// \param[in]  writeCount  Count of registers to write.
    /// \param[in]  writeValues Pointer to the input buffer which values must be written.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    /// \details Function for read the contents of a First-In-First-Out (FIFO) queue of register in a remote device.
    /// \param[in]  unit    Address of the remote Modbus device.
    /// \param[in]  fifoadr Address of FIFO (0-based).
    /// \param[in]  count   Count of registers.
    /// \param[out] values  Pointer to the output buffer where the read values are stored.
    /// \return The result `Modbus::StatusCode` of the operation. Default implementation returns `Status_BadIllegalFunction`.
    virtual Modbus::StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values);
#endif // MBF_READ_FIFO_QUEUE_DISABLE

};

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- Modbus namespace -------------------------------------------
// --------------------------------------------------------------------------------------------------------                 

/// Main Modbus namespace. Contains classes, functions and constants to work with Modbus-protocol.
namespace Modbus {

/// \brief Modbus::String class for strings
typedef std::string String;

/// \brief Modbus::List template class
template <class T>
using List = std::list<T>;

/// \details Returns string representation of the last error
MODBUS_EXPORT String getLastErrorText();

/// \details Convert interger value to Modbus::String
/// \returns Returns new Modbus::String value
inline String toModbusString(int val) { return std::to_string(val); }

/// \details Make string representation of bytes array and separate bytes by space
MODBUS_EXPORT String bytesToString(const uint8_t* buff, uint32_t count);

/// \details Make string representation of ASCII array and separate bytes by space
MODBUS_EXPORT String asciiToString(const uint8_t* buff, uint32_t count);

/// \details Return list of names of available serial ports
MODBUS_EXPORT List<String> availableSerialPorts();

/// \details Return list of baud rates
MODBUS_EXPORT List<int32_t> availableBaudRate();

/// \details Return list of data bits
MODBUS_EXPORT List<int8_t> availableDataBits();

/// \details Return list of `Parity` values
MODBUS_EXPORT List<Parity> availableParity();

/// \details Return list of `StopBits` values
MODBUS_EXPORT List<StopBits> availableStopBits();

/// \details Return list of `FlowControl` values
MODBUS_EXPORT List<FlowControl> availableFlowControl();

/// \details Function for creation `ModbusPort` with defined parameters:
/// \param[in]  type        Protocol type: TCP, RTU, ASC.
/// \param[in]  settings    For TCP must be pointer: `TcpSettings*`, `SerialSettings*` otherwise.
/// \param[in]  blocking    If true blocking will be set, non blocking otherwise.
MODBUS_EXPORT ModbusPort *createPort(ProtocolType type, const void *settings, bool blocking);

#ifndef MB_CLIENT_DISABLE
/// \details Function for creation `ModbusClientPort` with defined parameters:
/// \param[in]  type        Protocol type: TCP, RTU, ASC.
/// \param[in]  settings    For TCP must be pointer: `TcpSettings*`, `SerialSettings*` otherwise.
/// \param[in]  blocking    If true blocking will be set, non blocking otherwise.
MODBUS_EXPORT ModbusClientPort *createClientPort(ProtocolType type, const void *settings, bool blocking);
#endif // MB_CLIENT_DISABLE

#ifndef MB_SERVER_DISABLE
/// \details Function for creation `ModbusServerPort` with defined parameters:
/// \param[in]  device      Pointer to the `ModbusInterface` implementation to which all requests for Modbus functions are forwarded.
/// \param[in]  type        Protocol type: TCP, RTU, ASC.
/// \param[in]  settings    For TCP must be pointer: `TcpSettings*`, `SerialSettings*` otherwise.
/// \param[in]  blocking    If true blocking will be set, non blocking otherwise.
MODBUS_EXPORT ModbusServerPort *createServerPort(ModbusInterface *device, ProtocolType type, const void *settings, bool blocking);
#endif // MB_SERVER_DISABLE

/// \details Overloaded function
inline StatusCode readMemRegs(uint32_t offset, uint32_t count, void *values, const void *memBuff, uint32_t memRegCount)
{
    return readMemRegs(offset, count , values, memBuff, memRegCount, nullptr);
}

/// \details Overloaded function
inline StatusCode writeMemRegs(uint32_t offset, uint32_t count, const void *values, void *memBuff, uint32_t memRegCount)
{
    return writeMemRegs(offset, count , values, memBuff, memRegCount, nullptr);
}

/// \details Overloaded function
inline StatusCode readMemBits(uint32_t offset, uint32_t count, void *values, const void *memBuff, uint32_t memBitCount)
{
    return readMemBits(offset, count , values, memBuff, memBitCount, nullptr);
}

/// \details Overloaded function
inline StatusCode writeMemBits(uint32_t offset, uint32_t count, const void *values, void *memBuff, uint32_t memBitCount)
{
    return writeMemBits(offset, count , values, memBuff, memBitCount, nullptr);
}

} //namespace Modbus

#endif // MODBUS_H
