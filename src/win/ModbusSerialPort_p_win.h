#ifndef MODBUSSERIALPORT_P_WIN_H
#define MODBUSSERIALPORT_P_WIN_H

#include <Windows.h>

#include "../ModbusSerialPort_p.h"

class ModbusSerialPortPrivateWin : public ModbusSerialPortPrivate
{
public:
    ModbusSerialPortPrivateWin(bool blocking) :
        ModbusSerialPortPrivate(blocking)
    {
        this->serialPort = INVALID_HANDLE_VALUE;
        this->timestamp = 0;
        if (blocking)
        {
            writeMethod = &ModbusSerialPortPrivateWin::blockingWrite;
            readMethod  = &ModbusSerialPortPrivateWin::blockingRead ;
        }
        else
        {
            writeMethod = &ModbusSerialPortPrivateWin::nonBlockingWrite;
            readMethod  = &ModbusSerialPortPrivateWin::nonBlockingRead ;
        }
        oWrite.hEvent = NULL;
        oRead.hEvent = NULL;
    }

    ~ModbusSerialPortPrivateWin()
    {
        if (serialPort != INVALID_HANDLE_VALUE)
            CloseHandle(serialPort);
        if (oWrite.hEvent != NULL)
            CloseHandle(oWrite.hEvent);
        if (oRead.hEvent != NULL)
            CloseHandle(oRead.hEvent);
    }

public:
    inline bool serialPortIsInvalid() const { return serialPort == INVALID_HANDLE_VALUE; }
    inline bool serialPortIsValid() const { return serialPort != INVALID_HANDLE_VALUE; }
    inline bool serialPortIsOpen() const { return serialPortIsValid(); }
    inline void serialPortClose() { CloseHandle(serialPort); serialPort = INVALID_HANDLE_VALUE; }
    inline void timestampRefresh() { timestamp = GetTickCount(); }

public:
    StatusCode blockingWrite();
    StatusCode blockingRead ();
    StatusCode nonBlockingWrite();
    StatusCode nonBlockingRead ();

public:
    inline void zeroOverlapped(OVERLAPPED &o)
    {
        if (o.hEvent != NULL)
            CloseHandle(o.hEvent);
        ::ZeroMemory(&o, sizeof(OVERLAPPED));
    }

    inline void closeEventHandle(OVERLAPPED &o)
    {
        CloseHandle(o.hEvent);
        o.hEvent = NULL;
    }

public:
    HANDLE serialPort;
    DWORD timestamp;
    OVERLAPPED oWrite;
    OVERLAPPED oRead;

    typedef StatusCode (ModbusSerialPortPrivateWin::*RWMethodPtr_t)();
    RWMethodPtr_t writeMethod;
    RWMethodPtr_t readMethod ;
};

inline ModbusSerialPortPrivateWin *d_win(ModbusPortPrivate *d_ptr) { return static_cast<ModbusSerialPortPrivateWin*>(d_ptr); }

StatusCode ModbusSerialPortPrivateWin::blockingWrite()
{
    BOOL r;
    this->state = STATE_BEGIN;
    PurgeComm(this->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);
    r =  WriteFile(this->serialPort, this->buff, this->sz, NULL, NULL);
    if (!r)
    {
        DWORD err = GetLastError();
        return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->settings.portName +
                                                     StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                     StringLiteral(". ") + getLastErrorText());
    }
    return Status_Good;
}

StatusCode ModbusSerialPortPrivateWin::blockingRead()
{
    BOOL r;
    DWORD c;
    this->state = STATE_BEGIN;
    r = ReadFile(this->serialPort, this->buff, this->c_buffSz, &c, NULL);
    if (!r)
    {
        DWORD err = GetLastError();
        return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->settings.portName +
                                                    StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                    StringLiteral(". ") + getLastErrorText());
    }
    this->sz = static_cast<uint16_t>(c);
    return Status_Good;
}

