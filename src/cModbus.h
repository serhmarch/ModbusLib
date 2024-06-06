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
class ModbusClientPort;
class ModbusClient    ;
class ModbusServerPort;
class ModbusInterface ;

#else
typedef struct ModbusPort       ModbusPort      ;
typedef struct ModbusClientPort ModbusClientPort;
typedef struct ModbusClient     ModbusClient    ;
typedef struct ModbusServerPort ModbusServerPort;
typedef struct ModbusInterface  ModbusInterface ;
#endif

typedef ModbusPort      * cModbusPort      ;
typedef ModbusClientPort* cModbusClientPort;
typedef ModbusClient    * cModbusClient    ;
typedef ModbusServerPort* cModbusServerPort;
typedef ModbusInterface * cModbusInterface ;

typedef void* cModbusDevice;

typedef StatusCode (*pfReadCoils             )(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, void *values);
typedef StatusCode (*pfReadDiscreteInputs    )(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, void *values);
typedef StatusCode (*pfReadHoldingRegisters  )(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
typedef StatusCode (*pfReadInputRegisters    )(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
typedef StatusCode (*pfWriteSingleCoil       )(cModbusDevice dev, uint8_t unit, uint16_t offset, bool value);
typedef StatusCode (*pfWriteSingleRegister   )(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t value);
typedef StatusCode (*pfReadExceptionStatus   )(cModbusDevice dev, uint8_t unit, uint8_t *status);
typedef StatusCode (*pfWriteMultipleCoils    )(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, const void *values);
typedef StatusCode (*pfWriteMultipleRegisters)(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);

typedef void (*pfSlotOpened         )(const Char *source);
typedef void (*pfSlotClosed         )(const Char *source);
typedef void (*pfSlotTx             )(const Char *source, const uint8_t* buff, uint16_t size);
typedef void (*pfSlotRx             )(const Char *source, const uint8_t* buff, uint16_t size);
typedef void (*pfSlotError          )(const Char *source, StatusCode status, const Char *text);
typedef void (*pfSlotNewConnection  )(const Char *source);
typedef void (*pfSlotCloseConnection)(const Char *source);

/// \details 
MODBUS_EXPORT cModbusInterface cCreateModbusDevice(cModbusDevice            device                , 
                                                   pfReadCoils              readCoils             ,
                                                   pfReadDiscreteInputs     readDiscreteInputs    ,
                                                   pfReadHoldingRegisters   readHoldingRegisters  ,
                                                   pfReadInputRegisters     readInputRegisters    ,
                                                   pfWriteSingleCoil        writeSingleCoil       ,
                                                   pfWriteSingleRegister    writeSingleRegister   ,
                                                   pfReadExceptionStatus    readExceptionStatus   ,
                                                   pfWriteMultipleCoils     writeMultipleCoils    ,
                                                   pfWriteMultipleRegisters writeMultipleRegisters);


/// \details
MODBUS_EXPORT void cDeleteModbusDevice(cModbusInterface dev);

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------- ModbusPort ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \details
MODBUS_EXPORT cModbusPort cPortCreate(ProtocolType type, const void *settings, bool blocking);

/// \details 
MODBUS_EXPORT void cPortDelete(cModbusPort port);


// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusClientPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \details 
MODBUS_EXPORT cModbusClientPort cCpoCreate(ProtocolType type, const void *settings, bool blocking);

/// \details 
MODBUS_EXPORT cModbusClientPort cCpoCreateForPort(cModbusPort port);

/// \details 
MODBUS_EXPORT void cCpoDelete(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT const Char *cCpoGetObjectName(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT void cCpoSetObjectName(cModbusClientPort clientPort, const Char *name);

/// \details
MODBUS_EXPORT ProtocolType cCpoGetType(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT bool cCpoIsOpen(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT bool cCpoClose(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT uint32_t cCpoGetRepeatCount(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT void cCpoSetRepeatCount(cModbusClientPort clientPort, uint32_t count);

/// \details
MODBUS_EXPORT StatusCode cCpoReadCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values);

/// \details
MODBUS_EXPORT StatusCode cCpoReadDiscreteInputs(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values);

/// \details
MODBUS_EXPORT StatusCode cCpoReadHoldingRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);

/// \details
MODBUS_EXPORT StatusCode cCpoReadInputRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);

/// \details
MODBUS_EXPORT StatusCode cCpoWriteSingleCoil(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, bool value);

/// \details
MODBUS_EXPORT StatusCode cCpoWriteSingleRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t value);

/// \details
MODBUS_EXPORT StatusCode cCpoReadExceptionStatus(cModbusClientPort clientPort, uint8_t unit, uint8_t *value);

/// \details
MODBUS_EXPORT StatusCode cCpoWriteMultipleCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const void *values);

/// \details
MODBUS_EXPORT StatusCode cCpoWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);

/// \details
MODBUS_EXPORT StatusCode cCpoReadCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values);

/// \details
MODBUS_EXPORT StatusCode cCpoReadDiscreteInputsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values);

/// \details
MODBUS_EXPORT StatusCode cCpoWriteMultipleCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const bool *values);

