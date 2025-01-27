/*!
 * \file   cModbus.h
 * \brief  Contains library interface for C language
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef CMODBUS_H
#define CMODBUS_H

#include <stdbool.h>
#include "ModbusGlobal.h"

#ifdef __cplusplus
using namespace Modbus;
extern "C" {
#endif

#ifdef __cplusplus
class ModbusPort      ;
class ModbusInterface ;
class ModbusClientPort;
class ModbusClient    ;
class ModbusServerPort;

#else
typedef struct ModbusPort       ModbusPort      ;
typedef struct ModbusInterface  ModbusInterface ;
typedef struct ModbusClientPort ModbusClientPort;
typedef struct ModbusClient     ModbusClient    ;
typedef struct ModbusServerPort ModbusServerPort;
#endif

/// \brief Handle (pointer) of `ModbusPort` for C interface
typedef ModbusPort* cModbusPort;

/// \brief Handle (pointer) of `ModbusClientPort` for C interface
typedef ModbusClientPort* cModbusClientPort;

/// \brief Handle (pointer) of `ModbusClient` for C interface
typedef ModbusClient* cModbusClient;

/// \brief Handle (pointer) of `ModbusServerPort` for C interface
typedef ModbusServerPort* cModbusServerPort;

/// \brief Handle (pointer) of `ModbusInterface` for C interface
typedef ModbusInterface* cModbusInterface;

/// \brief Handle (pointer) of `ModbusDevice` for C interface
typedef void* cModbusDevice;

#ifndef MBF_READ_COILS_DISABLE
/// \details Pointer to C function for read coils (0x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::readCoils`
typedef StatusCode (*pfReadCoils)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
/// \details Pointer to C function for read discrete inputs (1x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::readDiscreteInputs`
typedef StatusCode (*pfReadDiscreteInputs)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
/// \details Pointer to C function for read holding registers (4x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::readHoldingRegisters`
typedef StatusCode (*pfReadHoldingRegisters)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
/// \details Pointer to C function for read input registers (3x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::readInputRegisters`
typedef StatusCode (*pfReadInputRegisters)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
/// \details Pointer to C function for write single coil (0x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::writeSingleCoil`
typedef StatusCode (*pfWriteSingleCoil)(cModbusDevice dev, uint8_t unit, uint16_t offset, bool value);
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
/// \details Pointer to C function for write single register (4x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::writeSingleRegister`
typedef StatusCode (*pfWriteSingleRegister)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t value);
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
/// \details Pointer to C function for read exception status bits. `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::readExceptionStatus`
typedef StatusCode (*pfReadExceptionStatus)(cModbusDevice dev, uint8_t unit, uint8_t *status);
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
/// \details Pointer to C function for diagnostics. `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::diagnostics`
typedef StatusCode (*pfDiagnostics)(cModbusDevice dev, uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata);
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
/// \details Pointer to C function for get communication event counter. `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::getCommEventCounter`
typedef StatusCode (*pfGetCommEventCounter)(cModbusDevice dev, uint8_t unit, uint16_t *status, uint16_t *eventCount);
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
/// \details Pointer to C function for get communication event logs. `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::getCommEventLog`
typedef StatusCode (*pfGetCommEventLog)(cModbusDevice dev, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
/// \details Pointer to C function for write coils (0x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::writeMultipleCoils`
typedef StatusCode (*pfWriteMultipleCoils)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, const void *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
/// \details Pointer to C function for write registers (4x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::writeMultipleRegisters`
typedef StatusCode (*pfWriteMultipleRegisters)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
/// \details Pointer to C function for report server id. `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::reportServerID`
typedef StatusCode (*pfReportServerID)(cModbusDevice dev, uint8_t unit, uint8_t *count, uint8_t *data);
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
/// \details Pointer to C function for mask write registers (4x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::maskWriteRegister`
typedef StatusCode (*pfMaskWriteRegister)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask);
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
/// \details Pointer to C function for write registers (4x). `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::writeMultipleRegisters`
typedef StatusCode (*pfReadWriteMultipleRegisters)(cModbusDevice dev, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
/// \details Pointer to C function for read FIFO queue. `dev` - pointer to any struct that can hold memory data. \sa `ModbusInterface::readFIFOQueue`
typedef StatusCode (*pfReadFIFOQueue)(cModbusDevice dev, uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values);
#endif // MBF_READ_FIFO_QUEUE_DISABLE

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusClientPort::signalOpened` and `ModbusServerPort::signalOpened`
typedef void (*pfSlotOpened)(const Char *source);

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusClientPort::signalClosed` and `ModbusServerPort::signalClosed`
typedef void (*pfSlotClosed)(const Char *source);

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusClientPort::signalTx` and `ModbusServerPort::signalTx`
typedef void (*pfSlotTx)(const Char *source, const uint8_t* buff, uint16_t size);

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusClientPort::signalRx` and `ModbusServerPort::signalRx`
typedef void (*pfSlotRx)(const Char *source, const uint8_t* buff, uint16_t size);

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusClientPort::signalError` and `ModbusServerPort::signalError`
typedef void (*pfSlotError)(const Char *source, StatusCode status, const Char *text);

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusTcpServer::signalNewConnection`
typedef void (*pfSlotNewConnection)(const Char *source);

/// \details Pointer to C callback function. `dev` - pointer to any struct that can hold memory data. \sa `ModbusTcpServer::signalCloseConnection`
typedef void (*pfSlotCloseConnection)(const Char *source);

/// \details Function create `ModbusInterface` object and returns pointer to it for server.
/// `dev` - pointer to any struct that can hold memory data.
/// readCoils,
/// readDiscreteInputs,
/// readHoldingRegisters,
/// readInputRegisters,
/// writeSingleCoil,
/// writeSingleRegister,
/// readExceptionStatus,
/// diagnostics,
/// getCommEventCounter,
/// getCommEventLog,
/// writeMultipleCoils
/// writeMultipleRegisters 
/// reportServerID,
/// maskWriteRegister,
/// readWriteMultipleRegisters,
/// readFIFOQueue - pointers to corresponding Modbus functions to process data.
/// Any pointer can have `NULL` value. In this case server will return `Status_BadIllegalFunction`.
MODBUS_EXPORT cModbusInterface cCreateModbusDevice(cModbusDevice                device                
#ifndef MBF_READ_COILS_DISABLE
                                                 , pfReadCoils                  readCoils             
#endif // MBF_READ_COILS_DISABLE
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
                                                 , pfReadDiscreteInputs         readDiscreteInputs    
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE
#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
                                                 , pfReadHoldingRegisters       readHoldingRegisters  
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
                                                 , pfReadInputRegisters         readInputRegisters    
#endif // MBF_READ_INPUT_REGISTERS_DISABLE
#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
                                                 , pfWriteSingleCoil            writeSingleCoil       
#endif // MBF_WRITE_SINGLE_COIL_DISABLE
#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
                                                 , pfWriteSingleRegister        writeSingleRegister   
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE
#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
                                                 , pfReadExceptionStatus        readExceptionStatus   
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE
#ifndef MBF_DIAGNOSTICS_DISABLE
                                                 , pfDiagnostics                diagnostics    
#endif // MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
                                                 , pfGetCommEventCounter        getCommEventCounter
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE
#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
                                                 , pfGetCommEventLog            getCommEventLog     
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE
#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
                                                 , pfWriteMultipleCoils         writeMultipleCoils
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                 , pfWriteMultipleRegisters     writeMultipleRegisters
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
#ifndef MBF_REPORT_SERVER_ID_DISABLE
                                                 , pfReportServerID             reportServerID
#endif // MBF_REPORT_SERVER_ID_DISABLE
#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
                                                 , pfMaskWriteRegister          maskWriteRegister
#endif // MBF_MASK_WRITE_REGISTER_DISABLE
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                 , pfReadWriteMultipleRegisters readWriteMultipleRegisters
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
#ifndef MBF_READ_FIFO_QUEUE_DISABLE
                                                 , pfReadFIFOQueue              readFIFOQueue
#endif // MBF_READ_FIFO_QUEUE_DISABLE
                                                 );


/// \details Deletes previously created `ModbusInterface` object represented by `dev` handle
MODBUS_EXPORT void cDeleteModbusDevice(cModbusInterface dev);

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------- ModbusPort ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \details Creates `ModbusPort` object and returns handle to it. \sa `Modbus::createPort`
MODBUS_EXPORT cModbusPort cPortCreate(ProtocolType type, const void *settings, bool blocking);

/// \details Deletes previously created `ModbusPort` object represented by `port` handle
MODBUS_EXPORT void cPortDelete(cModbusPort port);


// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusClientPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------
#ifndef MB_CLIENT_DISABLE

/// \details Creates `ModbusClientPort` object and returns handle to it. \sa `Modbus::createClientPort`
MODBUS_EXPORT cModbusClientPort cCpoCreate(ProtocolType type, const void *settings, bool blocking);

/// \details Creates `ModbusClientPort` object and returns handle to it.
MODBUS_EXPORT cModbusClientPort cCpoCreateForPort(cModbusPort port);

/// \details Deletes previously created `ModbusClientPort` object represented by `port` handle
MODBUS_EXPORT void cCpoDelete(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::objectName`
MODBUS_EXPORT const Char *cCpoGetObjectName(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::setObjectName`
MODBUS_EXPORT void cCpoSetObjectName(cModbusClientPort clientPort, const Char *name);

/// \details Wrapper for `ModbusClientPort::type`
MODBUS_EXPORT ProtocolType cCpoGetType(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::isOpen`
MODBUS_EXPORT bool cCpoIsOpen(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::close`
MODBUS_EXPORT bool cCpoClose(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::repeatCount`
MODBUS_EXPORT uint32_t cCpoGetRepeatCount(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::setRepeatCount`
MODBUS_EXPORT void cCpoSetRepeatCount(cModbusClientPort clientPort, uint32_t count);

#ifndef MBF_READ_COILS_DISABLE
/// \details Wrapper for `ModbusClientPort::readCoils`
MODBUS_EXPORT StatusCode cCpoReadCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
/// \details Wrapper for `ModbusClientPort::readDiscreteInputs`
MODBUS_EXPORT StatusCode cCpoReadDiscreteInputs(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
/// \details Wrapper for `ModbusClientPort::readHoldingRegisters`
MODBUS_EXPORT StatusCode cCpoReadHoldingRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
/// \details Wrapper for `ModbusClientPort::readInputRegisters`
MODBUS_EXPORT StatusCode cCpoReadInputRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
/// \details Wrapper for `ModbusClientPort::writeSingleCoil`
MODBUS_EXPORT StatusCode cCpoWriteSingleCoil(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, bool value);
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
/// \details Wrapper for `ModbusClientPort::writeSingleRegister`
MODBUS_EXPORT StatusCode cCpoWriteSingleRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t value);
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
/// \details Wrapper for `ModbusClientPort::readExceptionStatus`
MODBUS_EXPORT StatusCode cCpoReadExceptionStatus(cModbusClientPort clientPort, uint8_t unit, uint8_t *value);
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
/// \details Wrapper for `ModbusClientPort::diagnostics`
MODBUS_EXPORT StatusCode cCpoDiagnostics(cModbusClientPort clientPort, uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata);
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
/// \details Wrapper for `ModbusClientPort::getCommEventCounter`
MODBUS_EXPORT StatusCode cCpoGetCommEventCounter(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount);
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
/// \details Wrapper for `ModbusClientPort::getCommEventLog`
MODBUS_EXPORT StatusCode cCpoGetCommEventLog(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
/// \details Wrapper for `ModbusClientPort::writeMultipleCoils`
MODBUS_EXPORT StatusCode cCpoWriteMultipleCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const void *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
/// \details Wrapper for `ModbusClientPort::writeMultipleRegisters`
MODBUS_EXPORT StatusCode cCpoWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
/// \details Wrapper for `ModbusClientPort::reportServerID`
MODBUS_EXPORT StatusCode cCpoReportServerID(cModbusClientPort clientPort, uint8_t unit, uint8_t *count, uint8_t *data);
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
/// \details Wrapper for `ModbusClientPort::maskWriteRegister`
MODBUS_EXPORT StatusCode cCpoMaskWriteRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask);
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
/// \details Wrapper for `ModbusClientPort::readWriteMultipleRegisters`
MODBUS_EXPORT StatusCode cCpoReadWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
/// \details Wrapper for `ModbusClientPort::readFIFOQueue`
MODBUS_EXPORT StatusCode cCpoReadFIFOQueue(cModbusClientPort clientPort, uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values);
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
/// \details Wrapper for `ModbusClientPort::readCoilsAsBoolArray`
MODBUS_EXPORT StatusCode cCpoReadCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
/// \details Wrapper for `ModbusClientPort::readDiscreteInputsAsBoolArray`
MODBUS_EXPORT StatusCode cCpoReadDiscreteInputsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values);
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
/// \details Wrapper for `ModbusClientPort::writeMultipleCoilsAsBoolArray`
MODBUS_EXPORT StatusCode cCpoWriteMultipleCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const bool *values);
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

/// \details Wrapper for `ModbusClientPort::getLastStatus`
MODBUS_EXPORT StatusCode cCpoGetLastStatus(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::getLastErrorStatus`
MODBUS_EXPORT StatusCode cCpoGetLastErrorStatus(cModbusClientPort clientPort);

/// \details Wrapper for `ModbusClientPort::getLastErrorText`
MODBUS_EXPORT const Char *cCpoGetLastErrorText(cModbusClientPort clientPort);

/// \details Connects `funcPtr`-function to `ModbusClientPort::signalOpened` signal
MODBUS_EXPORT void cCpoConnectOpened(cModbusClientPort clientPort, pfSlotOpened funcPtr);

/// \details Connects `funcPtr`-function to `ModbusClientPort::signalClosed` signal
MODBUS_EXPORT void cCpoConnectClosed(cModbusClientPort clientPort, pfSlotClosed funcPtr);

/// \details Connects `funcPtr`-function to `ModbusClientPort::signalTx` signal
MODBUS_EXPORT void cCpoConnectTx(cModbusClientPort clientPort, pfSlotTx funcPtr);

/// \details Connects `funcPtr`-function to `ModbusClientPort::signalRx` signal
MODBUS_EXPORT void cCpoConnectRx(cModbusClientPort clientPort, pfSlotRx funcPtr);

/// \details Connects `funcPtr`-function to `ModbusClientPort::signalError` signal
MODBUS_EXPORT void cCpoConnectError(cModbusClientPort clientPort, pfSlotError funcPtr);

/// \details Disconnects `funcPtr`-function from `ModbusClientPort`
MODBUS_EXPORT void cCpoDisconnectFunc(cModbusClientPort clientPort, void *funcPtr);


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------- ModbusClient ---------------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \details Creates `ModbusClient` object and returns handle to it. \sa `Modbus::createClient`
MODBUS_EXPORT cModbusClient cCliCreate(uint8_t unit, ProtocolType type, const void *settings, bool blocking);

/// \details Creates `ModbusClient` object with `unit` for port `clientPort` and returns handle to it.
MODBUS_EXPORT cModbusClient cCliCreateForClientPort(uint8_t unit, cModbusClientPort clientPort);

/// \details Deletes previously created `ModbusClient` object represented by `client` handle
MODBUS_EXPORT void cCliDelete(cModbusClient client);

/// \details Wrapper for `ModbusClient::objectName`
MODBUS_EXPORT const Char *cCliGetObjectName(cModbusClient client);

/// \details Wrapper for `ModbusClient::setObjectName`
MODBUS_EXPORT void cCliSetObjectName(cModbusClient client, const Char *name);

/// \details Wrapper for `ModbusClient::type`
MODBUS_EXPORT ProtocolType cCliGetType(cModbusClient client);

/// \details Wrapper for `ModbusClient::unit`
MODBUS_EXPORT uint8_t cCliGetUnit(cModbusClient client);

/// \details Wrapper for `ModbusClient::setUnit`
MODBUS_EXPORT void cCliSetUnit(cModbusClient client, uint8_t unit);

/// \details Wrapper for `ModbusClient::isOpen`
MODBUS_EXPORT bool cCliIsOpen(cModbusClient client);

/// \details Wrapper for `ModbusClient::port`
MODBUS_EXPORT cModbusClientPort cCliGetPort(cModbusClient client);

/// \details Wrapper for `ModbusClient::readCoils`
MODBUS_EXPORT StatusCode cReadCoils(cModbusClient client, uint16_t offset, uint16_t count, void *values);

/// \details Wrapper for `ModbusClient::readDiscreteInputs`
MODBUS_EXPORT StatusCode cReadDiscreteInputs(cModbusClient client, uint16_t offset, uint16_t count, void *values);

/// \details Wrapper for `ModbusClient::readHoldingRegisters`
MODBUS_EXPORT StatusCode cReadHoldingRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values);

/// \details Wrapper for `ModbusClient::readInputRegisters`
MODBUS_EXPORT StatusCode cReadInputRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values);

/// \details Wrapper for `ModbusClient::writeSingleCoil`
MODBUS_EXPORT StatusCode cWriteSingleCoil(cModbusClient client, uint16_t offset, bool value);

/// \details Wrapper for `ModbusClient::writeSingleRegister`
MODBUS_EXPORT StatusCode cWriteSingleRegister(cModbusClient client, uint16_t offset, uint16_t value);

/// \details Wrapper for `ModbusClient::readExceptionStatus`
MODBUS_EXPORT StatusCode cReadExceptionStatus(cModbusClient client, uint8_t *value);

/// \details Wrapper for `ModbusClient::writeMultipleCoils`
MODBUS_EXPORT StatusCode cWriteMultipleCoils(cModbusClient client, uint16_t offset, uint16_t count, const void *values);

/// \details Wrapper for `ModbusClient::writeMultipleRegisters`
MODBUS_EXPORT StatusCode cWriteMultipleRegisters(cModbusClient client, uint16_t offset, uint16_t count, const uint16_t *values);

/// \details Wrapper for `ModbusClient::maskWriteRegister`
MODBUS_EXPORT StatusCode cMaskWriteRegister(cModbusClient client, uint16_t offset, uint16_t andMask, uint16_t orMask);

/// \details Wrapper for `ModbusClient::readWriteMultipleRegisters`
MODBUS_EXPORT StatusCode cReadWriteMultipleRegisters(cModbusClient client, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);

/// \details Wrapper for `ModbusClient::readCoilsAsBoolArray`
MODBUS_EXPORT StatusCode cReadCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values);

/// \details Wrapper for `ModbusClient::readDiscreteInputsAsBoolArray`
MODBUS_EXPORT StatusCode cReadDiscreteInputsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values);

/// \details Wrapper for `ModbusClient::lastPortStatus`
MODBUS_EXPORT StatusCode cWriteMultipleCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, const bool *values);

/// \details Wrapper for `ModbusClient::lastPortStatus`
MODBUS_EXPORT StatusCode cCliGetLastPortStatus(cModbusClient client);

/// \details Wrapper for `ModbusClient::lastPortErrorStatus`
MODBUS_EXPORT StatusCode cCliGetLastPortErrorStatus(cModbusClient client);

/// \details Wrapper for `ModbusClient::lastPortErrorText`
MODBUS_EXPORT const Char *cCliGetLastPortErrorText(cModbusClient client);

#endif // MB_CLIENT_DISABLE

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusServerPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

#ifndef MB_SERVER_DISABLE

/// \details Creates `ModbusServerPort` object and returns handle to it. \sa `Modbus::createServerPort`
MODBUS_EXPORT cModbusServerPort cSpoCreate(cModbusInterface device, ProtocolType type, const void *settings, bool blocking);

/// \details Deletes previously created `ModbusServerPort` object represented by `serverPort` handle
MODBUS_EXPORT void cSpoDelete(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::objectName`
MODBUS_EXPORT const Char *cSpoGetObjectName(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::setObjectName`
MODBUS_EXPORT void cSpoSetObjectName(cModbusServerPort serverPort, const Char *name);

/// \details Wrapper for `ModbusServerPort::type`
MODBUS_EXPORT ProtocolType cSpoGetType(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::isTcpServer`
MODBUS_EXPORT bool cSpoIsTcpServer(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::device`
MODBUS_EXPORT cModbusInterface cSpoGetDevice(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::isOpen`
MODBUS_EXPORT bool cSpoIsOpen(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::open`
MODBUS_EXPORT StatusCode cSpoOpen(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::close`
MODBUS_EXPORT StatusCode cSpoClose(cModbusServerPort serverPort);

/// \details Wrapper for `ModbusServerPort::process`
MODBUS_EXPORT StatusCode cSpoProcess(cModbusServerPort serverPort);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalOpened` signal
MODBUS_EXPORT void cSpoConnectOpened(cModbusServerPort serverPort, pfSlotOpened funcPtr);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalClosed` signal
MODBUS_EXPORT void cSpoConnectClosed(cModbusServerPort serverPort, pfSlotClosed funcPtr);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalTx` signal
MODBUS_EXPORT void cSpoConnectTx(cModbusServerPort serverPort, pfSlotTx funcPtr);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalRx` signal
MODBUS_EXPORT void cSpoConnectRx(cModbusServerPort serverPort, pfSlotRx funcPtr);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalError` signal
MODBUS_EXPORT void cSpoConnectError(cModbusServerPort serverPort, pfSlotError funcPtr);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalNewConnection` signal
MODBUS_EXPORT void cSpoConnectNewConnection(cModbusServerPort serverPort, pfSlotNewConnection funcPtr);

/// \details Connects `funcPtr`-function to `ModbusServerPort::signalCloseConnection` signal
MODBUS_EXPORT void cSpoConnectCloseConnection(cModbusServerPort serverPort, pfSlotCloseConnection funcPtr);

/// \details Disconnects `funcPtr`-function from `ModbusServerPort`
MODBUS_EXPORT void cSpoDisconnectFunc(cModbusServerPort serverPort, void *funcPtr);

#endif // MB_SERVER_DISABLE

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CMODBUS_H
