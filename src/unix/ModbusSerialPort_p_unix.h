#ifndef MODBUSSERIALPORT_P_UNIX_H
#define MODBUSSERIALPORT_P_UNIX_H

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "Modbus_unix.h"
#include "../ModbusSerialPort_p.h"


class ModbusSerialPortPrivateUnix : public ModbusSerialPortPrivate
{
public:
    ModbusSerialPortPrivateUnix(bool blocking) :
        ModbusSerialPortPrivate(blocking)
    {
        this->serialPort = -1;
        this->timestamp = 0;
        if (blocking)
        {
            writeMethod = &ModbusSerialPortPrivateUnix::blockingWrite;
            readMethod  = &ModbusSerialPortPrivateUnix::blockingRead ;
        }
        else
        {
            writeMethod = &ModbusSerialPortPrivateUnix::nonBlockingWrite;
            readMethod  = &ModbusSerialPortPrivateUnix::nonBlockingRead ;
        }
    }

public:
    inline bool serialPortIsInvalid() const { return serialPort == -1; }
    inline bool serialPortIsValid() const { return serialPort != -1; }
    inline bool serialPortIsOpen() const { return serialPortIsValid(); }
    inline void serialPortClose() { close(serialPort); serialPort = -1; }
    inline void timestampRefresh() { timestamp = timer(); }

public:
    StatusCode blockingWrite();
    StatusCode blockingRead ();
    StatusCode nonBlockingWrite();
    StatusCode nonBlockingRead ();

public:
    int serialPort;
    Timer timestamp;

    typedef StatusCode (ModbusSerialPortPrivateUnix::*RWMethodPtr_t)();
    RWMethodPtr_t writeMethod;
    RWMethodPtr_t readMethod ;
};

inline ModbusSerialPortPrivateUnix *d_unix(ModbusPortPrivate *this_ptr) { return static_cast<ModbusSerialPortPrivateUnix*>(this_ptr); }

StatusCode ModbusSerialPortPrivateUnix::blockingWrite()
{
    int c;
    this->state = STATE_BEGIN;
    tcflush(this->serialPort, TCIFLUSH);
    c = ::write(this->serialPort, this->buff, this->sz);
    if (c < 0)
    {
        this->state = STATE_BEGIN;
        return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->settings.portName +
                                                     StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                     StringLiteral(". ") + getLastErrorText());
    }
    return Status_Good;
}

StatusCode ModbusSerialPortPrivateUnix::blockingRead()
{
    int c;
    this->state = STATE_BEGIN;
    c = ::read(this->serialPort, this->buff, this->c_buffSz);
    if (c < 0)
    {
        this->state = STATE_BEGIN;
        return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->settings.portName +
                                                    StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                    StringLiteral(". ") + getLastErrorText());
    }
    this->sz = static_cast<uint16_t>(c);
    return Status_Good;
}

StatusCode ModbusSerialPortPrivateUnix::nonBlockingWrite()
{
    int c;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (this->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_WRITE:
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_WRITE;
            // no need break
        case STATE_WAIT_FOR_WRITE:
        case STATE_WAIT_FOR_WRITE_ALL:
            // Note: clean read buffer from garbage before write
            tcflush(this->serialPort, TCIFLUSH);
            c = ::write(this->serialPort, this->buff, this->sz);
            if (c >= 0)
            {
                this->state = STATE_BEGIN;
                return Status_Good;
            }
            else
            {
                if (errno != EWOULDBLOCK)
                {
                    this->state = STATE_BEGIN;
                    return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->settings.portName +
                                                                 StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                                 StringLiteral(". ") + getLastErrorText());
                }
            }
            break;
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

StatusCode ModbusSerialPortPrivateUnix::nonBlockingRead()
{
    int c;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch(this->state)
        {
        case STATE_BEGIN:
        case STATE_PREPARE_TO_READ:
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_READ;
            this->sz = 0;
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first byte state
            c = ::read(this->serialPort, this->buff, this->c_buffSz);
            if (c < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                    this->state = STATE_BEGIN;
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->settings.portName +
                                                                StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
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
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->settings.portName +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
            }
            else if (timer() - this->timestamp >= this->settingsBase.timeout) // waiting timeout read first byte elapsed
            {
                this->state = STATE_BEGIN;
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
            // next bytes state
            c = ::read(this->serialPort, this->buff+this->sz, this->c_buffSz-this->sz);
            if (c < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                    this->state = STATE_BEGIN;
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->settings.portName +
                                                                StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
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
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->settings.portName +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                this->timestampRefresh();
            }
            else if (timer() - this->timestamp >= this->settings.timeoutInterByte) // waiting timeout read next byte elapsed
            {
                this->state = STATE_BEGIN;
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

#endif // MODBUSSERIALPORT_P_UNIX_H