/// \details
MODBUS_EXPORT StatusCode cCpoGetLastStatus(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT StatusCode cCpoGetLastErrorStatus(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT const Char *cCpoGetLastErrorText(cModbusClientPort clientPort);

/// \details
MODBUS_EXPORT void cCpoConnectOpened(cModbusClientPort clientPort, pfSlotOpened funcPtr);

/// \details
MODBUS_EXPORT void cCpoConnectClosed(cModbusClientPort clientPort, pfSlotClosed funcPtr);

/// \details
MODBUS_EXPORT void cCpoConnectTx(cModbusClientPort clientPort, pfSlotTx funcPtr);

/// \details
MODBUS_EXPORT void cCpoConnectRx(cModbusClientPort clientPort, pfSlotRx funcPtr);

/// \details
MODBUS_EXPORT void cCpoConnectError(cModbusClientPort clientPort, pfSlotError funcPtr);

/// \details
MODBUS_EXPORT void cCpoDisconnectFunc(cModbusClientPort clientPort, void *funcPtr);


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------- ModbusClient ---------------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \details 
MODBUS_EXPORT cModbusClient cCliCreate(uint8_t unit, ProtocolType type, const void *settings, bool blocking);

/// \details 
MODBUS_EXPORT cModbusClient cCliCreateForClientPort(uint8_t unit, cModbusClientPort clientPort);

/// \details 
MODBUS_EXPORT void cCliDelete(cModbusClient client);

/// \details
MODBUS_EXPORT const Char *cCliGetObjectName(cModbusClient client);

/// \details
MODBUS_EXPORT void cCliSetObjectName(cModbusClient client, const Char *name);

/// \details
MODBUS_EXPORT ProtocolType cCliGetType(cModbusClient client);

/// \details
MODBUS_EXPORT uint8_t cCliGetUnit(cModbusClient client);

/// \details
MODBUS_EXPORT void cCliSetUnit(cModbusClient client, uint8_t unit);

/// \details
MODBUS_EXPORT bool cCliIsOpen(cModbusClient client);

/// \details
MODBUS_EXPORT cModbusClientPort cCliGetPort(cModbusClient client);

/// \details
MODBUS_EXPORT StatusCode cReadCoils(cModbusClient client, uint16_t offset, uint16_t count, void *values);

/// \details
MODBUS_EXPORT StatusCode cReadDiscreteInputs(cModbusClient client, uint16_t offset, uint16_t count, void *values);

/// \details
MODBUS_EXPORT StatusCode cReadHoldingRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values);

/// \details
MODBUS_EXPORT StatusCode cReadInputRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values);

/// \details
MODBUS_EXPORT StatusCode cWriteSingleCoil(cModbusClient client, uint16_t offset, bool value);

/// \details
MODBUS_EXPORT StatusCode cWriteSingleRegister(cModbusClient client, uint16_t offset, uint16_t value);

/// \details
MODBUS_EXPORT StatusCode cReadExceptionStatus(cModbusClient client, uint8_t *value);

/// \details
MODBUS_EXPORT StatusCode cWriteMultipleCoils(cModbusClient client, uint16_t offset, uint16_t count, const void *values);

/// \details
MODBUS_EXPORT StatusCode cWriteMultipleRegisters(cModbusClient client, uint16_t offset, uint16_t count, const uint16_t *values);

/// \details
MODBUS_EXPORT StatusCode cReadCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values);

/// \details
MODBUS_EXPORT StatusCode cReadDiscreteInputsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values);

/// \details
MODBUS_EXPORT StatusCode cWriteMultipleCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, const bool *values);

/// \details
MODBUS_EXPORT StatusCode cCliGetLastPortStatus(cModbusClient client);

/// \details
MODBUS_EXPORT StatusCode cCliGetLastPortErrorStatus(cModbusClient client);

/// \details
MODBUS_EXPORT const Char *cCliGetLastPortErrorText(cModbusClient client);


// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusServerPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

/// \details 
MODBUS_EXPORT cModbusServerPort cSpoCreate(cModbusInterface device, ProtocolType type, const void *settings, bool blocking);

/// \details 
MODBUS_EXPORT void cSpoDelete(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT const Char *cSpoGetObjectName(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT void cSpoSetObjectName(cModbusServerPort serverPort, const Char *name);

/// \details
MODBUS_EXPORT ProtocolType cSpoGetType(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT bool cSpoIsTcpServer(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT cModbusInterface cSpoGetDevice(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT bool cSpoIsOpen(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT StatusCode cSpoOpen(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT StatusCode cSpoClose(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT StatusCode cSpoProcess(cModbusServerPort serverPort);

/// \details
MODBUS_EXPORT void cSpoConnectOpened(cModbusServerPort serverPort, pfSlotOpened funcPtr);

/// \details
MODBUS_EXPORT void cSpoConnectClosed(cModbusServerPort serverPort, pfSlotClosed funcPtr);

/// \details
MODBUS_EXPORT void cSpoConnectTx(cModbusServerPort serverPort, pfSlotTx funcPtr);

/// \details
MODBUS_EXPORT void cSpoConnectRx(cModbusServerPort serverPort, pfSlotRx funcPtr);

/// \details
MODBUS_EXPORT void cSpoConnectError(cModbusServerPort serverPort, pfSlotError funcPtr);

/// \details
MODBUS_EXPORT void cSpoConnectNewConnection(cModbusServerPort serverPort, pfSlotNewConnection funcPtr);

/// \details
MODBUS_EXPORT void cSpoConnectCloseConnection(cModbusServerPort serverPort, pfSlotCloseConnection funcPtr);

/// \details
MODBUS_EXPORT void cSpoDisconnectFunc(cModbusServerPort serverPort, void *funcPtr);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // CMODBUS_H
