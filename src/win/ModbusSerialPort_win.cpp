#include "../ModbusSerialPort.h"
#include "ModbusSerialPort_p_win.h"

BYTE winParity(Parity v)
{
    switch (v)
    {
    case NoParity   : return NOPARITY   ;
    case EvenParity : return ODDPARITY  ;
    case OddParity  : return EVENPARITY ;
    case SpaceParity: return MARKPARITY ;
    case MarkParity : return SPACEPARITY;
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

ModbusSerialPort::Defaults::Defaults() :
    portName(StringLiteral("COM1")),
    baudRate(9600),
    dataBits(8),
    parity(NoParity),
    stopBits(OneStop),
    flowControl(NoFlowControl),
    timeoutFirstByte(1000),
    timeoutInterByte(5)
{
}

ModbusSerialPortPrivate *ModbusSerialPortPrivate::create(bool blocking)
{
    return new ModbusSerialPortPrivateWin(blocking);
}

ModbusSerialPort::~ModbusSerialPort()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    if (d->serialPortIsOpen())
        d->serialPortClose();
}

Handle ModbusSerialPort::handle() const
{
    return reinterpret_cast<Handle>(d_win(d_ptr)->serialPort);
}

StatusCode ModbusSerialPort::open()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
        {
            d->clearChanged();
            if (isOpen())
            {
                d->state = STATE_BEGIN;
                return Status_Good;
            }

            DWORD dwFlags;
            if  (isBlocking())
                dwFlags = 0; // Disables overlapped I/O
            else
                dwFlags = FILE_FLAG_OVERLAPPED; // For asynchronous I/O

            d->serialPort = CreateFileA(
                d->settings.portName.c_str(), // Port name
                GENERIC_READ | GENERIC_WRITE, // Read and write access
                0,                            // No sharing
                NULL,                         // No security attributes
                OPEN_EXISTING,                // Opens existing port
                dwFlags,                      // Device attributes and flags
                NULL                          // No template file
                );

            if (d->serialPortIsInvalid())
            {
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to open serial port '") + d->settings.portName +
                                                         StringLiteral("'. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            // Configure the serial port
            DCB dcb = { 0 };
            dcb.DCBlength = sizeof(dcb);
            if (!GetCommState(d->serialPort, &dcb))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to get serial port '") + d->settings.portName +
                                                         StringLiteral("'state. Error code: ") + toModbusString(err) +
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

            dcb.BaudRate = static_cast<DWORD>(d->settings.baudRate);
            dcb.ByteSize = static_cast<DWORD>(d->settings.dataBits);
            dcb.StopBits = winStopBits(d->settings.stopBits);
            dcb.Parity   = winParity(d->settings.parity);
            winFillDCBFlowControl(&dcb,d->settings.flowControl);

            if (!SetCommState(d->serialPort, &dcb))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set serial port '") + d->settings.portName +
                                                         StringLiteral("'state. Error code: ") + toModbusString(err));
            }

            // Set timeouts
            COMMTIMEOUTS timeouts = { 0 };
            if (isBlocking())
            {
                timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(this->timeoutFirstByte());  // Total timeout for first byte (in milliseconds)
                timeouts.ReadIntervalTimeout = static_cast<DWORD>(d->settings.timeoutInterByte);  // Timeout for inter-byte (in milliseconds)
            }
            else
            {
                timeouts.ReadIntervalTimeout = MAXDWORD; // No read timeout
            }

            if (!SetCommTimeouts(d->serialPort, &timeouts))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set timeouts of serial port '") + d->settings.portName +
                                                         StringLiteral("'. Error code: ") + toModbusString(err));
            }
            PurgeComm(d->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);

        }
            return Status_Good;
        default:
            if (!isOpen())
            {
                d->state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusSerialPort::close()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    if (d->serialPortIsOpen())
    {
        d->serialPortClose();
        //setMessage(QString("Serial port '%1' is closed").arg(serialPortName()));
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusSerialPort::isOpen() const
{
    return d_win(d_ptr)->serialPortIsOpen();
}

StatusCode ModbusSerialPort::write()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    BOOL r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_WRITE:
            // Note: clean read buffer from garbage before write
            PurgeComm(d->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);
            r = SetCommMask(d->serialPort, EV_TXEMPTY);
            if (!r)
            {
                d->state = STATE_BEGIN;
                DWORD err = GetLastError();
                CancelIo(d->serialPort);
                return d->setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                          StringLiteral("' (SetCommMask). Error code: ") + toModbusString(err) +
                                                          StringLiteral(". ") + getLastErrorText());
            }
            ::ZeroMemory(&d->oWrite, sizeof(d->oWrite));
            d->oWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (d->oWrite.hEvent == NULL)
            {
                d->state = STATE_BEGIN;
                DWORD err = GetLastError();
                CancelIo(d->serialPort);
                return d->setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                          StringLiteral("' (CreateEvent). Error code: ") + toModbusString(err) +
                                                          StringLiteral(". ") + getLastErrorText());
            }
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_WRITE;
            // no need break
        case STATE_WAIT_FOR_WRITE:
            r =  WriteFile(d->serialPort, d->buff, d->sz, NULL, &d->oWrite);
            if (!r)
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    d->state = STATE_BEGIN;
                    CloseHandle(d->oWrite.hEvent);
                    CancelIo(d->serialPort);
                    return d->setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                              StringLiteral("' (WriteFile). Error code: ") + toModbusString(err) +
                                                              StringLiteral(". ") + getLastErrorText());
                }
            }
            else
            {
                d->state = STATE_BEGIN;
                CloseHandle(d->oWrite.hEvent);
                CancelIo(d->serialPort);
                return Status_Good;
            }

            d->state = STATE_WAIT_FOR_WRITE_ALL;
            // no need break
        case STATE_WAIT_FOR_WRITE_ALL:
        {
            DWORD dwCommEvent;
            WaitCommEvent(d->serialPort, &dwCommEvent, &d->oWrite);
            if (!dwCommEvent)
            {
                d->state = STATE_BEGIN;
                DWORD err = GetLastError();
                CloseHandle(d->oWrite.hEvent);
                return d->setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                          StringLiteral("' (WaitCommEvent). Error code: ") + toModbusString(err) +
                                                          StringLiteral(". ") + getLastErrorText());
            }
            else if (dwCommEvent & EV_TXEMPTY)
            {
                d->state = STATE_BEGIN;
                CloseHandle(d->oWrite.hEvent);
                CancelIo(d->serialPort);
                return Status_Good;
            }
            else if (GetTickCount() - d->timestamp >= timeout()) // waiting timeout write
            {
                d->state = STATE_BEGIN;
                CloseHandle(d->oWrite.hEvent);
                CancelIo(d->serialPort);
                return d->setError(Status_BadSerialWriteTimeout, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                                 StringLiteral("'. Timeout"));
            }

        }
            break;
        default:
            if (isOpen())
            {
                d->state = STATE_BEGIN;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusSerialPort::read()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    DWORD c, dwCommEvent;
    BOOL r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch(d->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_READ:
            r = SetCommMask(d->serialPort, EV_RXCHAR);
            if (!r)
            {
                d->state = STATE_BEGIN;
                CancelIo(d->serialPort);
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                              StringLiteral("' (SetCommMask). Error code: ") + toModbusString(err) +
                                                              StringLiteral(". ") + getLastErrorText());
            }
            ::ZeroMemory(&d->oRead, sizeof(d->oRead));
            d->oRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (d->oRead.hEvent == NULL)
            {
                d->state = STATE_BEGIN;
                CancelIo(d->serialPort);
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialWrite, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                          StringLiteral("' (CreateEvent). Error code: ") + toModbusString(err) +
                                                          StringLiteral(". ") + getLastErrorText());
            }
            d->sz = 0;
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_READ;
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first bytes
            r = ReadFile(d->serialPort, d->buff, d->c_buffSz, &c, &d->oRead);
            if (!r)
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    d->state = STATE_BEGIN;
                    CloseHandle(d->oRead.hEvent);
                    CancelIo(d->serialPort);
                    return d->setError(Status_BadSerialRead, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                             StringLiteral("' (ReadFile). Error code: ") + toModbusString(err) +
                                                             StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                d->sz += static_cast<uint16_t>(c);
                if (d->sz > d->c_buffSz)
                {
                    d->state = STATE_BEGIN;
                    CloseHandle(d->oRead.hEvent);
                    CancelIo(d->serialPort);
                    return d->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                                     StringLiteral("'. Read buffer overflow"));
                }
            }
            else if (GetTickCount() - d->timestamp >= timeoutFirstByte()) // waiting timeout read first byte elapsed
            {
                d->state = STATE_BEGIN;
                CloseHandle(d->oRead.hEvent);
                CancelIo(d->serialPort);
                return d->setError(Status_BadSerialReadTimeout, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                                StringLiteral("'. Timeout"));
            }
            else
            {
                break;
            }
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_READ_ALL;
            // no need break
        case STATE_WAIT_FOR_READ_ALL:
            // read next bytes
            r = ReadFile(d->serialPort, d->buff+d->sz, d->c_buffSz, &c, &d->oRead);
            if (!r)
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    d->state = STATE_BEGIN;
                    CloseHandle(d->oRead.hEvent);
                    CancelIo(d->serialPort);
                    return d->setError(Status_BadSerialRead, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                             StringLiteral("'. Error code: ") + toModbusString(err) +
                                                             StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                d->sz += static_cast<uint16_t>(c);
                if (d->sz > d->c_buffSz)
                {
                    CloseHandle(d->oRead.hEvent);
                    CancelIo(d->serialPort);
                    return d->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                                     StringLiteral("'. Read buffer overflow"));
                }
                d->timestampRefresh();
            }
            else if (GetTickCount() - d->timestamp >= timeoutInterByte()) // waiting timeout read next byte elapsed
            {
                d->state = STATE_BEGIN;
                CloseHandle(d->oRead.hEvent);
                CancelIo(d->serialPort);
                return Status_Good;
            }
            return Status_Processing;
        default:
            if (isOpen())
            {
                d->state = STATE_BEGIN;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}
