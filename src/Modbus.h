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

/// \details Returns trim white spaces from the left and right side of the string `str`
MODBUS_EXPORT String trim(const String &str);

/// \details Convert integer value to oct string representation withleading zeroes.
template<class StringT, class T>
StringT toBinString(T value)
{
    int c = sizeof(value) * MB_BYTE_SZ_BITES;
    StringT res(c, '0');
    while (value)
    {
        res[--c] = '0' + static_cast<char>(value & 1);
        value >>= 1;
    }
    return res;
}

/// \details Convert integer value to oct string representation withleading zeroes.
template<class StringT, class T>
StringT toOctString(T value)
{
    int c = (sizeof(value) * MB_BYTE_SZ_BITES + 2) / 3;
    StringT res(c, '0');
    while (value)
    {
        res[--c] = '0' + static_cast<char>(value & 7);
        value >>= 3;
    }
    return res;
}

/// \details Convert integer value to hex string representation with upper case and leading zeroes.
template<class StringT, class T>
StringT toHexString(T value)
{
    int c = sizeof(value) * 2;
    StringT res(c, '0');
    T v;
    while (value)
    {
        v = value & 0xF;
        if (v < 10)
            res[--c] = '0' + static_cast<char>(v);
        else
            res[--c] = 'A' - 10 + static_cast<char>(v);
        value >>= 4;
    }
    return res;
}

/// \details Convert integer value to dec string representation.
template<class StringT, class T>
StringT toDecString(T value)
{
    using CharT = typename StringT::value_type;
    const size_t sz = sizeof(T)*3+1;
    CharT buffer[sz];
    buffer[sz-1] = '\0';
    CharT *ptr = &buffer[sz-1];
    do
    {
        --ptr;
        T v = value % 10;
        ptr[0] = ('0' + static_cast<char>(v));
        value /= 10;
    }
    while (value);
    return StringT(ptr);
}

/// \details Convert integer value to dec string representation
/// for `c`-count symbols with left digits filled with `fillChar`.
template<class StringT, class T>
StringT toDecString(T value, int c, char fillChar = '0')
{
    StringT res(c, fillChar);
    do
    {
        T v = value % 10;
        res[--c] = ('0' + static_cast<char>(v));
        value /= 10;
    }
    while (value && c);
    return res;
}

/// \details Returns true if string `s` starts with `prefix`.
template <typename StringT>
bool startsWith(const StringT& s, const char* prefix)
{
    if (!prefix)
        return false;

    using CharT = typename StringT::value_type;

    size_t prefixLen = std::char_traits<char>::length(prefix);
    if (prefixLen > static_cast<size_t>(s.size()))
        return false;

    auto it = s.begin();
    for (size_t i = 0; i < prefixLen; ++i, ++it)
    {
        if (static_cast<CharT>(prefix[i]) != *it)
            return false;
    }
    return true;
}

/// \details Returns value of decimal digit [0-9] for ASCII code `ch`
/// or -1 if the value cannot be converted.
inline int decDigitValue(int ch)
{
    switch (ch)
    {
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
        return ch-'0';
    default:
        return -1;
    }
}

/// \details Returns value of hex digit [0-15] for ASCII code `ch`.
/// or -1 if the value cannot be converted.
inline int hexDigitValue(int ch)
{
    switch (ch)
    {
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
        return ch-'0';
    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
        return ch-'A'+10;
    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        return ch-'a'+10;
    default:
        return -1;
    }
}

#ifdef QT_CORE_LIB

/// \details Same as `decDigitValue(int ch)` but for `QChar` type.
inline int decDigitValue(QChar ch) { return decDigitValue(ch.toLatin1()); }

/// \details Same as `hexDigitValue(int ch)` but for `QChar` type.
inline int hexDigitValue(QChar ch) { return hexDigitValue(ch.toLatin1()); }

#endif // QT_CORE_LIB

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


#ifndef MB_ADDRESS_CLASS_DISABLE

#define sIEC61131Prefix0x "%Q"
#define sIEC61131Prefix1x "%I"
#define sIEC61131Prefix3x "%IW"
#define sIEC61131Prefix4x "%MW"
#define cIEC61131SuffixHex 'h'

