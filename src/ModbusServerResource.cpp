/*
    Modbus

    Created: 2024
    Author: Serhii Marchuk, https://github.com/serhmarch

    Copyright (C) 2023  Serhii Marchuk

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "ModbusServerResource.h"
#include "ModbusServerResource_p.h"

inline ModbusServerResourcePrivate *d_cast(ModbusObjectPrivate *d_ptr) { return static_cast<ModbusServerResourcePrivate*>(d_ptr); }

ModbusServerResource::ModbusServerResource(ModbusPort *port, ModbusInterface *device) :
    ModbusServerPort(new ModbusServerResourcePrivate(port, device))
{
    port->setServerMode(true);
}

ModbusPort *ModbusServerResource::port() const
{
    return d_cast(d_ptr)->port;
}

ProtocolType ModbusServerResource::type() const
{
    return d_cast(d_ptr)->port->type();
}

StatusCode ModbusServerResource::open()
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    d->cmdClose = false;
    return Status_Good;
}

StatusCode ModbusServerResource::close()
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    d->cmdClose = true;
    return Status_Good;
}

bool ModbusServerResource::isOpen() const
{
    return d_cast(d_ptr)->port->isOpen();
}

uint32_t ModbusServerResource::timeout() const
{
    return d_cast(d_ptr)->port->timeout();
}

void ModbusServerResource::setTimeout(uint32_t timeout)
{
    d_cast(d_ptr)->port->setTimeout(timeout);
}

StatusCode ModbusServerResource::process()
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    const int szBuff = 500;

    StatusCode r = Status_Good;
    uint8_t buff[szBuff], func;
    uint16_t outBytes, outCount = 0;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_CLOSED:
            if (d->cmdClose)
                break;
            d->state = STATE_BEGIN_OPEN;
            // no need break
        case STATE_BEGIN_OPEN:
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_OPEN;
            // no need break
        case STATE_WAIT_FOR_OPEN:
            if (d->cmdClose)
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            r = d->port->open();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))  // an error occured
            {
                SET_PORT_ERROR(r);
                d->state = STATE_TIMEOUT;
                return r;
            }
            signalOpened(this->objectName());
            d->state = STATE_OPENED;
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = d->port->close();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))
                SET_PORT_ERROR(r);
            signalClosed(this->objectName());
            d->state = STATE_CLOSED;
            return r;
        case STATE_OPENED:
            d->state = STATE_BEGIN_READ;
            // no need break
        case STATE_BEGIN_READ:
            d->timestampRefresh();
            d->state = STATE_READ;
            // no need break
        case STATE_READ:
            if (d->cmdClose)
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            r = d->port->read();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r)) // an error occured
            {
                SET_PORT_ERROR(r);
                d->state = STATE_TIMEOUT;
                RAISE_COMPLETED(r);
            }
            if (!d->port->isOpen())
            {
                d->state = STATE_CLOSED;
                signalClosed(this->objectName());
                RAISE_COMPLETED(Status_Uncertain);
            }
            signalRx(d->getName(), d->port->readBufferData(), d->port->readBufferSize());
            // verify unit id
            r = d->port->readBuffer(d->unit, d->func, buff, szBuff, &outBytes);
            if (StatusIsBad(r))
            {
                d->setPortError(r);
            }
            else if (!d->isUnitEnabled(d->unit))
            {
                d->state = STATE_BEGIN_READ;
                return Status_Good;
            }
            if (StatusIsGood(r))
                r = processInputData(buff, outBytes);
            if (StatusIsBad(r)) // data error
            {
                if (StatusIsStandardError(r)) // return standard error to device
                {
                    d->state = STATE_BEGIN_WRITE;
                    fRepeatAgain = true;
                    break;
                }
                else
                {
                    d->state = STATE_BEGIN_READ;
                    RAISE_ERROR_COMPLETED(r, d->lastErrorTextData())
                }
            }
            d->state = STATE_PROCESS_DEVICE;
            // no need break
        case STATE_PROCESS_DEVICE:
            r = processDevice();
            if (StatusIsProcessing(r))
                return r;
            if ((r == Status_BadGatewayPathUnavailable) || d->isBroadcast())
            {
                d->state = STATE_BEGIN_READ;
                RAISE_COMPLETED(Modbus::Status_Good);
            }
            d->state = STATE_BEGIN_WRITE;
            // no need break
        case STATE_BEGIN_WRITE:
            d->timestampRefresh();
            func = d->func;
            if (StatusIsBad(r))
            {
                signalError(d->getName(), r, d->lastErrorTextData());
                func |= MBF_EXCEPTION;
                if (StatusIsStandardError(r))
                    buff[0] = static_cast<uint8_t>(r & 0xFF);
                else
                    buff[0] = static_cast<uint8_t>(Status_BadServerDeviceFailure & 0xFF);
                outCount = 1;
            }
            else
                processOutputData(buff, outCount);
            d->port->writeBuffer(d->unit, func, buff, outCount);
            d->state = STATE_WRITE;
            // no need break
        case STATE_WRITE:
        {
            // Note: r - status can be Bad so do not rewrite it,
            // use separate `ws` variable for write status
            Modbus::StatusCode ws = d->port->write();
            if (StatusIsProcessing(ws))
                return ws;
            if (StatusIsBad(ws))
            {
                SET_PORT_ERROR(ws);
                d->state = STATE_TIMEOUT;
                RAISE_COMPLETED(ws)
            }
            else
            {
                signalTx(d->getName(), d->port->writeBufferData(), d->port->writeBufferSize());
                d->state = STATE_BEGIN_READ;
                RAISE_COMPLETED(r)
            }
        }
        case STATE_TIMEOUT:
            if (timer() - d->timestamp < d->port->timeout())
                return Status_Processing;
            d->state = STATE_UNKNOWN;
            fRepeatAgain = true;
            break;
        default:
            if (d->port->isOpen())
            {
                if (d->cmdClose)
                    d->state = STATE_WAIT_FOR_CLOSE;
                else
                    d->state = STATE_OPENED;
            }
            else
                d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

const Modbus::Char *ModbusServerResource::lastErrorText() const
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    if (d->isLastPortError)
        return d->port->lastErrorText();
    else
        return d->lastErrorText.data();
}

StatusCode ModbusServerResource::processInputData(const uint8_t *buff, uint16_t sz)
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    switch (d->func)
    {

#ifndef MBF_READ_COILS_DISABLE
    case MBF_READ_COILS:
#endif // MBF_READ_COILS_DISABLE
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    case MBF_READ_DISCRETE_INPUTS:
#endif // MBF_READ_DISCRETE_INPUTS
#if !defined(MBF_READ_COILS_DISABLE) || !defined(MBF_READ_DISCRETE_INPUTS_DISABLE)
        if (sz != 4) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset = buff[1] | (buff[0] << 8);
        d->count  = buff[3] | (buff[2] << 8);
        if (d->count > MB_MAX_DISCRETS) // prevent valueBuff overflow
            return d->setError(Status_BadIllegalDataValue, StringLiteral("Incorrect data value"));
        break;
#endif // !defined(MBF_READ_COILS_DISABLE) || !defined(MBF_READ_DISCRETE_INPUTS_DISABLE)

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    case MBF_READ_HOLDING_REGISTERS:
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    case MBF_READ_INPUT_REGISTERS:
#endif // MBF_READ_INPUT_REGISTERS_DISABLE
#if !defined(MBF_READ_HOLDING_REGISTERS_DISABLE) || !defined(MBF_READ_INPUT_REGISTERS_DISABLE)
        if (sz != 4) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset = buff[1] | (buff[0]<<8);
        d->count = buff[3] | (buff[2]<<8);
        if (d->count > MB_MAX_REGISTERS) // prevent valueBuff overflow
            return d->setError(Status_BadIllegalDataValue, StringLiteral("Incorrect data value"));
        break;
#endif // !defined(MBF_READ_HOLDING_REGISTERS_DISABLE) || !defined(MBF_READ_INPUT_REGISTERS_DISABLE)

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    case MBF_WRITE_SINGLE_COIL:
        if (sz != 4) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if (!(buff[2] == 0x00 || buff[2] == 0xFF) || (buff[3] != 0))  // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect data value"));
        d->offset = buff[1] | (buff[0]<<8);
        d->valueBuff[0] = buff[2];
        break;
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    case MBF_WRITE_SINGLE_REGISTER:
        if (sz != 4) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset = buff[1] | (buff[0]<<8);
        d->valueBuff[0] = buff[3];
        d->valueBuff[1] = buff[2];
        break;
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    case MBF_READ_EXCEPTION_STATUS:
        if (sz > 0) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        break;
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    case MBF_DIAGNOSTICS:
        if (sz < 2) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->subfunc = buff[1] | (buff[0]<<8);
        d->count = sz - 2;
        memcpy(d->valueBuff, &buff[2], d->count);
        break;
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    case MBF_GET_COMM_EVENT_COUNTER:
        if (sz > 0) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        break;
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    case MBF_GET_COMM_EVENT_LOG:
        if (sz > 0) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        break;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    case MBF_WRITE_MULTIPLE_COILS: // Write multiple coils
        if (sz < 5) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if (sz != buff[4]+5) // don't match readed bytes and number of data bytes to follow
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset = buff[1] | (buff[0]<<8);
        d->count = buff[3] | (buff[2]<<8);
        if ((d->count+7)/8 != buff[4]) // don't match count bites and bytes
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if (d->count > MB_MAX_DISCRETS) // prevent valueBuff overflow
            return d->setError(Status_BadIllegalDataValue, StringLiteral("Incorrect data value"));
        memcpy(d->valueBuff, &buff[5], (d->count+7)/8);
        break;
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_WRITE_MULTIPLE_REGISTERS:
        if (sz < 5) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if (sz != buff[4]+5) // don't match readed bytes and number of data bytes to follow
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset = buff[1] | (buff[0]<<8);
        d->count = buff[3] | (buff[2]<<8);
        if (d->count*2 != buff[4]) // don't match count values and bytes
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if (d->count > MB_MAX_REGISTERS) // prevent valueBuff overflow
            return d->setError(Status_BadIllegalDataValue, StringLiteral("Incorrect data value"));
        for (uint16_t i = 0; i < d->count; i++)
        {
            d->valueBuff[i*2]   = buff[6+i*2];
            d->valueBuff[i*2+1] = buff[5+i*2];
        }
        break;
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    case MBF_REPORT_SERVER_ID:
        if (sz > 0) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        break;
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    case MBF_MASK_WRITE_REGISTER:
        if (sz != 6) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset  = buff[1] | (buff[0]<<8);
        d->andMask = buff[3] | (buff[2]<<8);
        d->orMask  = buff[5] | (buff[4]<<8);
        break;
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_READ_WRITE_MULTIPLE_REGISTERS:
        if (sz < 9) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if (sz != buff[8]+9) // don't match readed bytes and number of data bytes to follow
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset      = buff[1] | (buff[0]<<8);
        d->count       = buff[3] | (buff[2]<<8);
        d->writeOffset = buff[5] | (buff[4]<<8);
        d->writeCount  = buff[7] | (buff[6]<<8);
        if (d->writeCount*2 != buff[8]) // don't match count values and bytes
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        if ((d->count > MB_MAX_REGISTERS) || (d->writeCount > MB_MAX_REGISTERS)) // prevent valueBuff overflow
            return d->setError(Status_BadIllegalDataValue, StringLiteral("Incorrect data value"));
        for (uint16_t i = 0; i < d->count; i++)
        {
            d->valueBuff[i*2]   = buff[10+i*2];
            d->valueBuff[i*2+1] = buff[ 9+i*2];
        }
        break;
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    case MBF_READ_FIFO_QUEUE:
        if (sz < 2) // Incorrect request from client - don't respond
            return d->setError(Status_BadNotCorrectRequest, StringLiteral("Incorrect received data size"));
        d->offset  = buff[1] | (buff[0]<<8);
        break;
#endif // MBF_READ_FIFO_QUEUE_DISABLE

    default:
        return d->setError(Status_BadIllegalFunction, StringLiteral("Unsupported function"));
    }
    return Status_Good;
}

StatusCode ModbusServerResource::processDevice()
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    StatusCode r;
    switch (d->func)
    {

#ifndef MBF_READ_COILS_DISABLE
    case MBF_READ_COILS:
        r = d->device->readCoils(d->unit, d->offset, d->count, d->valueBuff);
        break;
#endif // MBF_READ_COILS_DISABLE

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    case MBF_READ_DISCRETE_INPUTS:
        r = d->device->readDiscreteInputs(d->unit, d->offset, d->count, d->valueBuff);
        break;
#endif // MBF_READ_DISCRETE_INPUTS_DISABLE

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    case MBF_READ_HOLDING_REGISTERS:
        r = d->device->readHoldingRegisters(d->unit, d->offset, d->count, reinterpret_cast<uint16_t*>(d->valueBuff));
        break;
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    case MBF_READ_INPUT_REGISTERS:
        r = d->device->readInputRegisters(d->unit, d->offset, d->count, reinterpret_cast<uint16_t*>(d->valueBuff));
        break;
#endif // MBF_READ_INPUT_REGISTERS_DISABLE

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    case MBF_WRITE_SINGLE_COIL:
        r = d->device->writeSingleCoil(d->unit, d->offset, d->valueBuff[0]);
        break;
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    case MBF_WRITE_SINGLE_REGISTER:
        r = d->device->writeSingleRegister(d->unit, d->offset, reinterpret_cast<uint16_t*>(d->valueBuff)[0]);
        break;
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    case MBF_READ_EXCEPTION_STATUS:
        r = d->device->readExceptionStatus(d->unit, d->valueBuff);
        break;
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    case MBF_DIAGNOSTICS:
        r = d->device->diagnostics(d->unit, d->subfunc, d->byteCount, d->valueBuff, &d->outByteCount, d->valueBuff);
        break;
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    case MBF_GET_COMM_EVENT_COUNTER:
        r = d->device->getCommEventCounter(d->unit, &d->status, &d->count);
        break;
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    case MBF_GET_COMM_EVENT_LOG:
        r = d->device->getCommEventLog(d->unit, &d->status, &d->count, &d->messageCount, &d->outByteCount, d->valueBuff);
        break;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    case MBF_WRITE_MULTIPLE_COILS:
        r = d->device->writeMultipleCoils(d->unit, d->offset, d->count, d->valueBuff);
        break;
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_WRITE_MULTIPLE_REGISTERS:
        r = d->device->writeMultipleRegisters(d->unit, d->offset, d->count, reinterpret_cast<uint16_t*>(d->valueBuff));
        break;
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    case MBF_REPORT_SERVER_ID:
        r = d->device->reportServerID(d->unit, &d->outByteCount, d->valueBuff);
        break;
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    case MBF_MASK_WRITE_REGISTER:
        r = d->device->maskWriteRegister(d->unit, d->offset, d->andMask, d->orMask);
        break;
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_READ_WRITE_MULTIPLE_REGISTERS: 
        r = d->device->readWriteMultipleRegisters(d->unit, d->offset, d->count, reinterpret_cast<uint16_t*>(d->valueBuff), d->writeOffset, d->writeCount, reinterpret_cast<uint16_t*>(d->valueBuff));
        break;
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    case MBF_READ_FIFO_QUEUE:
        r = d->device->readFIFOQueue(d->unit, d->offset, &d->count, reinterpret_cast<uint16_t*>(d->valueBuff));
        break;
#endif // MBF_READ_FIFO_QUEUE_DISABLE

    default:
        return d->setError(Status_BadIllegalFunction, StringLiteral("Unsupported function"));
    }
    if (StatusIsBad(r))
    {
        if (StatusIsStandardError(r))
            d->setError(r, StringLiteral("Device returned a standard exception with code 0x") + toHexString<String, uint8_t>(r & 0xFF));
        else
            d->setError(r, StringLiteral("Device returned an error with code 0x") + toHexString<String, uint32_t>(r & 0xFF));
    }
    return r;
}

StatusCode ModbusServerResource::processOutputData(uint8_t *buff, uint16_t &sz)
{
    ModbusServerResourcePrivate *d = d_cast(d_ptr);
    switch (d->func)
    {
#ifndef MBF_READ_COILS_DISABLE
    case MBF_READ_COILS:
#endif // MBF_READ_COILS_DISABLE
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    case MBF_READ_DISCRETE_INPUTS:
#endif // MBF_READ_DISCRETE_INPUTS
#if !defined(MBF_READ_COILS_DISABLE) || !defined(MBF_READ_DISCRETE_INPUTS_DISABLE)
        buff[0] = static_cast<uint8_t>((d->count+7)/8);
        memcpy(&buff[1], d->valueBuff, buff[0]);
        sz = buff[0] + 1;
        break;
#endif // !defined(MBF_READ_COILS_DISABLE) || !defined(MBF_READ_DISCRETE_INPUTS_DISABLE)

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    case MBF_READ_HOLDING_REGISTERS:
#endif // MBF_READ_HOLDING_REGISTERS_DISABLE
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    case MBF_READ_INPUT_REGISTERS:
#endif // MBF_READ_INPUT_REGISTERS_DISABLE
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_READ_WRITE_MULTIPLE_REGISTERS:
#endif // MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
#if !defined(MBF_READ_HOLDING_REGISTERS_DISABLE) || !defined(MBF_READ_INPUT_REGISTERS_DISABLE) || !defined(MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE)
        buff[0] = static_cast<uint8_t>(d->count * 2);
        for (uint16_t i = 0; i < d->count; i++)
        {
            buff[2+i*2] = d->valueBuff[i*2];
            buff[1+i*2] = d->valueBuff[i*2+1];
        }
        sz = buff[0] + 1;
        break;
#endif // !defined(MBF_READ_HOLDING_REGISTERS_DISABLE) || !defined(MBF_READ_INPUT_REGISTERS_DISABLE) || !defined(MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE)

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    case MBF_WRITE_SINGLE_COIL:
        buff[0] = static_cast<uint8_t>(d->offset >> 8);      // address of coil (Hi-byte)
        buff[1] = static_cast<uint8_t>(d->offset & 0xFF);    // address of coil (Lo-byte)
        buff[2] = d->valueBuff[0] ? 0xFF : 0x00;             // value (Hi-byte)
        buff[3] = 0;                                         // value (Lo-byte)
        sz = 4;
        break;
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    case MBF_WRITE_SINGLE_REGISTER:
        buff[0] = static_cast<uint8_t>(d->offset >> 8);      // address of register (Hi-byte)
        buff[1] = static_cast<uint8_t>(d->offset & 0xFF);    // address of register (Lo-byte)
        buff[2] = d->valueBuff[1];                           // value (Hi-byte)
        buff[3] = d->valueBuff[0];                           // value (Lo-byte)
        sz = 4;
        break;
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    case MBF_READ_EXCEPTION_STATUS:
        buff[0] = d->valueBuff[0];
        sz = 1;
        break;
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    case MBF_DIAGNOSTICS:
        buff[0] = static_cast<uint8_t>(d->subfunc >> 8);      // address of register (Hi-byte)
        buff[1] = static_cast<uint8_t>(d->subfunc & 0xFF);    // address of register (Lo-byte)
        memcpy(&buff[2], d->valueBuff, d->outByteCount);
        sz = d->outByteCount+2;
        break;
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    case MBF_GET_COMM_EVENT_COUNTER:
        buff[0] = static_cast<uint8_t>(d->status >> 8);      // status of counter (Hi-byte)
        buff[1] = static_cast<uint8_t>(d->status & 0xFF);    // status of counter (Lo-byte)
        buff[2] = static_cast<uint8_t>(d->count >> 8);       // event counter value (Hi-byte)
        buff[3] = static_cast<uint8_t>(d->count & 0xFF);     // event counter value (Lo-byte)
        sz = 4;
        break;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    case MBF_GET_COMM_EVENT_LOG:
        buff[0] = d->outByteCount+6;                           // output bytes count
        buff[1] = static_cast<uint8_t>(d->status >> 8);        // status of counter (Hi-byte)
        buff[2] = static_cast<uint8_t>(d->status & 0xFF);      // status of counter (Lo-byte)
        buff[3] = static_cast<uint8_t>(d->count >> 8);         // event counter value (Hi-byte)
        buff[4] = static_cast<uint8_t>(d->count & 0xFF);       // event counter value (Lo-byte)
        buff[5] = static_cast<uint8_t>(d->messageCount >> 8);  // message counter value (Hi-byte)
        buff[6] = static_cast<uint8_t>(d->messageCount & 0xFF);// message counter value (Lo-byte)
        memcpy(&buff[7], d->valueBuff, d->outByteCount);
        sz = d->outByteCount+7;
        break;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    case MBF_WRITE_MULTIPLE_COILS:
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_WRITE_MULTIPLE_REGISTERS: 
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
#if !defined(MBF_WRITE_MULTIPLE_COILS_DISABLE) || !defined(MBF_WRITE_MULTIPLE_REGISTERS_DISABLE)
        buff[0] = static_cast<uint8_t>(d->offset >> 8);      // offset of written values (Hi-byte)
        buff[1] = static_cast<uint8_t>(d->offset & 0xFF);    // offset of written values (Lo-byte)
        buff[2] = static_cast<uint8_t>(d->count >> 8);       // count of written values (Hi-byte)
        buff[3] = static_cast<uint8_t>(d->count & 0xFF);     // count of written values (Lo-byte)
        sz = 4;
        break;
#endif // !defined(MBF_WRITE_MULTIPLE_COILS_DISABLE) || !defined(MBF_WRITE_MULTIPLE_REGISTERS_DISABLE)

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    case MBF_REPORT_SERVER_ID:
        buff[0] = d->outByteCount;                           // output bytes count
        memcpy(&buff[1], d->valueBuff, d->outByteCount);
        sz = d->outByteCount+1;
        break;
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    case MBF_MASK_WRITE_REGISTER:
        buff[0] = static_cast<uint8_t>(d->offset >> 8);      // address of register (Hi-byte)
        buff[1] = static_cast<uint8_t>(d->offset & 0xFF);    // address of register (Lo-byte)
        buff[2] = static_cast<uint8_t>(d->andMask >> 8);     // And mask (Hi-byte)
        buff[3] = static_cast<uint8_t>(d->andMask & 0xFF);   // And mask (Lo-byte)
        buff[4] = static_cast<uint8_t>(d->orMask >> 8);      // Or mask (Hi-byte)
        buff[5] = static_cast<uint8_t>(d->orMask & 0xFF);    // Or mask (Lo-byte)
        sz = 6;
        break;
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    case MBF_READ_FIFO_QUEUE:
    {
        uint16_t byteCount = (d->count * 2) + 2;
        buff[0] = static_cast<uint8_t>(byteCount >> 8);      // status of counter (Hi-byte)
        buff[1] = static_cast<uint8_t>(byteCount & 0xFF);    // status of counter (Lo-byte)
        buff[2] = static_cast<uint8_t>(d->count >> 8);       // event counter value (Hi-byte)
        buff[3] = static_cast<uint8_t>(d->count & 0xFF);     // event counter value (Lo-byte)
        for (uint16_t i = 0; i < d->count; i++)
        {
            buff[5+i*2] = d->valueBuff[i*2];
            buff[4+i*2] = d->valueBuff[i*2+1];
        }
        sz = (d->count * 2) + 4;
    }
        break;
#endif // MBF_READ_FIFO_QUEUE_DISABLE

    }
    return Status_Good;
}
