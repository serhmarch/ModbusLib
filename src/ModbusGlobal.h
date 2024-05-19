/*!
 * \file   ModbusGlobal.h
 * \brief  Contains general definitions of the Modbus libarary (for C++ and "pure" C).
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSGLOBAL_H
#define MODBUSGLOBAL_H

#include <stdint.h>
#include <string.h>

#include "ModbusPlatform.h"
#include "Modbus_config.h"

/*
   MODBUSLIB_VERSION is (major << 16) + (minor << 8) + patch.
*/
#define MODBUSLIB_VERSION ((MODBUSLIB_VERSION_MAJOR<<16)|(MODBUSLIB_VERSION_MINOR<<8)|(MODBUSLIB_VERSION_PATCH))

/*
   MODBUSLIB_VERSION_STR "major.minor.patch"
*/

#define MODBUSLIB_VERSION_STR_HELPER(major,minor,patch) #major"."#minor"."#patch

#define MODBUSLIB_VERSION_STR_MAKE(major,minor,patch) MODBUSLIB_VERSION_STR_HELPER(major,minor,patch)

#define MODBUSLIB_VERSION_STR MODBUSLIB_VERSION_STR_MAKE(MODBUSLIB_VERSION_MAJOR,MODBUSLIB_VERSION_MINOR,MODBUSLIB_VERSION_PATCH)


#if defined(MODBUS_EXPORTS) && defined(MB_DECL_EXPORT)
#define MODBUS_EXPORT MB_DECL_EXPORT
#elif defined(MB_DECL_IMPORT)
#define MODBUS_EXPORT MB_DECL_IMPORT
#else
#define MODBUS_EXPORT
#endif

#define StringLiteral(cstr) cstr
#define CharLiteral(cchar) cchar

// --------------------------------------------------------------------------------------------------------
// -------------------------------------------- Helper macros ---------------------------------------------
// --------------------------------------------------------------------------------------------------------

#define GET_BIT(bitBuff, bitNum) ((((const uint8_t*)(bitBuff))[(bitNum)/8] & (1<<((bitNum)%8))) != 0)

#define SET_BIT(bitBuff, bitNum, value)                                                                                     \
    if (value)                                                                                                              \
        ((uint8_t*)(bitBuff))[(bitNum)/8] |= (1<<((bitNum)%8));                                                             \
    else                                                                                                                    \
        ((uint8_t*)(bitBuff))[(bitNum)/8] &= (~(1<<((bitNum)%8)));

#define GET_BITS(bitBuff, bitNum, bitCount, boolBuff)                                                                       \
    for (uint16_t __i__ = 0; __i__ < bitCount; __i__++)                                                                     \
        boolBuff[__i__] = (((const uint8_t*)(bitBuff))[((bitNum)+__i__)/8] & (1<<(((bitNum)+__i__)%8))) != 0;

#define SET_BITS(bitBuff, bitNum, bitCount, boolBuff)                                                                       \
    for (uint16_t __i__ = 0; __i__ < bitCount; __i__++)                                                                     \
        if (boolBuff[__i__])                                                                                                \
            ((uint8_t*)(bitBuff))[((bitNum)+__i__)/8] |= (1<<(((bitNum)+__i__)%8));                                         \
        else                                                                                                                \
            ((uint8_t*)(bitBuff))[((bitNum)+__i__)/8] &= (~(1<<(((bitNum)+__i__)%8)));