/// \brief Modbus Data Address class. Represents Modbus Data Address.

/// \details `Address` class is used to represent Modbus Data Address. It contains memory type and offset.
/// E.g. `Address(Modbus::Memory_4x, 0)` creates `400001` standard address.
/// E.g. `Address(400001)` creates `Address` with type `Modbus::Memory_4x` and offset `0`, and
/// `Address(1)` creates `Address` with type `Modbus::Memory_0x` and offset `0`.
/// Class provides convertions from/to String using template methods for this.
/// `template <class StringT>` - `StringT` can be `std::basic_string` or `QString`.
class Address
{
public:
    /// \brief Type of Modbus Data Address notation.
    enum Notation
    {
        Notation_Default    , ///< Default notation which is equal to Modbus notation
        Notation_Modbus     , ///< Standard Modbus address notation like `000001`, `100001`, `300001`, `400001`
        Notation_IEC61131   , ///< IEC-61131 address notation like `%%Q0`, `%%I0`, `%%IW0`, `%%MW0`
        Notation_IEC61131Hex  ///< IEC-61131 Hex address notation like `%%Q0000h`, `%%I0000h`, `%%IW0000h`, `%%MW0000h`
    };

public:
    /// \details Defauilt constructor ot the class. Creates invalid Modbus Data Address
    Address() : m_type(Modbus::Memory_Unknown), m_offset(0) {}

    /// \details Constructor ot the class. E.g. `Address(Modbus::Memory_4x, 0)` creates `400001` standard address.
    Address(Modbus::MemoryType type, uint16_t offset) : m_type(type), m_offset(offset) {}

    /// \details Constructor ot the class. E.g. `Address(400001)` creates `Address` with type `Modbus::Memory_4x`
    /// and offset `0`, and `Address(1)` creates `Address` with type `Modbus::Memory_0x` and offset `0`.
    Address(uint32_t adr) { this->operator=(adr); }

public:
    /// \details Returns `true` if memory type is not `Modbus::Memory_Unknown`, `false` otherwise
    inline bool isValid() const { return m_type != Modbus::Memory_Unknown; }

    /// \details Returns memory type of Modbus Data Address
    inline Modbus::MemoryType type() const { return static_cast<Modbus::MemoryType>(m_type); }
 
    /// \details Returns memory offset of Modbus Data Address
    inline uint16_t offset() const { return m_offset; }

    /// \details Set memory offset of Modbus Data Address
    inline void setOffset(uint16_t offset) { m_offset = offset; }

    /// \details Returns memory number (offset+1) of Modbus Data Address
    inline uint32_t number() const { return m_offset+1; }

    /// \details Set memory number of Modbus Data Address
    inline void setNumber(uint16_t number) { m_offset = number-1; }

    /// \details Returns int repr of Modbus Data Address
    /// e.g. `Address(Modbus::Memory_4x, 0)` will be converted to `400001`.
    inline int toInt() const { return (m_type*100000) + number();  }

    /// \details Converts current Modbus Data Address to `quint32`,
    /// e.g. `Address(Modbus::Memory_4x, 0)` will be converted to `400001`.
    inline operator uint32_t () const { return (m_type*100000) + number();  }

    /// \details Assigment operator definition.
    inline Address &operator=(uint32_t v)
    {
        uint32_t number = v % 100000;
        if ((number < 1) || (number > 65536))
        {
            m_type = Modbus::Memory_Unknown;
            m_offset = 0;
            return *this;
        }
        uint16_t type = static_cast<uint16_t>(v/100000);
        switch(type)
        {
        case Modbus::Memory_0x:
        case Modbus::Memory_1x:
        case Modbus::Memory_3x:
        case Modbus::Memory_4x:
            m_type = type;
            m_offset = static_cast<uint16_t>(number-1);
            break;
        default:
            m_type = Modbus::Memory_Unknown;
            m_offset = 0;
            break;
        }
        return *this;
    }

    /// \details Add operator definition. Increase address offset by `c` value
    inline Address& operator+= (uint16_t c) { m_offset += c; return *this; }

