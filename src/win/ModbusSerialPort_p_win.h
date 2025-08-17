#ifndef MODBUSSERIALPORT_P_WIN_H
#define MODBUSSERIALPORT_P_WIN_H

#include <Windows.h>

#include "../ModbusSerialPort_p.h"

BYTE winParity(Parity v)
{
    switch (v)
    {
    case NoParity   : return NOPARITY   ;
    case EvenParity : return EVENPARITY ;
    case OddParity  : return ODDPARITY  ;
    case SpaceParity: return SPACEPARITY;
    case MarkParity : return MARKPARITY ;
    default         : return NOPARITY   ;
    }
}

BYTE winStopBits(StopBits v)
{
    switch (v)
    {
    case OneStop       : return ONESTOPBIT  ;
    case OneAndHalfStop: return ONE5STOPBITS;
    case TwoStop       : return TWOSTOPBITS ;
    default            : return ONESTOPBIT  ;
    }
}

void winFillDCBFlowControl(DCB *dcb, FlowControl v)
{
    dcb->fInX = FALSE;
    dcb->fOutX = FALSE;
    dcb->fOutxCtsFlow = FALSE;
    if (dcb->fRtsControl == RTS_CONTROL_HANDSHAKE)
        dcb->fRtsControl = RTS_CONTROL_DISABLE;
    switch (v)
    {
    case NoFlowControl:
        break;
    case SoftwareControl:
        dcb->fInX = TRUE;
        dcb->fOutX = TRUE;
        break;
    case HardwareControl:
        dcb->fOutxCtsFlow = TRUE;
        dcb->fRtsControl = RTS_CONTROL_HANDSHAKE;
        break;
    default:
        break;
    }
}

