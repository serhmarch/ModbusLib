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
    ModbusSerialPortPrivateUnix(uint16_t maxBuffSize, bool blocking) :
        ModbusSerialPortPrivate(maxBuffSize, blocking)
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
    Modbus::Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

public:
    inline bool serialPortIsInvalid() const { return serialPort == -1; }
    inline bool serialPortIsValid() const { return serialPort != -1; }
    inline bool serialPortIsOpen() const { return serialPortIsValid(); }
    inline void serialPortClose() { ::close(serialPort); serialPort = -1; }
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

Handle ModbusSerialPortPrivateUnix::handle() const
{
    return reinterpret_cast<Handle>(this->serialPort);
}

StatusCode ModbusSerialPortPrivateUnix::open()
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
            if (isOpen())
            {
                if (this->isChanged())
                    this->close();
                else
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
            }
            this->clearChanged();
            struct termios options;
            speed_t sp;
            int flags = O_RDWR | O_NOCTTY;
            if (isBlocking())
            {
                flags |= O_SYNC;
            }
            else
            {
                flags |= O_NONBLOCK;
            }
            this->serialPort = ::open(this->portName().c_str(),  flags);

            if (this->serialPortIsInvalid())
            {
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to open '") + this->portName() +
                                                            StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                            StringLiteral(". ") + getLastErrorText());
            }

            //fcntl(this->serialPort, F_SETFL, 0); // Note: change file (serial port) flags

            // Configuring serial port
            int r;
            r = tcgetattr(this->serialPort, &options);
            if (r < 0)
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to get attributes for '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());
            switch(this->settings.baudRate)
            {
            case 1200:  sp = B1200;  break;
            case 2400:  sp = B2400;  break;
            case 4800:  sp = B4800;  break;
            case 9600:  sp = B9600;  break;
            case 19200: sp = B19200; break;
            case 38400: sp = B38400; break;
            case 57600: sp = B57600; break;
            case 115200:sp = B115200;break;
            default:    sp = B9600;  break;
            }

            r = cfsetispeed(&options, sp);
            if (r < 0)
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to set input baud rate for '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());
            r = cfsetospeed(&options, sp);
            if (r < 0)
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to set output baud rate for '") + this->portName() +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());

            options.c_cflag |= (CLOCAL | CREAD);

            // data bits
            options.c_cflag &= ~CSIZE;
            switch (this->settings.dataBits)
            {
            case 5: options.c_cflag |= CS5; break;
            case 6: options.c_cflag |= CS6; break;
            case 7: options.c_cflag |= CS7; break;
            case 8: options.c_cflag |= CS8; break;
            }

            // parity
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~PARODD;
            switch (this->settings.parity)
            {
            case EvenParity: options.c_cflag |= PARENB; break;
            case OddParity:  options.c_cflag |= PARENB; options.c_cflag |= PARODD; break;
            default: break; // TODO: Space, Mark Parities
            }

            // stop bits
            switch (this->settings.stopBits)
            {
            case OneStop:
                options.c_cflag &= ~CSTOPB;  // Clear CSTOPB flag for 1 stop bit
                break;
            case OneAndHalfStop:
                options.c_cflag |= CSTOPB;   // Set CSTOPB flag for 2 stop bits (1.5 stop bits not directly supported, use 2)
                break;
            case TwoStop:
                options.c_cflag |= CSTOPB;   // Set CSTOPB flag for 2 stop bits
                break;
            }

            // disable hardware flow control
            //options.c_cflag &= ~CNEW_RTSCTS;

            // setting Raw input mode
            options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

            // ignore parity errors (we are using CRC!)
            options.c_iflag &= ~INPCK;
            options.c_iflag |= IGNPAR;

            // disable software flow control
            //options.c_iflag &= ~(PARMRK | ISTRIP | IXON | IXOFF | IXANY | ICRNL | IGNBRK | BRKINT | INLCR | IGNCR | ICRNL | IUCLC | IMAXBEL);
            options.c_iflag = 0;

            // setting Raw output mode
            //options.c_oflag &= ~OPOST;
            options.c_oflag = 0;

            // setting serial read timeouts
            if (isBlocking())
            {
                options.c_cc[VMIN]  = 0;
                options.c_cc[VTIME] = this->timeoutFirstByte() / 100;
            }
            else
            {
                options.c_cc[VMIN]  = 0;
                options.c_cc[VTIME] = 0;
            }

            r = tcsetattr(this->serialPort, TCSANOW, &options);
            if (r < 0)
                return this->setError(Status_BadSerialOpen, StringLiteral("Failed to set attributes for '") + this->portName() +
                                                            StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                            StringLiteral(". ") + getLastErrorText());

        }
            return Status_Good;
        default:
            if (isOpen() && !this->isChanged())
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }
            this->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusSerialPortPrivateUnix::close()
{
    if (this->serialPortIsOpen())
    {
        this->serialPortClose();
        //setMessage(QString("Serial port '%1' is closed").arg(serialPortName()));
    }
    this->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusSerialPortPrivateUnix::isOpen() const
{
    return this->serialPortIsOpen();
}

StatusCode ModbusSerialPortPrivateUnix::write()
{
    return (this->*(this->writeMethod))();
}

StatusCode ModbusSerialPortPrivateUnix::read()
{
    return (this->*(this->readMethod))();
}

StatusCode ModbusSerialPortPrivateUnix::blockingWrite()
{
    int c;
    this->state = STATE_OPENED;
    tcflush(this->serialPort, TCIFLUSH);
    c = ::write(this->serialPort, this->buff, this->sz);
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
    c = ::read(this->serialPort, this->buff, this->c_buffSz);
    if (c < 0)
    {
        return this->setError(Status_BadSerialRead, StringLiteral("Error while reading '") + this->portName() +
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
        case STATE_OPENED:
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
                return this->setError(Status_BadSerialWrite, StringLiteral("Internal error"));
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
            this->sz = 0;
            // no need break
        case STATE_WAIT_FOR_READ:
            // read first byte state
            c = ::read(this->serialPort, this->buff, this->c_buffSz);
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
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->portName() +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                }
            }
            else if (timer() - this->timestamp >= this->timeoutFirstByte()) // waiting timeout read first byte elapsed
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
            c = ::read(this->serialPort, this->buff+this->sz, this->c_buffSz-this->sz);
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
                this->sz += static_cast<uint16_t>(c);
                if (this->sz == this->c_buffSz) // input buffer is full. Try to handle it
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                if (this->sz > this->c_buffSz)
                    return this->setError(Status_BadReadBufferOverflow, StringLiteral("Error while reading '") + this->portName() +
                                                                        StringLiteral("' serial port. Read buffer overflow"));
                this->timestampRefresh();
            }
            else if (timer() - this->timestamp >= this->settings.timeoutInterByte) // waiting timeout read next byte elapsed
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
                return this->setError(Status_BadSerialRead, StringLiteral("Internal error"));
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

inline ModbusSerialPortPrivateUnix *d_unix(ModbusPortPrivate *this_ptr) { return static_cast<ModbusSerialPortPrivateUnix*>(this_ptr); }

#endif // MODBUSSERIALPORT_P_UNIX_H
