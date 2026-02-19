#include "../ModbusSerialPort.h"
#include "ModbusSerialPort_p_win.h"

ModbusSerialPortPrivate *ModbusSerialPortPrivate::create(ModbusFramePrivate *f, bool blocking)
{
    return new ModbusSerialPortPrivateWin(f, blocking);
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
            if (this->isOpen())
            {
                if (d->isChanged())
                    this->close();
                else
                {
                    d->state = STATE_OPENED;
                    return Status_Good;
                }
            }
            d->clearChanged();
            DWORD dwFlags;
            if (d->isBlocking())
                dwFlags = 0; // Disables overlapped I/O
            else
                dwFlags = FILE_FLAG_OVERLAPPED; // For asynchronous I/O


            char swincom[MAX_PATH];
            snprintf(swincom, MAX_PATH-1, "\\\\.\\%s", d->portName().data());
            d->serialPort = CreateFileA(
                swincom,                      // Port name
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
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to open '") + d->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            // Configure the serial port
            DCB dcb = { 0 };
            dcb.DCBlength = sizeof(dcb);
            if (!GetCommState(d->serialPort, &dcb))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to get state of '") + d->portName() +
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

            dcb.BaudRate = static_cast<DWORD>(d->baudRate());
            dcb.ByteSize = static_cast<DWORD>(d->dataBits());
            dcb.StopBits = winStopBits(d->stopBits());
            dcb.Parity   = winParity(d->parity());
            winFillDCBFlowControl(&dcb,d->flowControl());

            if (!SetCommState(d->serialPort, &dcb))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set state of '") + d->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            // Set timeouts
            COMMTIMEOUTS timeouts = { 0 };
            if (d->isBlocking())
            {
                timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(d->timeoutFirstByte());  // Total timeout for first byte (in milliseconds)
                timeouts.ReadIntervalTimeout = static_cast<DWORD>(d->timeoutInterByte());  // Timeout for inter-byte (in milliseconds)
                timeouts.WriteTotalTimeoutConstant = static_cast<DWORD>(d->timeoutFirstByte());
                timeouts.WriteTotalTimeoutMultiplier = 0;
            }
            else
            {
                timeouts.ReadIntervalTimeout = MAXDWORD; // No read timeout
            }

            if (!SetCommTimeouts(d->serialPort, &timeouts))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set timeouts of '") + d->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err));
            }
            //PurgeComm(d->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);

        }
            return Status_Good;
        default:
            if (this->isOpen() && !d->isChanged())
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
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
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusSerialPort::isOpen() const
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    return d->serialPortIsOpen();
}

StatusCode ModbusSerialPort::write()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    return (d->*(d->writeMethod))();
}

StatusCode ModbusSerialPort::read()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    return (d->*(d->readMethod))();
}

