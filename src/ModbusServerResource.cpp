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
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset = buff[1] | (buff[0] << 8);
        d->count  = buff[3] | (buff[2] << 8);
        if (d->count > MB_MAX_DISCRETS) // prevent valueBuff overflow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Requested count of bits %hu is too large (max=%hu)"), d->func, d->count, (uint16_t)MB_MAX_DISCRETS);
            return d->setError(Status_BadIllegalDataValue, errbuff);
        }
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
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset = buff[1] | (buff[0]<<8);
        d->count = buff[3] | (buff[2]<<8);
        if (d->count > MB_MAX_REGISTERS) // prevent valueBuff overflow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Requested count of registers %hu is too large (max=%hu)"), d->func, d->count, (uint16_t)MB_MAX_REGISTERS);
            return d->setError(Status_BadIllegalDataValue, errbuff);
        }
        break;
#endif // !defined(MBF_READ_HOLDING_REGISTERS_DISABLE) || !defined(MBF_READ_INPUT_REGISTERS_DISABLE)

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    case MBF_WRITE_SINGLE_COIL:
        if (sz != 4) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if (!(buff[2] == 0x00 || buff[2] == 0xFF) || (buff[3] != 0))  // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect data value"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset = buff[1] | (buff[0]<<8);
        d->valueBuff[0] = buff[2];
        break;
#endif // MBF_WRITE_SINGLE_COIL_DISABLE

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    case MBF_WRITE_SINGLE_REGISTER:
        if (sz != 4) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset = buff[1] | (buff[0]<<8);
        d->valueBuff[0] = buff[3];
        d->valueBuff[1] = buff[2];
        break;
#endif // MBF_WRITE_SINGLE_REGISTER_DISABLE

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    case MBF_READ_EXCEPTION_STATUS:
        if (sz > 0) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        break;
#endif // MBF_READ_EXCEPTION_STATUS_DISABLE

#ifndef MBF_DIAGNOSTICS_DISABLE
    case MBF_DIAGNOSTICS:
        if (sz < 2) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->subfunc = buff[1] | (buff[0]<<8);
        d->count = sz - 2;
        switch (d->subfunc)
        {
        case MBF_DIAGNOSTICS_RETURN_QUERY_DATA:
            break;
        default:
            if (d->count != 2) // Incorrect request from client - don't respond
            {
                const size_t len = 100;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC%02hhu. Subfunction %02hu. Data size must be 2"), d->func, d->subfunc);
                return d->setError(Status_BadNotCorrectRequest, errbuff);
            }
            break;
        }
        memcpy(d->valueBuff, &buff[2], d->count);
        break;
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    case MBF_GET_COMM_EVENT_COUNTER:
        if (sz > 0) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        break;
#endif // MBF_GET_COMM_EVENT_COUNTER_DISABLE

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    case MBF_GET_COMM_EVENT_LOG:
        if (sz > 0) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        break;
#endif // MBF_GET_COMM_EVENT_LOG_DISABLE

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    case MBF_WRITE_MULTIPLE_COILS: // Write multiple coils
        if (sz < 5) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if (sz != buff[4]+5) // don't match readed bytes and number of data bytes to follow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset = buff[1] | (buff[0]<<8);
        d->count = buff[3] | (buff[2]<<8);
        if ((d->count+7)/8 != buff[4]) // don't match count bites and bytes
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if (d->count > MB_MAX_DISCRETS) // prevent valueBuff overflow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect data value"), d->func);
            return d->setError(Status_BadIllegalDataValue, errbuff);
        }
        memcpy(d->valueBuff, &buff[5], (d->count+7)/8);
        break;
#endif // MBF_WRITE_MULTIPLE_COILS_DISABLE

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_WRITE_MULTIPLE_REGISTERS:
        if (sz < 5) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if (sz != buff[4]+5) // don't match readed bytes and number of data bytes to follow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset = buff[1] | (buff[0]<<8);
        d->count = buff[3] | (buff[2]<<8);
        if (d->count*2 != buff[4]) // don't match count values and bytes
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if (d->count > MB_MAX_REGISTERS) // prevent valueBuff overflow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect data value"), d->func);
            return d->setError(Status_BadIllegalDataValue, errbuff);
        }
        for (uint16_t i = 0; i < d->count; i++)
        {
            d->valueBuff[i*2  ] = buff[6+i*2];
            d->valueBuff[i*2+1] = buff[5+i*2];
        }
        break;
#endif // MBF_WRITE_MULTIPLE_REGISTERS_DISABLE

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    case MBF_REPORT_SERVER_ID:
        if (sz > 0) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        break;
#endif // MBF_REPORT_SERVER_ID_DISABLE

