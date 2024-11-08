#include "../ModbusSerialPort.h"
#include "ModbusSerialPort_p_unix.h"

#include <termios.h>

ModbusSerialPort::Defaults::Defaults() :
    portName(StringLiteral("/dev/ttyS0")),
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
    return new ModbusSerialPortPrivateUnix(blocking);
}

ModbusSerialPort::~ModbusSerialPort()
{
    ModbusSerialPortPrivateUnix *d = d_unix(d_ptr);
    if (d->serialPortIsOpen())
        d->serialPortClose();
}

Handle ModbusSerialPort::handle() const
{
    return reinterpret_cast<Handle>(d_unix(d_ptr)->serialPort);
}

StatusCode ModbusSerialPort::open()
{
    ModbusSerialPortPrivateUnix *d = d_unix(d_ptr);
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
            if (isOpen())
            {
                d->state = STATE_BEGIN;
                return Status_Good;
            }

            d->clearChanged();
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
            d->serialPort = ::open(d->settings.portName.c_str(),  flags);

            if (d->serialPortIsInvalid())
            {
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to open '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());
            }

            //fcntl(d->serialPort, F_SETFL, 0); // Note: change file (serial port) flags

            // Configuring serial port
            int r;
            r = tcgetattr(d->serialPort, &options);
            if (r < 0)
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to get attributes for '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());
            switch(d->settings.baudRate)
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
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set input baud rate for '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());
            r = cfsetospeed(&options, sp);
            if (r < 0)
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set output baud rate for '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());

            options.c_cflag |= (CLOCAL | CREAD);

            // data bits
            options.c_cflag &= ~CSIZE;
            switch (d->settings.dataBits)
            {
            case 5: options.c_cflag |= CS5; break;
            case 6: options.c_cflag |= CS6; break;
            case 7: options.c_cflag |= CS7; break;
            case 8: options.c_cflag |= CS8; break;
            }

            // parity
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~PARODD;
            switch (d->settings.parity)
            {
            case EvenParity: options.c_cflag |= PARENB; break;
            case OddParity:  options.c_cflag |= PARENB; options.c_cflag |= PARODD; break;
            default: break; // TODO: Space, Mark Parities
            }

            // stop bits
            switch (d->settings.stopBits)
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
                options.c_cc[VTIME] = d->settingsBase.timeout / 100;
            }
            else
            {
                options.c_cc[VMIN]  = 0;
                options.c_cc[VTIME] = 0;
            }

            r = tcsetattr(d->serialPort, TCSANOW, &options);
            if (r < 0)
                return d->setError(Status_BadSerialOpen, StringLiteral("Failed to set attributes for '") + d->settings.portName +
                                                         StringLiteral("' serial port. Error code: ") + toModbusString(errno) +
                                                         StringLiteral(". ") + getLastErrorText());

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
    ModbusSerialPortPrivateUnix *d = d_unix(d_ptr);
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
    return d_unix(d_ptr)->serialPortIsOpen();
}

StatusCode ModbusSerialPort::write()
{
    ModbusSerialPortPrivateUnix *d = d_unix(d_ptr);
    return (d->*(d->writeMethod))();
}

StatusCode ModbusSerialPort::read()
{
    ModbusSerialPortPrivateUnix *d = d_unix(d_ptr);
    return (d->*(d->readMethod))();
}
