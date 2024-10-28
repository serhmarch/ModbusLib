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

            d->serialPort = CreateFileA(
                d->settings.portName.c_str(), // Port name
                GENERIC_READ | GENERIC_WRITE, // Read and write access
                0,                            // No sharing
                NULL,                         // No security attributes
                OPEN_EXISTING,                // Opens existing port
                0,                            // Disables overlapped I/O
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
            DCB dcbSerialParams = { 0 };
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            if (!GetCommState(d->serialPort, &dcbSerialParams))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to get serial port '") + d->settings.portName +
                                                         StringLiteral("'state. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            dcbSerialParams.BaudRate = static_cast<DWORD>(d->settings.baudRate);
            dcbSerialParams.ByteSize = static_cast<DWORD>(d->settings.dataBits);
            dcbSerialParams.StopBits = winStopBits(d->settings.stopBits);
            dcbSerialParams.Parity   = winParity(d->settings.parity);
            // TODO: FlowControl

            if (!SetCommState(d->serialPort, &dcbSerialParams))
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

//void dummyOverlappedCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
//{
//    // Note: dummy
//}

StatusCode ModbusSerialPort::write()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    DWORD c;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_WRITE:
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_WRITE;
            // no need break
        case STATE_WAIT_FOR_WRITE:
        case STATE_WAIT_FOR_WRITE_ALL:
        {
            // Note: clean read buffer from garbage before write
            PurgeComm(d->serialPort, PURGE_RXCLEAR);
            //if (d->isBlocking())
            //    r = WriteFile(d->serialPort, d->buff, d->sz, &c, NULL);
            //else
            //{
            //    OVERLAPPED o;
            //    o.Internal = 0;
            //    o.InternalHigh = 0;
            //    o.Offset = 0xFFFFFFFF;
            //    o.OffsetHigh = 0xFFFFFFFF;
            //    o.Pointer = 0;
            //    o.hEvent = 0;
            //    r = WriteFileEx(d->serialPort, d->buff, d->sz, &o, &dummyOverlappedCompletionRoutine);
            //}
            BOOL r = WriteFile(d->serialPort, d->buff, d->sz, &c, NULL);
            if (r)
            {
                d->state = STATE_BEGIN;
                return Status_Good;
            }
            else
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    d->state = STATE_BEGIN;
                    return d->setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port '") + d->settings.portName +
                                                              StringLiteral("'. Error code: ") + toModbusString(err) +
                                                              StringLiteral(". ") + getLastErrorText());
                }
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
    DWORD c;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch(d->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_READ:
            d->timestampRefresh();
            d->state = STATE_WAIT_FOR_READ;
            d->sz = 0;
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first byte state
            c = 0;
            if (!ReadFile(d->serialPort, d->buff, d->c_buffSz, &c, NULL))
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    d->state = STATE_BEGIN;
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
                    d->state = STATE_BEGIN;
                    return d->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                                     StringLiteral("'. Read buffer overflow"));
                }
                if (isBlocking())
                {
                    d->state = STATE_BEGIN;
                    return Status_Good;
                }
            }
            else if (GetTickCount() - d->timestamp >= timeoutFirstByte()) // waiting timeout read first byte elapsed
            {
                d->state = STATE_BEGIN;
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
            // next bytes state
            c = 0;
            if (!ReadFile(d->serialPort, d->buff+d->sz, d->c_buffSz, &c, NULL))
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    d->state = STATE_BEGIN;
                    return d->setError(Status_BadSerialRead, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                             StringLiteral("'. Error code: ") + toModbusString(err) +
                                                             StringLiteral(". ") + getLastErrorText());
                }
            }

            if (c > 0)
            {
                d->sz += static_cast<uint16_t>(c);
                if (d->sz > d->c_buffSz)
                    return d->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading serial port '") + d->settings.portName +
                                                                     StringLiteral("'. Read buffer overflow"));
                d->timestampRefresh();
            }
            else if (GetTickCount() - d->timestamp >= timeoutInterByte()) // waiting timeout read next byte elapsed
            {
                d->state = STATE_BEGIN;
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