class ModbusSerialPortPrivateWin : public ModbusSerialPortPrivate
{
public:
    ModbusSerialPortPrivateWin(uint16_t maxBuffSize, bool blocking) :
        ModbusSerialPortPrivate(maxBuffSize, blocking)
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
    Modbus::Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

public:
    StatusCode blockingWrite();
    StatusCode blockingRead();
    StatusCode nonBlockingWrite();
    StatusCode nonBlockingRead();

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

Handle ModbusSerialPortPrivateWin::handle() const
{
    return reinterpret_cast<Handle>(this->serialPort);
}

StatusCode ModbusSerialPortPrivateWin::open()
{
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (this->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
        {
            this->clearChanged();
            if (isOpen())
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }

            DWORD dwFlags;
            if (isBlocking())
                dwFlags = 0; // Disables overlapped I/O
            else
                dwFlags = FILE_FLAG_OVERLAPPED; // For asynchronous I/O


            char swincom[MAX_PATH];
            snprintf(swincom, MAX_PATH-1, "\\\\.\\%s", this->portName().c_str());
            this->serialPort = CreateFileA(
                swincom,                      // Port name
                GENERIC_READ | GENERIC_WRITE, // Read and write access
                0,                            // No sharing
                NULL,                         // No security attributes
                OPEN_EXISTING,                // Opens existing port
                dwFlags,                      // Device attributes and flags
                NULL                          // No template file
                );

            if (this->serialPortIsInvalid())
            {
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to open '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            // Configure the serial port
            DCB dcb = { 0 };
            dcb.DCBlength = sizeof(dcb);
            if (!GetCommState(this->serialPort, &dcb))
            {
                this->serialPortClose();
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to get state of '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            dcb.fBinary = TRUE;
            dcb.fAbortOnError = FALSE;
            dcb.fNull = FALSE;
            dcb.fErrorChar = FALSE;

            if (dcb.fDtrControl == DTR_CONTROL_HANDSHAKE)
                dcb.fDtrControl = DTR_CONTROL_DISABLE;

            if (dcb.fRtsControl != RTS_CONTROL_HANDSHAKE)
                dcb.fRtsControl = RTS_CONTROL_DISABLE;

            dcb.BaudRate = static_cast<DWORD>(this->settings.baudRate);
            dcb.ByteSize = static_cast<DWORD>(this->settings.dataBits);
            dcb.StopBits = winStopBits(this->settings.stopBits);
            dcb.Parity   = winParity(this->settings.parity);
            winFillDCBFlowControl(&dcb,this->settings.flowControl);

            if (!SetCommState(this->serialPort, &dcb))
            {
                this->serialPortClose();
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to set state of '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            // Set timeouts
            COMMTIMEOUTS timeouts = { 0 };
            if (isBlocking())
            {
                timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(this->timeoutFirstByte());  // Total timeout for first byte (in milliseconds)
                timeouts.ReadIntervalTimeout = static_cast<DWORD>(this->settings.timeoutInterByte);  // Timeout for inter-byte (in milliseconds)
                timeouts.WriteTotalTimeoutConstant = static_cast<DWORD>(this->timeoutFirstByte());
                timeouts.WriteTotalTimeoutMultiplier = 0;
            }
            else
            {
                timeouts.ReadIntervalTimeout = MAXDWORD; // No read timeout
            }

            if (!SetCommTimeouts(this->serialPort, &timeouts))
            {
                this->serialPortClose();
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to set timeouts of '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err));
            }
            //PurgeComm(this->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);

        }
            return Status_Good;
        default:
            if (!isOpen())
            {
                this->state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusSerialPortPrivateWin::close()
{
    if (this->serialPortIsOpen())
    {
        this->serialPortClose();
        //setMessage(QString("Serial port '%1' is closed").arg(serialPortName()));
    }
    this->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusSerialPortPrivateWin::isOpen() const
{
    return this->serialPortIsOpen();
}

StatusCode ModbusSerialPortPrivateWin::write()
{
    return (this->*(this->writeMethod))();
}

StatusCode ModbusSerialPortPrivateWin::read()
{
    return (this->*(this->readMethod))();
}

StatusCode ModbusSerialPortPrivateWin::blockingWrite()
{
    BOOL r;
    this->state = STATE_OPENED;
    PurgeComm(this->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);
    r =  WriteFile(this->serialPort, this->buff, this->sz, NULL, NULL);
    if (!r)
    {
        DWORD err = GetLastError();
        return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->portName() +
                                                     StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                     StringLiteral(". ") + getLastErrorText());
    }
    return Status_Good;
}

StatusCode ModbusSerialPortPrivateWin::blockingRead()
{
    BOOL r;
    DWORD c;
    this->state = STATE_OPENED;
    r = ReadFile(this->serialPort, this->buff, this->c_buffSz, &c, NULL);
    if (!r)
    {
        DWORD err = GetLastError();
        return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
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
        case STATE_OPENED:
        case STATE_PREPARE_TO_WRITE:
            // Note: clean read buffer from garbage before write
            PurgeComm(this->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);
            zeroOverlapped(this->oWrite);
            this->oWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (this->oWrite.hEvent == NULL)
            {
                this->state = STATE_OPENED;
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->portName() +
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
                    this->state = STATE_OPENED;
                    closeEventHandle(this->oWrite);
                    return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->portName() +
                                                                 StringLiteral("' serial port (WriteFile). Error code: ") + toModbusString(err) +
                                                                 StringLiteral(". ") + getLastErrorText());
                }
            }
            this->state = STATE_OPENED;
            closeEventHandle(this->oWrite);
            return Status_Good;
        default:
            if (this->serialPortIsOpen())
            {
                this->state = STATE_OPENED;
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
        case STATE_OPENED:
        case STATE_PREPARE_TO_READ:
            zeroOverlapped(this->oRead);
            this->oRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (this->oRead.hEvent == NULL)
            {
                this->state = STATE_OPENED;
                DWORD err = GetLastError();
                return this->setError(Status_BadSerialWrite, StringLiteral("Error while reading '") + this->portName() +
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
                    this->state = STATE_OPENED;
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
                                                                StringLiteral("' serial port (ReadFile). Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                this->sz += static_cast<uint16_t>(c);
                if ((this->settings.timeoutInterByte == 0) || // timeoutInterByte = 0 means no need to wait next bytes
                    (this->sz == this->c_buffSz))             // input buffer is full. Try to handle it
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                if (this->sz > this->c_buffSz)
                {
                    this->state = STATE_OPENED;
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->portName() +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
            }
            else if (GetTickCount() - this->timestamp >= this->timeoutFirstByte()) // waiting timeout read first byte elapsed
            {
                this->state = STATE_OPENED;
                closeEventHandle(this->oRead);
                return this->setError(Status_BadSerialReadTimeout, StringLiteral("Error while reading '") + this->portName() +
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
                    this->state = STATE_OPENED;
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
                                                                StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                this->sz += static_cast<uint16_t>(c);
                if (this->sz == this->c_buffSz) // input buffer is full. Try to handle it
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                if (this->sz > this->c_buffSz)
                {
                    closeEventHandle(this->oRead);
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->portName() +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
                this->timestampRefresh();
            }
            else if (GetTickCount() - this->timestamp >= this->settings.timeoutInterByte) // waiting timeout read next byte elapsed
            {
                this->state = STATE_OPENED;
                closeEventHandle(this->oRead);
                return Status_Good;
            }
            return Status_Processing;
        default:
            if (this->serialPortIsOpen())
            {
                this->state = STATE_OPENED;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

inline ModbusSerialPortPrivateWin *d_win(ModbusPortPrivate *d_ptr) { return static_cast<ModbusSerialPortPrivateWin*>(d_ptr); }

#endif // MODBUSSERIALPORT_P_WIN_H
