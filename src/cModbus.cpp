#include "cModbus.h"

#include "Modbus.h"
#include "ModbusPort.h"
#include "ModbusClientPort.h"
#include "ModbusClient.h"
#include "ModbusServerPort.h"
#include "ModbusTcpServer.h"

class cModbusInterfaceImpl : public ModbusInterface
{
public:
    cModbusInterfaceImpl                          (cModbusDevice                device
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
    ) :
                                                 m_device(device)
#ifndef MBF_READ_COILS_DISABLE
                                                ,m_readCoils                  (readCoils)            
#endif // MBF_READ_COILS_DISABLE
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
                                                ,m_readDiscreteInputs         (readDiscreteInputs)    
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE
#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
                                                ,m_readHoldingRegisters       (readHoldingRegisters)  
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
                                                , m_readInputRegisters         (readInputRegisters)    
#endif // MBF_READ_INPUT_REGISTERS_DISABLE
#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
                                                , m_writeSingleCoil            (writeSingleCoil)       
#endif // MBF_WRITE_SINGLE_COIL_DISABLE
#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
                                                , m_writeSingleRegister        (writeSingleRegister)   
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE
#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
                                                , m_readExceptionStatus        (readExceptionStatus)   
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE
#ifndef MBF_DIAGNOSTICS_DISABLE
                                                , m_diagnostics                (diagnostics)    
#endif // MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
                                                , m_getCommEventCounter        (getCommEventCounter)
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE
#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
                                                , m_getCommEventLog            (getCommEventLog)     
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE
#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
                                                , m_writeMultipleCoils         (writeMultipleCoils)
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                , m_writeMultipleRegisters     (writeMultipleRegisters)
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
#ifndef MBF_REPORT_SERVER_ID_DISABLE
                                                , m_reportServerID             (reportServerID)
#endif // MBF_REPORT_SERVER_ID_DISABLE
#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
                                                , m_maskWriteRegister          (maskWriteRegister)
#endif // MBF_MASK_WRITE_REGISTER_DISABLE
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                , m_readWriteMultipleRegisters (readWriteMultipleRegisters)
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
#ifndef MBF_READ_FIFO_QUEUE_DISABLE
                                                , m_readFIFOQueue              (readFIFOQueue)
#endif // MBF_READ_FIFO_QUEUE_DISABLE
    {
    }

