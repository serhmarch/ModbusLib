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
    timeoutInterByte(50)
{
}

ModbusSerialPortPrivate *ModbusSerialPortPrivate::create(bool blocking)
{
    return new ModbusSerialPortPrivateWin(blocking);
}

ModbusSerialPort::~ModbusSerialPort()
{
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
            if (isBlocking())
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
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to open '") + d->settings.portName +
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
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to get state of '") + d->settings.portName +
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

            dcb.BaudRate = static_cast<DWORD>(d->settings.baudRate);
            dcb.ByteSize = static_cast<DWORD>(d->settings.dataBits);
            dcb.StopBits = winStopBits(d->settings.stopBits);
            dcb.Parity   = winParity(d->settings.parity);
            winFillDCBFlowControl(&dcb,d->settings.flowControl);

            if (!SetCommState(d->serialPort, &dcb))
            {
                d->serialPortClose();
                DWORD err = GetLastError();
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set state of '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            // Set timeouts
            COMMTIMEOUTS timeouts = { 0 };
            if (isBlocking())
            {
                timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(this->timeoutFirstByte());  // Total timeout for first byte (in milliseconds)
                timeouts.ReadIntervalTimeout = static_cast<DWORD>(d->settings.timeoutInterByte);  // Timeout for inter-byte (in milliseconds)
                timeouts.WriteTotalTimeoutConstant = static_cast<DWORD>(this->timeoutFirstByte());
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
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set timeouts of '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(err));
            }
            //PurgeComm(d->serialPort, PURGE_TXCLEAR|PURGE_RXCLEAR);

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
    return (d->*(d->writeMethod))();
}

StatusCode ModbusSerialPort::read()
{
    ModbusSerialPortPrivateWin *d = d_win(d_ptr);
    return (d->*(d->readMethod))();
}