#ifndef MBF_READ_FILE_RECORD_DISABLE
    case MBF_READ_FILE_RECORD:
    {
        if (sz < 8) // Incorrect request from client - don't respond, minimum 1 record (7 bytes) + 1 byte count
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->recordsCount = buff[0] / 7; // each record is 7 bytes
        if (sz != 1 + d->recordsCount*7) // don't match readed bytes and number of records to follow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        auto records = d->fileRecordBuff();
        for (uint8_t i = 0; i < d->recordsCount; i++)
        {
            const uint8_t *recBuff = &buff[1+i*7];
            auto referenceType = recBuff[0];
            if (referenceType != 0x06) // Incorrect request from client - don't respond
            {
                const size_t len = 100;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect reference type %hhu in record %hhu"), d->func, referenceType, i);
                return d->setError(Status_BadNotCorrectRequest, errbuff);
            }
            records[i].fileNumber    = recBuff[2] | (recBuff[1]<<8);
            records[i].recordNumber  = recBuff[4] | (recBuff[3]<<8);
            records[i].recordLength  = recBuff[6] | (recBuff[5]<<8);
        }
        break;
    }
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    case MBF_WRITE_FILE_RECORD:
    {
        if (sz < 8 || (buff[0] != sz-1)) // Incorrect request from client - don't respond, minimum 1 record (7 bytes) + 1 byte count
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        const uint8_t szDataBuff = 255;
        uint8_t dataBuff[szDataBuff];
        d->recordsCount = 0;
        auto records = d->fileRecordBuff();
        uint8_t c = 0;
        for (uint8_t i = 1; i < sz;)
        {
            const uint8_t *recBuff = &buff[i];
            auto referenceType = recBuff[0];
            if (referenceType != 0x06) // Incorrect request from client - don't respond
            {
                const size_t len = 100;
                Char errbuff[len];
                snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect reference type %hhu in record %hhu"), d->func, referenceType, d->recordsCount);
                return d->setError(Status_BadNotCorrectRequest, errbuff);
            }
            records[d->recordsCount].fileNumber    = recBuff[2] | (recBuff[1]<<8);
            records[d->recordsCount].recordNumber  = recBuff[4] | (recBuff[3]<<8);
            records[d->recordsCount].recordLength  = recBuff[6] | (recBuff[5]<<8);
            auto len = records[d->recordsCount].recordLength;
            for (uint16_t j = 0; j < len; ++j)
            {
                dataBuff[j*2  ] = recBuff[8+j*2];
                dataBuff[j*2+1] = recBuff[7+j*2];
            }
            d->recordsCount++;
            c += len*2;
            i += 7 + c;
        }
        memcpy(d->fileDataBuff(), dataBuff, c);
        d->fileDataBuffSize = c;
    }
        break;
#endif // MBF_WRITE_FILE_RECORD_DISABLE

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    case MBF_MASK_WRITE_REGISTER:
        if (sz != 6) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset  = buff[1] | (buff[0]<<8);
        d->andMask = buff[3] | (buff[2]<<8);
        d->orMask  = buff[5] | (buff[4]<<8);
        break;
#endif // MBF_MASK_WRITE_REGISTER_DISABLE

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    case MBF_READ_WRITE_MULTIPLE_REGISTERS:
        if (sz < 9) // Incorrect request from client - don't respond
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if (sz != buff[8]+9) // don't match readed bytes and number of data bytes to follow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset      = buff[1] | (buff[0]<<8);
        d->count       = buff[3] | (buff[2]<<8);
        d->writeOffset = buff[5] | (buff[4]<<8);
        d->writeCount  = buff[7] | (buff[6]<<8);
        if (d->writeCount*2 != buff[8]) // don't match count values and bytes
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        if ((d->count > MB_MAX_REGISTERS) || (d->writeCount > MB_MAX_REGISTERS)) // prevent valueBuff overflow
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect data value"), d->func);
            return d->setError(Status_BadIllegalDataValue, errbuff);
        }
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
        {
            const size_t len = 100;
            Char errbuff[len];
            snprintf(errbuff, len, StringLiteral("FC%02hhu. Incorrect received data size"), d->func);
            return d->setError(Status_BadNotCorrectRequest, errbuff);
        }
        d->offset  = buff[1] | (buff[0]<<8);
        break;