StatusCode ModbusSerialPortPrivateWin::nonBlockingWrite()
{
    BOOL r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (this->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_WRITE:
            // Note: clean read buffer from garbage before write
            PurgeComm(this->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);
            zeroOverlapped(this->oWrite);
            this->oWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (this->oWrite.hEvent == NULL)
            {
                this->state = STATE_BEGIN;
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->settings.portName +
                                                             StringLiteral("' serial port (CreateEvent). Error code: ") + toModbusString(err) +
                                                             StringLiteral(". ") + getLastErrorText());
            }
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_WRITE;
            // no need break
        case STATE_WAIT_FOR_WRITE:
        case STATE_WAIT_FOR_WRITE_ALL:
            r =  WriteFile(this->serialPort, this->buff, this->sz, NULL, &this->oWrite);
            if (!r)
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    this->state = STATE_BEGIN;
                    closeEventHandle(this->oWrite);
                    return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->settings.portName +
                                                                 StringLiteral("' serial port (WriteFile). Error code: ") + toModbusString(err) +
                                                                 StringLiteral(". ") + getLastErrorText());
                }
            }
            this->state = STATE_BEGIN;
            closeEventHandle(this->oWrite);
            return Status_Good;
        default:
            if (this->serialPortIsOpen())
            {
                this->state = STATE_BEGIN;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusSerialPortPrivateWin::nonBlockingRead()
{
    DWORD c;
    BOOL r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch(this->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_READ:
            zeroOverlapped(this->oRead);
            this->oRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (this->oRead.hEvent == NULL)
            {
                this->state = STATE_BEGIN;
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialWrite, StringLiteral("Error while reading '") + this->settings.portName +
                                                             StringLiteral("' serial port (CreateEvent). Error code: ") + toModbusString(err) +
                                                             StringLiteral(". ") + getLastErrorText());
            }
            this->sz = 0;
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_READ;
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first bytes
            r = ReadFile(this->serialPort, this->buff, this->c_buffSz, &c, &this->oRead);
            if (!r)
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    this->state = STATE_BEGIN;
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->settings.portName +
                                                                StringLiteral("' serial port (ReadFile). Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                this->sz += static_cast<uint16_t>(c);
                if (this->sz == this->c_buffSz) // input buffer is full. Try to handle it
                {
                    this->state = STATE_BEGIN;
                    return Status_Good;
                }
                if (this->sz > this->c_buffSz)
                {
                    this->state = STATE_BEGIN;
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->settings.portName +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
            }
            else if (GetTickCount() - this->timestamp >= this->settingsBase.timeout) // waiting timeout read first byte elapsed
            {
                this->state = STATE_BEGIN;
                closeEventHandle(this->oRead);
                return this->setError(Status_BadSerialReadTimeout, StringLiteral("Error while reading '") + this->settings.portName +
                                                                   StringLiteral("' serial port. Timeout"));
            }
            else
            {
                break;
            }
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_READ_ALL;
            // no need break
        case STATE_WAIT_FOR_READ_ALL:
            // read next bytes
            r = ReadFile(this->serialPort, this->buff+this->sz, this->c_buffSz-this->sz, &c, &this->oRead);
            if (!r)
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    this->state = STATE_BEGIN;
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->settings.portName +
                                                                StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                this->sz += static_cast<uint16_t>(c);
                if (this->sz == this->c_buffSz) // input buffer is full. Try to handle it
                {
                    this->state = STATE_BEGIN;
                    return Status_Good;
                }
                if (this->sz > this->c_buffSz)
                {
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->settings.portName +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
                this->timestampRefresh();
            }
            else if (GetTickCount() - this->timestamp >= this->settings.timeoutInterByte) // waiting timeout read next byte elapsed
            {
                this->state = STATE_BEGIN;
                closeEventHandle(this->oRead);
                return Status_Good;
            }
            return Status_Processing;
        default:
            if (this->serialPortIsOpen())
            {
                this->state = STATE_BEGIN;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

#endif // MODBUSSERIALPORT_P_WIN_H