// --------------------------------------------------------------------------------------------------------
// ----------------------------------------- Modbus function codes ----------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \name Modbus Functions
/// Modbus Function's codes.
///@{ 
#define MBF_READ_COILS                          1
#define MBF_READ_DISCRETE_INPUTS                2
#define MBF_READ_HOLDING_REGISTERS              3
#define MBF_READ_INPUT_REGISTERS                4
#define MBF_WRITE_SINGLE_COIL                   5
#define MBF_WRITE_SINGLE_REGISTER               6
#define MBF_READ_EXCEPTION_STATUS               7
#define MBF_DIAGNOSTICS                         8
#define MBF_GET_COMM_EVENT_COUNTER              11
#define MBF_GET_COMM_EVENT_LOG                  12
#define MBF_WRITE_MULTIPLE_COILS                15
#define MBF_WRITE_MULTIPLE_REGISTERS            16
#define MBF_REPORT_SERVER_ID                    17
#define MBF_READ_FILE_RECORD                    20
#define MBF_WRITE_FILE_RECORD                   21
#define MBF_MASK_WRITE_REGISTER                 22
#define MBF_READ_WRITE_MULTIPLE_REGISTERS       23
#define MBF_READ_FIFO_QUEUE                     24
#define MBF_ENCAPSULATED_INTERFACE_TRANSPORT    43
#define MBF_ILLEGAL_FUNCTION                    73 
#define MBF_EXCEPTION                           128    
///@}


// --------------------------------------------------------------------------------------------------------
// ---------------------------------------- Modbus count constants ----------------------------------------
// --------------------------------------------------------------------------------------------------------

// 8 = count bits in byte (byte size in bits)
#define MB_BYTE_SZ_BITES 8

// 16 = count bits in 16 bit register (register size in bits) 
#define MB_REGE_SZ_BITES 16

// 2 = count bytes in 16 bit register (register size in bytes) 
#define MB_REGE_SZ_BYTES 2

// 255 - count_of_bytes in function readHoldingRegisters, readCoils etc
#define MB_VALUE_BUFF_SZ 255

// 127 = 255(count_of_bytes in function readHoldingRegisters etc) / 2 (register size in bytes)
#define MB_MAX_REGISTERS 127

// 2040 = 255(count_of_bytes in function readCoils etc) * 8 (bits in byte)
#define MB_MAX_DISCRETS 2040

// Maximum func data size: WriteMultipleCoils
// 261 = 1 byte(function) + 2 bytes (starting offset) + 2 bytes (count) + 1 bytes (byte count) + 255 bytes(maximum data length)

// 1 byte(unit) + 261 (max func data size: WriteMultipleCoils) + 2 bytes(CRC)
#define MB_RTU_IO_BUFF_SZ 264

// 1 byte(start symbol ':')+(( 1 byte(unit) + 261 (max func data size: WriteMultipleCoils)) + 1 byte(LRC) ))*2+2 bytes(CR+LF)
#define MB_ASC_IO_BUFF_SZ 529

// 6 bytes(tcp-prefix)+1 byte(unit)+261 (max func data size: WriteMultipleCoils)
#define MB_TCP_IO_BUFF_SZ 268

#ifdef __cplusplus