#endif // MBF_READ_FIFO_QUEUE_DISABLE

    default:
    {
        const size_t len = 100;
        Char errbuff[len];
        snprintf(errbuff, len, StringLiteral("FC%02hhu. Unsupported function"), d->func);
        return d->setError(Status_BadIllegalFunction, errbuff);
    }
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
        switch (d->subfunc)
        {
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
        case MBF_DIAGNOSTICS_RETURN_QUERY_DATA:
            r = d->device->diagnosticsReturnQueryData(d->unit, d->byteCount, d->valueBuff, &d->outByteCount, d->valueBuff);
            break;
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
        case MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION:
            r = d->device->diagnosticsRestartCommunicationsOption(d->unit, d->valueBuff[0]);
            break;
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
        case MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER:
            r = d->device->diagnosticsReturnDiagnosticRegister(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
        case MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER:
            r = d->device->diagnosticsChangeAsciiInputDelimiter(d->unit, static_cast<char>(d->valueBuff[0]));
            break;
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
        case MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE:
            r = d->device->diagnosticsForceListenOnlyMode(d->unit);
            break;
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
        case MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER:
            r = d->device->diagnosticsClearCountersAndDiagnosticRegister(d->unit);
            break;
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT:
            r = d->device->diagnosticsReturnBusMessageCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT :
            r = d->device->diagnosticsReturnBusCommunicationErrorCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT:
            r = d->device->diagnosticsReturnBusExceptionErrorCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT:
            r = d->device->diagnosticsReturnServerMessageCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT:
            r = d->device->diagnosticsReturnServerNoResponseCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT:
            r = d->device->diagnosticsReturnServerNAKCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT:
            r = d->device->diagnosticsReturnServerBusyCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_CHARACTER_OVERRUN_COUNT_DISABLE
        case MBF_DIAGNOSTICS_RETURN_BUS_CHARACTER_OVERRUN_COUNT:
            r = d->device->diagnosticsReturnBusCharacterOverrunCount(d->unit, reinterpret_cast<uint16_t*>(d->valueBuff));
            break;
#endif // MBF_DIAGNOSTICS_RETURN_BUS_CHARACTER_OVERRUN_COUNT_DISABLE

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
        case MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG:
            r = d->device->diagnosticsClearOverrunCounterAndFlag(d->unit);
            break;
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
        default:
            return d->setError(Status_BadIllegalFunction, StringLiteral("Unsupported function"));
        }
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

#ifndef MBF_READ_FILE_RECORD_DISABLE
    case MBF_READ_FILE_RECORD:
        r = d->device->readFileRecord(d->unit, d->recordsCount, d->fileRecordBuff(), &d->fileDataBuffSize, d->fileDataBuff());
        break;
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    case MBF_WRITE_FILE_RECORD:
        r = d->device->writeFileRecord(d->unit, d->recordsCount, d->fileRecordBuff(), d->fileDataBuffSize, d->fileDataBuff());
        break;
#endif // MBF_WRITE_FILE_RECORD_DISABLE

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
        switch (d->subfunc)
        {
        case MBF_DIAGNOSTICS_RETURN_QUERY_DATA:
            memcpy(&buff[2], d->valueBuff, d->outByteCount);
            sz = d->outByteCount+2;
            break;
        case MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION:
        case MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER:
        case MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE:
        case MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER:
        case MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG:
            buff[0] = d->valueBuff[0]; 
            buff[1] = d->valueBuff[1]; 
            sz = 4;
            break;
        default:
            buff[0] = d->valueBuff[1]; 
            buff[1] = d->valueBuff[0]; 
            sz = 4;
            break;
        }
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

#ifndef MBF_READ_FILE_RECORD_DISABLE
    case MBF_READ_FILE_RECORD:
    {
        uint8_t buffSz = 1;
        uint8_t dataPtr = 0;
        for (uint8_t i = 0; i < d->recordsCount; i++)
        {
            const auto &rec = d->fileRecordBuff()[i];
            uint8_t *recBuff = &buff[buffSz];
            recBuff[0] = rec.recordLength*2+1; // reference type
            recBuff[1] = 0x06; // reference type
            for (uint16_t j = 0; j < rec.recordLength; ++j)
            {
                recBuff[2+j*2] = d->fileDataBuff()[dataPtr+j*2+1];
                recBuff[3+j*2] = d->fileDataBuff()[dataPtr+j*2  ];
            }
            dataPtr += rec.recordLength * 2;
            buffSz += rec.recordLength * 2 + 2;
        }
        buff[0] = buffSz+1; // byte count
        sz = buffSz + 1;
    }
        break;
#endif // MBF_READ_FILE_RECORD_DISABLE

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    case MBF_WRITE_FILE_RECORD:
    {
        uint8_t buffSz = 1;
        uint8_t dataPtr = 0;
        for (uint8_t i = 0; i < d->recordsCount; i++)
        {
            const auto &rec = d->fileRecordBuff()[i];
            uint8_t *recBuff = &buff[buffSz];
            recBuff[0] = 0x06; // reference type
            recBuff[1] = reinterpret_cast<const uint8_t*>(&rec.fileNumber)[1];
            recBuff[2] = reinterpret_cast<const uint8_t*>(&rec.fileNumber)[0]; 
            recBuff[3] = reinterpret_cast<const uint8_t*>(&rec.recordNumber)[1]; 
            recBuff[4] = reinterpret_cast<const uint8_t*>(&rec.recordNumber)[0]; 
            recBuff[5] = reinterpret_cast<const uint8_t*>(&rec.recordLength)[1]; 
            recBuff[6] = reinterpret_cast<const uint8_t*>(&rec.recordLength)[0]; 
            for (uint16_t j = 0; j < rec.recordLength; ++j)
            {
                recBuff[7+j*2] = d->fileDataBuff()[dataPtr+j*2+1];
                recBuff[8+j*2] = d->fileDataBuff()[dataPtr+j*2  ];
            }
            dataPtr += rec.recordLength * 2;
            buffSz += rec.recordLength * 2 + 7;
        }
        buff[0] = buffSz+1; // byte count
        sz = buffSz + 1;
    }
        break;
#endif // MBF_WRITE_FILE_RECORD_DISABLE

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