    /// \brief Make modbus address from string representaion
    template<class StringT>
    static Address fromString(const StringT &s)
    {
        if (s.size() && s.at(0) == '%')
        {
            Address adr;
            decltype(s.size()) i;
            // Note: 3x (%IW) handled before 1x (%I)
            if (startsWith(s, sIEC61131Prefix3x)) // Check if string starts with sIEC61131Prefix3x
            {
                adr.m_type = Modbus::Memory_3x;
                i = sizeof(sIEC61131Prefix3x)-1;
            }
            else if (startsWith(s, sIEC61131Prefix4x)) // Check if string starts with sIEC61131Prefix4x
            {
                adr.m_type = Modbus::Memory_4x;
                i = sizeof(sIEC61131Prefix4x)-1;
            }
            else if (startsWith(s, sIEC61131Prefix0x)) // Check if string starts with sIEC61131Prefix0x
            {
                adr.m_type = Modbus::Memory_0x;
                i = sizeof(sIEC61131Prefix0x)-1;
            }
            else if (startsWith(s, sIEC61131Prefix1x)) // Check if string starts with sIEC61131Prefix1x
            {
                adr.m_type = Modbus::Memory_1x;
                i = sizeof(sIEC61131Prefix1x)-1;
            }
            else
                return Address();

            adr.m_offset = 0;
            auto suffix = s.back();
            if (suffix == cIEC61131SuffixHex)
            {
                for (; i < s.size()-1; i++)
                {
                    adr.m_offset *= 16;
                    int d = hexDigitValue(s.at(i));
                    if (d < 0)
                        return Address();
                    adr.m_offset += static_cast<uint16_t>(d);
                }
            }
            else
            {
                for (; i < s.size(); i++)
                {
                    adr.m_offset *= 10;
                    int d = decDigitValue(s.at(i));
                    if (d < 0)
                        return Address();
                    adr.m_offset += static_cast<uint16_t>(d);
                }
            }
            return adr;
        }
        uint32_t acc = 0;
        for (decltype(s.size()) i = 0; i < s.size(); i++)
        {
            acc *= 10;
            int d = decDigitValue(s.at(i));
            if (d < 0)
                return Address();
            acc += static_cast<uint16_t>(d);
        }
        return Address(acc);
    }

    /// \details Returns string repr of Modbus Data Address
    /// e.g. `Address(Modbus::Memory_4x, 0)` will be converted to `QString("400001")`.
    template<class StringT>
    StringT toString(Notation notation) const
    {
        if (isValid())
        {
            switch (notation)
            {
            case Notation_IEC61131:
                switch (m_type)
                {
                case Modbus::Memory_0x:
                    return StringT(sIEC61131Prefix0x) + toDecString<StringT>(offset());
                case Modbus::Memory_1x:
                    return StringT(sIEC61131Prefix1x) + toDecString<StringT>(offset());
                case Modbus::Memory_3x:
                    return StringT(sIEC61131Prefix3x) + toDecString<StringT>(offset());
                case Modbus::Memory_4x:
                    return StringT(sIEC61131Prefix4x) + toDecString<StringT>(offset());
                default:
                    return StringT();;
                }
                break;
            case Notation_IEC61131Hex:
            {
                switch (m_type)
                {
                case Modbus::Memory_0x:
                    return StringT(sIEC61131Prefix0x) + toHexString<StringT>(offset()) + cIEC61131SuffixHex;
                case Modbus::Memory_1x:
                    return StringT(sIEC61131Prefix1x) + toHexString<StringT>(offset()) + cIEC61131SuffixHex;
                case Modbus::Memory_3x:
                    return StringT(sIEC61131Prefix3x) + toHexString<StringT>(offset()) + cIEC61131SuffixHex;
                case Modbus::Memory_4x:
                    return StringT(sIEC61131Prefix4x) + toHexString<StringT>(offset()) + cIEC61131SuffixHex;
                default:
                    return StringT();
                }
            }
                break;
            default:
                return toDecString<StringT>(toInt(), 6);
            }
        }
        else
            return StringT();
    }

private:
    uint16_t m_type;
    uint16_t m_offset;
};

#endif // MB_ADDRESS_CLASS_DISABLE

} //namespace Modbus

#endif // MODBUS_H
