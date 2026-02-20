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
    ModbusSerialPortPrivateUnix(ModbusFramePrivate *f, bool blocking) :
        ModbusSerialPortPrivate(f, blocking)
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

    ~ModbusSerialPortPrivateUnix()
    {
        if (this->serialPortIsOpen())
            this->serialPortClose();
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

StatusCode ModbusSerialPortPrivateUnix::blockingWrite()
{
    int c;
    this->state = STATE_OPENED;
    tcflush(this->serialPort, TCIFLUSH);
    c = ::write(this->serialPort, this->buff(), this->buffSize());
    if (c < 0)
    {
        return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->portName() +
                                                     StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                     StringLiteral(". ") + getLastErrorText());
    }
    return Status_Good;
}

StatusCode ModbusSerialPortPrivateUnix::blockingRead()
{
    int c;
    this->state = STATE_OPENED;
    c = ::read(this->serialPort, this->buff(), this->buffMaxSize());
    if (c < 0)
    {
        return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
                                                    StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                    StringLiteral(". ") + getLastErrorText());
    }
    this->setBuffSize(static_cast<uint16_t>(c));
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
        case STATE_OPENED:
        case STATE_PREPARE_TO_WRITE:
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_WRITE;
            // no need break
        case STATE_WAIT_FOR_WRITE:
        case STATE_WAIT_FOR_WRITE_ALL:
            // Note: clean read buffer from garbage before write
            tcflush(this->serialPort, TCIFLUSH);
            c = ::write(this->serialPort, this->buff(), this->buffSize());
            if (c >= 0)
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }
            else
            {
                if (errno != EWOULDBLOCK)
                {
                    this->state = STATE_OPENED;
                    return this->setError(Status_BadSerialWrite, StringLiteral("Error while writing '") + this->portName() +
                                                                 StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                                 StringLiteral(". ") + getLastErrorText());
                }
            }
            break;
        default:
            if (this->serialPortIsOpen())
            {
                this->state = STATE_OPENED;
                fRepeatAgain = true;
            }
            else
                return this->setError(Status_BadSerialWrite, StringLiteral("Internal state error"));
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
        case STATE_OPENED:
        case STATE_PREPARE_TO_READ:
            this->timestampRefresh();
            this->state = STATE_WAIT_FOR_READ;
            this->setBuffSize(0);
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first byte state
            c = ::read(this->serialPort, this->buff(), this->buffMaxSize());
            if (c < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                    this->state = STATE_OPENED;
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
                                                                StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                                StringLiteral(". ") + getLastErrorText());
                }
            }
            if (c > 0)
            {
                this->addBuffSize(static_cast<uint16_t>(c));
                if ((this->timeoutInterByte() == 0) || // timeoutInterByte = 0 means no need to wait next bytes
                    (this->buffSize() == this->buffMaxSize()))    // input buffer is full. Try to handle it
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                if (this->buffSize() > this->buffMaxSize())
                {
                    this->state = STATE_OPENED;
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->portName() +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
            }
            else if (timer() - this->timestamp >= this->timeout()) // waiting timeout to read first byte is elapsed
            {
                this->state = STATE_OPENED;
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
            // next bytes state
            c = ::read(this->serialPort, this->buffNext(), this->buffFreeSize());
            if (c < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                    this->state = STATE_OPENED;
                    return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
                                                                StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                                StringLiteral(". ") + getLastErrorText());
                }
            }

            if (c > 0)
            {
                this->addBuffSize(static_cast<uint16_t>(c));
                if (this->buffSize() == this->buffMaxSize()) // input buffer is full. Try to handle it
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                if (this->buffSize() > this->buffMaxSize())
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->portName() +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                this->timestampRefresh();
            }
            else if (timer() - this->timestamp >= this->timeoutInterByte()) // waiting timeout to read next byte is elapsed
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }
            return Status_Processing;
        default:
            if (this->serialPortIsOpen())
            {
                this->state = STATE_OPENED;
                fRepeatAgain = true;
            }
            else
                return this->setError(Status_BadSerialRead, StringLiteral("Internal state error"));
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

inline ModbusSerialPortPrivateUnix *d_unix(ModbusPortPrivate *this_ptr) { return static_cast<ModbusSerialPortPrivateUnix*>(this_ptr); }

#endif // MODBUSSERIALPORT_P_UNIX_H
