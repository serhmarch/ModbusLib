#include "../ModbusPortSerial.h"

#include <Windows.h>

namespace Modbus {

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

PortSerial::Defaults::Defaults() :
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

struct PortSerial::PlatformData
{
    inline bool serialPortIsInvalid() const { return serialPort == INVALID_HANDLE_VALUE; }
    inline bool serialPortIsValid() const { return serialPort != INVALID_HANDLE_VALUE; }
    inline bool serialPortIsOpen() const { return serialPortIsValid(); }
    inline void serialPortClose() { CloseHandle(serialPort); }
    inline void timestampRefresh() { timestamp = GetTickCount(); }
    HANDLE serialPort;
    DWORD timestamp;
};

void PortSerial::constructorPrivate()
{
    m_platformData = new PlatformData;
    m_platformData->serialPort = INVALID_HANDLE_VALUE;
}

void PortSerial::destructorPrivate()
{
    delete m_platformData;
}

Handle PortSerial::handle() const
{
    return reinterpret_cast<Handle>(m_platformData->serialPort);
}

StatusCode PortSerial::open()
{
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (m_state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
        {
            clearChanged();
            if (isOpen())
            {
                m_state = STATE_BEGIN;
                return Status_Good;
            }

            m_platformData->serialPort = CreateFileA(
                m_settings.portName.c_str(),  // Port name
                GENERIC_READ | GENERIC_WRITE, // Read and write access
                0,                            // No sharing
                NULL,                         // No security attributes
                OPEN_EXISTING,                // Opens existing port
                0,                            // Disables overlapped I/O
                NULL                          // No template file
                );

            if (m_platformData->serialPortIsInvalid())
            {
                return setError(Status_BadSerialOpen, StringLiteral("Failed to open serial port. Error code: ") + toString(GetLastError()));
            }

            // Configure the serial port
            DCB dcbSerialParams = { 0 };
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            if (!GetCommState(m_platformData->serialPort, &dcbSerialParams))
            {
                m_platformData->serialPortClose();
                return setError(Status_BadSerialOpen, StringLiteral("Failed to get serial port state. Error code: ") + toString(GetLastError()));
            }

            dcbSerialParams.BaudRate = static_cast<DWORD>(m_settings.baudRate);
            dcbSerialParams.ByteSize = static_cast<DWORD>(m_settings.dataBits);
            dcbSerialParams.StopBits = winStopBits(m_settings.stopBits);
            dcbSerialParams.Parity   = winParity(m_settings.parity);
            // TODO: FlowControl

            if (!SetCommState(m_platformData->serialPort, &dcbSerialParams))
            {
                m_platformData->serialPortClose();
                return setError(Status_BadSerialOpen, StringLiteral("Failed to set serial port state. Error code: ") + toString(GetLastError()));
            }

            // Set timeouts
            COMMTIMEOUTS timeouts = { 0 };
            if (isBlocking())
            {
                timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(m_settings.timeoutFirstByte);  // Total timeout for first byte (in milliseconds)
                timeouts.ReadIntervalTimeout = static_cast<DWORD>(m_settings.timeoutInterByte);  // Timeout for inter-byte (in milliseconds)
            }
            else
            {
                timeouts.ReadIntervalTimeout = MAXDWORD; // No read timeout
            }

            if (!SetCommTimeouts(m_platformData->serialPort, &timeouts))
            {
                m_platformData->serialPortClose();
                return setError(Status_BadSerialOpen, StringLiteral("Failed to set timeouts. Error code: ") + toString(GetLastError()));
            }
        }
            return Status_Good;
        default:
            if (!isOpen())
            {
                m_state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode PortSerial::close()
{
    if (m_platformData->serialPortIsOpen())
    {
        m_platformData->serialPortClose();
        //setMessage(QString("Serial port '%1' is closed").arg(serialPortName()));
    }
    m_state = STATE_CLOSED;
    return Status_Good;
}

bool PortSerial::isOpen() const
{
    return m_platformData->serialPortIsOpen();
}

StatusCode PortSerial::write()
{
    DWORD c;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (m_state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_WRITE:
            m_platformData->timestampRefresh();
            m_state = STATE_WAIT_FOR_WRITE;
            // no need break
        case STATE_WAIT_FOR_WRITE:
        case STATE_WAIT_FOR_WRITE_ALL:
            // Note: clean read buffer from garbage before write
            PurgeComm(m_platformData->serialPort, PURGE_RXCLEAR);
            if (WriteFile(m_platformData->serialPort, m_buff, m_sz, &c, NULL))
            {
                emitTx(m_buff, m_sz);
                m_state = STATE_BEGIN;
                return Status_Good;
            }
            else
            {
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    m_state = STATE_BEGIN;
                    return setError(Status_BadSerialWrite, StringLiteral("Error while writing serial port"));
                }
            }
            break;
        default:
            if (isOpen())
            {
                m_state = STATE_BEGIN;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode PortSerial::read()
{
    DWORD c;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch(m_state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_READ:
            m_platformData->timestampRefresh();
            m_state = STATE_WAIT_FOR_READ;
            m_sz = 0;
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first byte state
            c = 0;
            if (!ReadFile(m_platformData->serialPort, m_buff, c_buffSz, &c, NULL))
            {
                if (GetLastError() != ERROR_IO_PENDING)
                {
                    m_state = STATE_BEGIN;
                    return setError(Status_BadSerialRead, StringLiteral("Error while reading serial port "));
                }
            }
            if (c > 0)
            {
                m_sz += static_cast<uint16_t>(c);
                if (m_sz > c_buffSz)
                {
                    m_state = STATE_BEGIN;
                    return setError(Status_BadReadBufferOverflow, StringLiteral("Serial port's '%1' read-buffer overflow"));
                }
                if (isBlocking())
                {
                    emitRx(m_buff, m_sz);
                    m_state = STATE_BEGIN;
                    return Status_Good;
                }
            }
            else if (GetTickCount() - m_platformData->timestamp >= timeoutFirstByte()) // waiting timeout read first byte elapsed
            {
                m_state = STATE_BEGIN;
                return setError(Status_BadSerialRead, StringLiteral("Error while reading serial port "));
            }
            else
            {
                break;
            }
            m_platformData->timestampRefresh();
            m_state = STATE_WAIT_FOR_READ_ALL;
            // no need break
        case STATE_WAIT_FOR_READ_ALL:
            // next bytes state
            c = 0;
            if (!ReadFile(m_platformData->serialPort, m_buff, c_buffSz, &c, NULL))
            {
                if (GetLastError() != ERROR_IO_PENDING)
                {
                    m_state = STATE_BEGIN;
                    return setError(Status_BadSerialRead, StringLiteral("Error while reading serial port "));
                }
            }

            if (c > 0)
            {
                m_sz += static_cast<uint16_t>(c);
                if (m_sz > c_buffSz)
                    return setError(Modbus::Status_BadReadBufferOverflow, StringLiteral("Serial port's read-buffer overflow"));
                m_platformData->timestampRefresh();
            }
            else if (GetTickCount() - m_platformData->timestamp >= timeoutInterByte()) // waiting timeout read next byte elapsed
            {
                emitRx(m_buff, m_sz);
                m_state = STATE_BEGIN;
                return Status_Good;
            }
            return Status_Processing;
        default:
            if (isOpen())
            {
                m_state = STATE_BEGIN;
                fRepeatAgain = true;
            }
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

} // namespace Modbus