namespace Modbus {

extern "C" {

#endif // __cplusplus

typedef void* Handle;
typedef char Char;
typedef uint32_t Timer;

/// \brief Define list of contants of Modbus protocol.
enum Constants
{
    VALID_MODBUS_ADDRESS_BEGIN = 1  , ///< Start of Modbus device address range according to specification
    VALID_MODBUS_ADDRESS_END   = 247, ///< End of the Modbus protocol device address range according to the specification
    STANDARD_TCP_PORT          = 502  ///< Standard TCP port of the Modbus protocol
};

//=========== Modbus protocol types ===============

/// \brief Defines type of Modbus protocol.
typedef enum _ProtocolType
{
    ASC,
    RTU,
    TCP
} ProtocolType;

/// \brief Defines type of memory used in Modbus protocol.
typedef enum _MemoryType
{
    Memory_Unknown = 0xFFFF,
    Memory_0x = 0,                           ///< Memory allocated for coils/discrete outputs
    Memory_Coils = Memory_0x,                ///< Same as `Memory_0x`.
    Memory_1x = 1,                           ///< Memory allocated for discrete inputs
    Memory_DiscreteInputs = Memory_1x,       ///< Same as `Memory_1x`.
    Memory_3x = 3,                           ///< Memory allocated for analog inputs
    Memory_InputRegisters = Memory_3x,       ///< Same as `Memory_3x`.
    Memory_4x = 4,                           ///< Memory allocated for holding registers/analog outputs
    Memory_HoldingRegisters = Memory_4x,     ///< Same as `Memory_4x`.
} MemoryType;

/// \brief Defines status of executed Modbus functions.
typedef enum _StatusCode
{
    Status_Processing               = 0x80000000, ///< The operation is not complete. Further operation is required
    Status_Good                     = 0x00000000, ///< Successful result
    Status_Bad                      = 0x01000000, ///< Error. General
    Status_Uncertain                = 0x02000000, ///< The status is undefined
                                                          
    //------ Modbus standart errors begin -------         
    // from 0 to 255                                      
    Status_BadIllegalFunction                    = Status_Bad | 0x01, ///< Standard error. The feature is not supported
    Status_BadIllegalDataAddress                 = Status_Bad | 0x02, ///< Standard error. Invalid data address
    Status_BadIllegalDataValue                   = Status_Bad | 0x03, ///< Standard error. Invalid data value
    Status_BadServerDeviceFailure                = Status_Bad | 0x04, ///< Standard error. Failure during a specified operation
    Status_BadAcknowledge                        = Status_Bad | 0x05, ///< Standard error. The server has accepted the request and is processing it, but it will take a long time
    Status_BadServerDeviceBusy                   = Status_Bad | 0x06, ///< Standard error. The server is busy processing a long command. The request must be repeated later
    Status_BadNegativeAcknowledge                = Status_Bad | 0x07, ///< Standard error. The programming function cannot be performed
    Status_BadMemoryParityError                  = Status_Bad | 0x08, ///< Standard error. The server attempted to read a record file but detected a parity error in memory
    Status_BadGatewayPathUnavailable             = Status_Bad | 0x0A, ///< Standard error. Indicates that the gateway was unable to allocate an internal communication path from the input port o the output port for processing the request. Usually means that the gateway is misconfigured or overloaded
    Status_BadGatewayTargetDeviceFailedToRespond = Status_Bad | 0x0B, ///< Standard error. Indicates that no response was obtained from the target device. Usually means that the device is not present on the network
    //------- Modbus standart errors end --------
                                                          
    //------- Modbus common errors begin --------         
    Status_BadEmptyResponse         = Status_Bad | 0x101, ///< Error. Empty request/response body
    Status_BadNotCorrectRequest     ,                     ///< Error. Invalid request
    Status_BadNotCorrectResponse    ,                     ///< Error. Invalid response
    Status_BadWriteBufferOverflow   ,                     ///< Error. Write buffer overflow
    Status_BadReadBufferOverflow    ,                     ///< Error. Request receive buffer overflow

    //-------- Modbus common errors end ---------

    //--_ Modbus serial specified errors begin --         
    Status_BadSerialOpen            = Status_Bad | 0x201, ///< Error. Serial port cannot be opened
    Status_BadSerialWrite           ,                     ///< Error. Cannot send a parcel to the serial port
    Status_BadSerialRead            ,                     ///< Error. Reading the serial port (timeout)
    //---_ Modbus serial specified errors end ---         
                                                          
    //---- Modbus ASC specified errors begin ----         
    Status_BadAscMissColon          = Status_Bad | 0x301, ///< Error (ASC). Missing packet start character ':'
    Status_BadAscMissCrLf           ,                     ///< Error (ASC). '\\r\\n' end of packet character missing
    Status_BadAscChar               ,                     ///< Error (ASC). Invalid ASCII character
    Status_BadLrc                   ,                     ///< Error (ASC). Invalid checksum
    //---- Modbus ASC specified errors end ----           
                                                          
    //---- Modbus RTU specified errors begin ----         
    Status_BadCrc                   = Status_Bad | 0x401, ///< Error (RTU). Wrong checksum
    //----- Modbus RTU specified errors end -----         
                                                          
    //--_ Modbus TCP specified errors begin --            
    Status_BadTcpCreate             = Status_Bad | 0x501, ///< Error. Unable to create a TCP socket
    Status_BadTcpConnect,                                 ///< Error. Unable to create a TCP connection
    Status_BadTcpWrite,                                   ///< Error. Unable to send a TCP packet
    Status_BadTcpRead,                                    ///< Error. Unable to receive a TCP packet
    Status_BadTcpBind,                                    ///< Error. Unable to bind a TCP socket (server side)
    Status_BadTcpListen,                                  ///< Error. Unable to listen a TCP socket (server side)
    Status_BadTcpAccept,                                  ///< Error. Unable accept bind a TCP socket (server side)
    //---_ Modbus TCP specified errors end ---
} StatusCode;

typedef enum _Parity
{
    NoParity,
    EvenParity,
    OddParity,
    SpaceParity,
    MarkParity
} Parity;

typedef enum _StopBits
{
    OneStop,
    OneAndHalfStop,
    TwoStop
} StopBits;

typedef enum _FlowControl
{
    NoFlowControl,
    HardwareControl,
    SoftwareControl
} FlowControl;

typedef struct 
{
    const Char *portName        ;
    int32_t     baudRate        ;
    int8_t      dataBits        ;
    Parity      parity          ;
    StopBits    stopBits        ;
    FlowControl flowControl     ;
    uint32_t    timeoutFirstByte;
    uint32_t    timeoutInterByte;
} SerialPortSettings;

typedef struct 
{
    const Char *host   ;
    uint16_t    port   ;
    uint16_t    timeout;
} TcpSettings;

/// \details Returns a general indication that the result of the operation is incomplete.
inline bool StatusIsProcessing(StatusCode status) { return status == Status_Processing; }

/// \details Returns a general indication that the operation result is successful.
inline bool StatusIsGood(StatusCode status) { return status == Status_Good; }

/// \details Returns a general indication that the operation result is unsuccessful.
inline bool StatusIsBad(StatusCode status) { return (status & Status_Bad) != 0; }

/// \details Returns a general sign that the result of the operation is undefined.
inline bool StatusIsUncertain(StatusCode status) { return (status & Status_Uncertain) != 0; }

/// \details Returns a general sign that the result is standard error.
inline bool StatusIsStandardError(StatusCode status) { return (status & Status_Bad) && ((status & 0xFF00) == 0); }

/// \details Returns the value of the bit with number `bitNum' from the bit array `bitBuff'.
inline bool getBit(const void *bitBuff, uint16_t bitNum) { return GET_BIT (bitBuff, bitNum); }

/// \details Returns the value of the bit with the number `bitNum' from the bit array `bitBuff', if the bit number is greater than or equal to `maxBitCount', then `false' is returned.
inline bool getBitS(const void *bitBuff, uint16_t bitNum, uint16_t maxBitCount) { return (bitNum < maxBitCount) ? getBit(bitBuff, bitNum) : false; }

/// \details Sets the value of the bit with the number `bitNum' to the bit array `bitBuff'.
inline void setBit(void *bitBuff, uint16_t bitNum, bool value) { SET_BIT (bitBuff, bitNum, value) }

/// \details Sets the value of the bit with the number `bitNum' to the bit array `bitBuff', controlling the size of the array `maxBitCount' in bits.
inline void setBitS(void *bitBuff, uint16_t bitNum, bool value, uint16_t maxBitCount) { if (bitNum < maxBitCount) setBit(bitBuff, bitNum, value); }

/// \details Gets the values of bits with number `bitNum` and count `bitCount` from the bit array `bitBuff` and stores their values in the boolean array `boolBuff`,
/// where the value of each bit is stored as a separate `bool` value.
/// \return A pointer to the `boolBuff` array.
inline bool *getBits(const void *bitBuff, uint16_t bitNum, uint16_t bitCount, bool *boolBuff) { GET_BITS(bitBuff, bitNum, bitCount, boolBuff) return boolBuff; }

/// \details Similar to the `Modbus::getBits(const void*,uint16_t,uint16_t,bool*)` function, but it is controlled that the size does not exceed the maximum number of bits `maxBitCount`.
/// \return A pointer to the `boolBuff` array.
inline bool *getBitsS(const void *bitBuff, uint16_t bitNum, uint16_t bitCount, bool *boolBuff, uint16_t maxBitCount) { if ((bitNum+bitCount) <= maxBitCount) getBits(bitBuff, bitNum, bitCount, boolBuff); return boolBuff; }

/// \details Sets the values of the bits in the `bitBuff` array starting with the number `bitNum` and the count `bitCount` from the `boolBuff` array,
/// where the value of each bit is stored as a separate `bool` value.
/// \return A pointer to the `bitBuff` array.
inline void *setBits(void *bitBuff, uint16_t bitNum, uint16_t bitCount, const bool *boolBuff) { SET_BITS(bitBuff, bitNum, bitCount, boolBuff) return bitBuff; }

/// \details Similar to the `Modbus::setBits(void*,uint16_t,uint16_t,const bool*)` function, but it is controlled that the size does not exceed the maximum number of bits `maxBitCount`.
/// \return A pointer to the `bitBuff` array.
inline void *setBitsS(void *bitBuff, uint16_t bitNum, uint16_t bitCount, const bool *boolBuff, uint16_t maxBitCount) { if ((bitNum + bitCount) <= maxBitCount) setBits(bitBuff, bitNum, bitCount, boolBuff); return bitBuff; }

/// \details CRC16 checksum, hash function (for Modbus RTU).
/// \returns Returns a 16-bit unsigned integer value of the checksum
MODBUS_EXPORT uint16_t crc16(const uint8_t *byteArr, uint32_t count);

/// \details LRC checksum, hash function (for Modbus ASCII).
/// \returns Returns an 8-bit unsigned integer value of the checksum
MODBUS_EXPORT uint8_t lrc(const uint8_t *byteArr, uint32_t count);

/// \details
MODBUS_EXPORT StatusCode readMemRegs(uint32_t offset, uint32_t count, void *values, const void *memBuff, uint32_t memRegCount);

/// \details
MODBUS_EXPORT StatusCode writeMemRegs(uint32_t offset, uint32_t count, const void *values, void *memBuff, uint32_t memRegCount);

/// \details
MODBUS_EXPORT StatusCode readMemBits(uint32_t offset, uint32_t count, void *values, const void *memBuff, uint32_t memBitCount);

/// \details
MODBUS_EXPORT StatusCode writeMemBits(uint32_t offset, uint32_t count, const void *values, void *memBuff, uint32_t memBitCount);

/// \details Function converts byte array \c bytesBuff to ASCII repr of byte array.
/// Every byte of \c bytesBuff are repr as two bytes in \c asciiBuff,
/// where most signified tetrabits represented as leading byte in hex digit in ASCII encoding (upper) and
/// less signified tetrabits represented as tailing byte in hex digit in ASCII encoding (upper).
/// \c count is count bytes of \c bytesBuff.
/// \note Output array \c asciiBuff must be twice bigger than input array \c bytesBuff.
/// \returns Returns size of \c asciiBuff in bytes which calc as \c {outCount = count * 2}
MODBUS_EXPORT uint16_t bytesToAscii(const uint8_t* bytesBuff, uint8_t* asciiBuff, uint32_t count);

/// \details Convert ASCII array to bytes array.
/// \returns Returns count bytes converted
MODBUS_EXPORT uint16_t asciiToBytes(const uint8_t* asciiBuff, uint8_t* bytesBuff, uint32_t count);

/// \details Make string representation of bytes array and separate bytes by space
MODBUS_EXPORT Char *sbytes(const uint8_t* buff, uint32_t count, Char *str, uint32_t strmaxlen);

/// \details Make string representation of ASCII array and separate bytes by space
MODBUS_EXPORT Char *sascii(const uint8_t* buff, uint32_t count, Char *str, uint32_t strmaxlen);

/// \details Get timer value in milliseconds.
MODBUS_EXPORT Timer timer();

/// \details Make current thread sleep with 'msec' milliseconds.
MODBUS_EXPORT void msleep(uint32_t msec);

#ifdef __cplusplus

} //extern "C"

} //namespace Modbus

#endif // __cplusplus

#endif // MODBUSGLOBAL_H