    virtual ~cModbusInterfaceImpl()
    {
    }

public:
#ifndef MBF_READ_COILS_DISABLE
    StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        if (m_readCoils)
            return m_readCoils(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        if (m_readDiscreteInputs)
            return m_readDiscreteInputs(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
    {
        if (m_readHoldingRegisters)
            return m_readHoldingRegisters(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
    {
        if (m_readInputRegisters)
            return m_readInputRegisters(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
    {
        if (m_writeSingleCoil)
            return m_writeSingleCoil(m_device, unit, offset, value);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
    {
        if (m_writeSingleRegister)
            return m_writeSingleRegister(m_device, unit, offset, value);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    StatusCode readExceptionStatus(uint8_t unit, uint8_t *status)
    {
        if (m_readExceptionStatus)
            return m_readExceptionStatus(m_device, unit, status);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    StatusCode diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata)
    {
        if (m_diagnostics)
            return m_diagnostics(m_device, unit, subfunc, insize, indata, outsize, outdata);
        return Status_BadIllegalFunction;
    }
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    StatusCode getCommEventCounter(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount)
    {
        if (m_getCommEventCounter)
            return m_getCommEventCounter(m_device, unit, status, eventCount);
        return Status_BadIllegalFunction;
    }
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    StatusCode getCommEventLog(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
    {
        if (m_getCommEventLog)
            return m_getCommEventLog(m_device, unit, status, eventCount, messageCount, eventBuffSize, eventBuff);
        return Status_BadIllegalFunction;
    }
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
    {
        if (m_writeMultipleCoils)
            return m_writeMultipleCoils(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
    {
        if (m_writeMultipleRegisters)
            return m_writeMultipleRegisters(m_device, unit, offset, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    StatusCode reportServerID(uint8_t unit, uint8_t *count, uint8_t *data)
    {
        if (m_reportServerID)
            return m_reportServerID(m_device, unit, count, data);
        return Status_BadIllegalFunction;
    }
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
    {
        if (m_writeSingleRegister)
            return m_maskWriteRegister(m_device, unit, offset, andMask, orMask);
        return Status_BadIllegalFunction;
    }
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
    {
        if (m_readWriteMultipleRegisters)
            return m_readWriteMultipleRegisters(m_device, unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)
    {
        if (m_readFIFOQueue)
            return m_readFIFOQueue(m_device, unit, fifoadr, count, values);
        return Status_BadIllegalFunction;
    }
#endif // MBF_READ_FIFO_QUEUE_DISABLE

private:
    cModbusDevice m_device;

#ifndef MBF_READ_COILS_DISABLE
                                                 pfReadCoils                  m_readCoils                   ;
#endif // MBF_READ_COILS_DISABLE        
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE        
                                                 pfReadDiscreteInputs         m_readDiscreteInputs          ;
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE      
#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE      
                                                 pfReadHoldingRegisters       m_readHoldingRegisters        ;
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE        
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE        
                                                 pfReadInputRegisters         m_readInputRegisters          ;
#endif // MBF_READ_INPUT_REGISTERS_DISABLE      
#ifndef MBF_WRITE_SINGLE_COIL_DISABLE       
                                                 pfWriteSingleCoil            m_writeSingleCoil             ;
#endif // MBF_WRITE_SINGLE_COIL_DISABLE     
#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE       
                                                 pfWriteSingleRegister        m_writeSingleRegister         ;
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE     
#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE       
                                                 pfReadExceptionStatus        m_readExceptionStatus         ;
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE     
#ifndef MBF_DIAGNOSTICS_DISABLE     
                                                 pfDiagnostics                m_diagnostics                 ;
#endif // MBF_DIAGNOSTICS_DISABLE       
#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE      
                                                 pfGetCommEventCounter        m_getCommEventCounter         ;
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE        
#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE      
                                                 pfGetCommEventLog            m_getCommEventLog             ;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE        
#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE        
                                                 pfWriteMultipleCoils         m_writeMultipleCoils          ;
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE      
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE        
                                                 pfWriteMultipleRegisters     m_writeMultipleRegisters      ;
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE      
#ifndef MBF_REPORT_SERVER_ID_DISABLE        
                                                 pfReportServerID             m_reportServerID              ;
#endif // MBF_REPORT_SERVER_ID_DISABLE      
#ifndef MBF_MASK_WRITE_REGISTER_DISABLE     
                                                 pfMaskWriteRegister          m_maskWriteRegister           ;
#endif // MBF_MASK_WRITE_REGISTER_DISABLE
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                 pfReadWriteMultipleRegisters m_readWriteMultipleRegisters  ;
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
#ifndef MBF_READ_FIFO_QUEUE_DISABLE
                                                 pfReadFIFOQueue              m_readFIFOQueue               ;
#endif // MBF_READ_FIFO_QUEUE_DISABLE
};

cModbusInterface cCreateModbusDevice(cModbusDevice device
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
    )
{
    return new cModbusInterfaceImpl(device
#ifndef MBF_READ_COILS_DISABLE
                                                 , readCoils             
#endif // MBF_READ_COILS_DISABLE
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
                                                 , readDiscreteInputs    
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE
#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
                                                 , readHoldingRegisters  
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
                                                 , readInputRegisters    
#endif // MBF_READ_INPUT_REGISTERS_DISABLE
#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
                                                 , writeSingleCoil       
#endif // MBF_WRITE_SINGLE_COIL_DISABLE
#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
                                                 , writeSingleRegister   
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE
#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
                                                 , readExceptionStatus   
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE
#ifndef MBF_DIAGNOSTICS_DISABLE
                                                 , diagnostics    
#endif // MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
                                                 , getCommEventCounter
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE
#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
                                                 , getCommEventLog     
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE
#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
                                                 , writeMultipleCoils
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                 , writeMultipleRegisters
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
#ifndef MBF_REPORT_SERVER_ID_DISABLE
                                                 , reportServerID
#endif // MBF_REPORT_SERVER_ID_DISABLE
#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
                                                 , maskWriteRegister
#endif // MBF_MASK_WRITE_REGISTER_DISABLE
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
                                                 , readWriteMultipleRegisters
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABL
#ifndef MBF_READ_FIFO_QUEUE_DISABLE
                                                 , readFIFOQueue
#endif // MBF_READ_FIFO_QUEUE_DISABLE
                                    );
}

void cDeleteModbusDevice(cModbusInterface dev)
{
    delete static_cast<cModbusInterfaceImpl*>(dev);
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------- ModbusPort ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

cModbusPort cPortCreate(ProtocolType type, const void *settings, bool blocking)
{
    return createPort(type, settings, blocking);
}

void cPortDelete(cModbusPort port)
{
    delete port;
}

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusClientPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

cModbusClientPort cCpoCreate(ProtocolType type, const void *settings, bool blocking)
{
    return createClientPort(type, settings, blocking);
}

cModbusClientPort cCpoCreateForPort(cModbusPort port)
{
    return new ModbusClientPort(port);
}

void cCpoDelete(cModbusClientPort clientPort)
{
    delete clientPort;
}

const Char *cCpoGetObjectName(cModbusClientPort clientPort)
{
    return clientPort->objectName();
}

void cCpoSetObjectName(cModbusClientPort clientPort, const Char *name)
{
    clientPort->setObjectName(name);
}

ProtocolType cCpoGetType(cModbusClientPort clientPort)
{
    return clientPort->type();
}

bool cCpoIsOpen(cModbusClientPort clientPort)
{
    return clientPort->isOpen();
}

bool cCpoClose(cModbusClientPort clientPort)
{
    return clientPort->close();
}

uint32_t cCpoGetRepeatCount(cModbusClientPort clientPort)
{
    return clientPort->close();
}

void cCpoSetRepeatCount(cModbusClientPort clientPort, uint32_t count)
{
    clientPort->setRepeatCount(count);
}

#ifndef MBF_READ_COILS_DISABLE
StatusCode cCpoReadCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return clientPort->readCoils(unit, offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cCpoReadDiscreteInputs(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    return clientPort->readDiscreteInputs(unit, offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode cCpoReadHoldingRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return clientPort->readHoldingRegisters(unit, offset, count, values);
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode cCpoReadInputRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    return clientPort->readInputRegisters(unit, offset, count, values);
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode cCpoWriteSingleCoil(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, bool value)
{
    return clientPort->writeSingleCoil(unit, offset, value);
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode cCpoWriteSingleRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t value)
{
    return clientPort->writeSingleRegister(unit, offset, value);
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode cCpoReadExceptionStatus(cModbusClientPort clientPort, uint8_t unit, uint8_t *value)
{
    return clientPort->readExceptionStatus(unit, value);
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
StatusCode cCpoDiagnostics(cModbusClientPort clientPort, uint8_t unit, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata)
{
    return clientPort->diagnostics(unit, subfunc, insize, indata, outsize, outdata);
}
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
StatusCode cCpoGetCommEventCounter(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    return clientPort->getCommEventCounter(unit, status, eventCount);
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
StatusCode cCpoGetCommEventLog(cModbusClientPort clientPort, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    return clientPort->getCommEventLog(unit, status, eventCount, messageCount, eventBuffSize, eventBuff);
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cCpoWriteMultipleCoils(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    return clientPort->writeMultipleCoils(unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cCpoWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    return clientPort->writeMultipleRegisters(unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode cCpoReportServerID(cModbusClientPort clientPort, uint8_t unit, uint8_t *count, uint8_t *data)
{
    return clientPort->reportServerID(unit, count, data);
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode cCpoMaskWriteRegister(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    return clientPort->maskWriteRegister(unit, offset, andMask, orMask);
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cCpoReadWriteMultipleRegisters(cModbusClientPort clientPort, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    return clientPort->readWriteMultipleRegisters(unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
StatusCode cCpoReadFIFOQueue(cModbusClientPort clientPort, uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    return clientPort->readFIFOQueue(unit, fifoadr, count, values);
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
StatusCode cCpoReadCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    return clientPort->readCoilsAsBoolArray(unit, offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cCpoReadDiscreteInputsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, bool *values)
{
    return clientPort->readDiscreteInputsAsBoolArray(unit, offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cCpoWriteMultipleCoilsAsBoolArray(cModbusClientPort clientPort, uint8_t unit, uint16_t offset, uint16_t count, const bool *values)
{
    return clientPort->writeMultipleCoilsAsBoolArray(unit, offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

StatusCode cCpoGetLastStatus(cModbusClientPort clientPort)
{
    return clientPort->lastStatus();
}

StatusCode cCpoGetLastErrorStatus(cModbusClientPort clientPort)
{
    return clientPort->lastErrorStatus();
}

const Char *cCpoGetLastErrorText(cModbusClientPort clientPort)
{
    return clientPort->lastErrorText();
}

void cCpoConnectOpened(cModbusClientPort clientPort, pfSlotOpened funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalOpened, funcPtr);
}

void cCpoConnectClosed(cModbusClientPort clientPort, pfSlotClosed funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalClosed, funcPtr);
}

void cCpoConnectTx(cModbusClientPort clientPort, pfSlotTx funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalTx, funcPtr);
}

void cCpoConnectRx(cModbusClientPort clientPort, pfSlotRx funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalRx, funcPtr);
}

void cCpoConnectError(cModbusClientPort clientPort, pfSlotError funcPtr)
{
    clientPort->connect(&ModbusClientPort::signalError, funcPtr);
}

void cCpoDisconnectFunc(cModbusClientPort clientPort, void *funcPtr)
{
    clientPort->disconnectFunc(funcPtr);
}


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------- ModbusClient ---------------------------------------------
// --------------------------------------------------------------------------------------------------------

cModbusClient cCliCreate(uint8_t unit, ProtocolType type, const void *settings, bool blocking)
{
    ModbusClientPort *clientPort = createClientPort(type, settings, blocking);
    ModbusClient *client = new ModbusClient(unit, clientPort);
    return client;
}

cModbusClient cCliCreateForClientPort(uint8_t unit, cModbusClientPort clientPort)
{
    ModbusClient *client = new ModbusClient(unit, clientPort);
    return client;
}

void cCliDelete(cModbusClient client)
{
    delete client;
}

const Char *cCliGetObjectName(cModbusClient client)
{
    return client->objectName();
}

void cCliSetObjectName(cModbusClient client, const Char *name)
{
    client->setObjectName(name);
}

ProtocolType cCliGetType(cModbusClient client)
{
    return client->type();
}

uint8_t cCliGetUnit(cModbusClient client)
{
    return client->unit();
}

void cCliSetUnit(cModbusClient client, uint8_t unit)
{
    client->setUnit(unit);
}

bool cCliIsOpen(cModbusClient client)
{
    return client->isOpen();
}

cModbusClientPort cCliGetPort(cModbusClient client)
{
    return client->port();
}

#ifndef MBF_READ_COILS_DISABLE
StatusCode cReadCoils(cModbusClient client, uint16_t offset, uint16_t count, void *values)
{
    return client->readCoils(offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cReadDiscreteInputs(cModbusClient client, uint16_t offset, uint16_t count, void *values)
{
    return client->readDiscreteInputs(offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode cReadHoldingRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values)
{
    return client->readHoldingRegisters(offset, count, values);
}
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode cReadInputRegisters(cModbusClient client, uint16_t offset, uint16_t count, uint16_t *values)
{
    return client->readInputRegisters(offset, count, values);
}
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode cWriteSingleCoil(cModbusClient client, uint16_t offset, bool value)
{
    return client->writeSingleCoil(offset, value);
}
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode cWriteSingleRegister(cModbusClient client, uint16_t offset, uint16_t value)
{
    return client->writeSingleRegister(offset, value);
}
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode cReadExceptionStatus(cModbusClient client, uint8_t *value)
{
    return client->readExceptionStatus(value);
}
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
StatusCode cDiagnostics(cModbusClient client, uint16_t subfunc, uint8_t insize, const uint8_t *indata, uint8_t *outsize, uint8_t *outdata)
{
    return client->diagnostics(subfunc, insize, indata, outsize, outdata);
}
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
StatusCode cGetCommEventCounter(cModbusClient client, uint16_t *status, uint16_t *eventCount)
{
    return client->getCommEventCounter(status, eventCount);
}
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
StatusCode cGetCommEventLog(cModbusClient client, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff)
{
    return client->getCommEventLog(status, eventCount, messageCount, eventBuffSize, eventBuff);
}
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cWriteMultipleCoils(cModbusClient client, uint16_t offset, uint16_t count, const void *values)
{
    return client->writeMultipleCoils(offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cWriteMultipleRegisters(cModbusClient client, uint16_t offset, uint16_t count, const uint16_t *values)
{
    return client->writeMultipleRegisters(offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode cReportServerID(cModbusClient client, uint8_t *count, uint8_t *data)
{
    return client->reportServerID(count, data);
}
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode cMaskWriteRegister(cModbusClient client, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    return client->maskWriteRegister(offset, andMask, orMask);
}
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cReadWriteMultipleRegisters(cModbusClient client, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    return client->readWriteMultipleRegisters(readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
}
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
StatusCode cReadFIFOQueue(cModbusClient client, uint16_t fifoadr, uint16_t *count, uint16_t *values)
{
    return client->readFIFOQueue(fifoadr, count, values);
}
#endif // MBF_READ_FIFO_QUEUE_DISABLE

#ifndef MBF_READ_COILS_DISABLE
StatusCode cReadCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values)
{
    return client->readCoilsAsBoolArray(offset, count, values);
}
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cReadDiscreteInputsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, bool *values)
{
    return client->readDiscreteInputsAsBoolArray(offset, count, values);
}
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cWriteMultipleCoilsAsBoolArray(cModbusClient client, uint16_t offset, uint16_t count, const bool *values)
{
    return client->writeMultipleCoilsAsBoolArray(offset, count, values);
}
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

StatusCode cCliGetLastPortStatus(cModbusClient client)
{
    return client->lastPortStatus();
}

StatusCode cCliGetLastPortErrorStatus(cModbusClient client)
{
    return client->lastPortErrorStatus();
}

const Char *cCliGetLastPortErrorText(cModbusClient client)
{
    return client->lastPortErrorText();
}


// --------------------------------------------------------------------------------------------------------
// ------------------------------------------- ModbusServerPort -------------------------------------------
// --------------------------------------------------------------------------------------------------------

cModbusServerPort cSpoCreate(cModbusInterface device, ProtocolType type, const void *settings, bool blocking)
{
    return createServerPort(device, type, settings, blocking);
}

void cSpoDelete(cModbusServerPort serverPort)
{
    delete serverPort;
}

const Char *cSpoGetObjectName(cModbusServerPort serverPort)
{
    return serverPort->objectName();
}

void cSpoSetObjectName(cModbusServerPort serverPort, const Char *name)
{
    serverPort->setObjectName(name);
}

ProtocolType cSpoGetType(cModbusServerPort serverPort)
{
    return serverPort->type();
}

bool cSpoIsTcpServer(cModbusServerPort serverPort)
{
    return serverPort->isTcpServer();
}

cModbusInterface cSpoGetDevice(cModbusServerPort serverPort)
{
    return serverPort->device();
}

bool cSpoIsOpen(cModbusServerPort serverPort)
{
    return serverPort->isOpen();
}

StatusCode cSpoOpen(cModbusServerPort serverPort)
{
    return serverPort->open();
}

StatusCode cSpoClose(cModbusServerPort serverPort)
{
    return serverPort->close();
}

StatusCode cSpoProcess(cModbusServerPort serverPort)
{
    return serverPort->process();
}

void cSpoConnectOpened(cModbusServerPort serverPort, pfSlotOpened funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalOpened, funcPtr);
}

void cSpoConnectClosed(cModbusServerPort serverPort, pfSlotClosed funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalClosed, funcPtr);
}

void cSpoConnectTx(cModbusServerPort serverPort, pfSlotTx funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalTx, funcPtr);
}

void cSpoConnectRx(cModbusServerPort serverPort, pfSlotRx funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalRx, funcPtr);
}

void cSpoConnectError(cModbusServerPort serverPort, pfSlotError funcPtr)
{
    serverPort->connect(&ModbusServerPort::signalError, funcPtr);
}

void cSpoConnectNewConnection(cModbusServerPort serverPort, pfSlotNewConnection funcPtr)
{
    if (serverPort->isTcpServer())
        static_cast<ModbusTcpServer*>(serverPort)->connect(&ModbusTcpServer::signalNewConnection, funcPtr);
}

void cSpoConnectCloseConnection(cModbusServerPort serverPort, pfSlotCloseConnection funcPtr)
{
    if (serverPort->isTcpServer())
        static_cast<ModbusTcpServer*>(serverPort)->connect(&ModbusTcpServer::signalCloseConnection, funcPtr);
}

void cSpoDisconnectFunc(cModbusServerPort serverPort, void *funcPtr)
{
    serverPort->disconnectFunc(funcPtr);
}
