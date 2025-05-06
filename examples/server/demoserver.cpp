#include <iostream>

#include <vector>
#include <thread>

#include <ModbusTcpServer.h>
#include <ModbusServerResource.h>
#include <ModbusRtuPort.h>
#include <ModbusAscPort.h>

const char* help_options =
"Usage: demoserver [options]\n"
"\n"
"Options:\n"
"  -help (-?)          - show this help.\n"
"  -type (-t) <type>   - protocol type. Can be TCP, RTU or ASC (default is TCP)\n"
"  -port (-p) <port>   - remote TCP port (502 is default)\n"
"  -tm <timeout>       - timeout for TCP (millisec, default is 3000)\n"
"  -maxconn <count>    - max active TCP connections (default is 10)\n"
"  -serial (-sl)       - serial port name for RTU and ASC\n"
"  -baud (-b)          - baud rate (for RTU and ASC, default is 9600)\n"
"  -data (-d)          - data bits (5-8, for RTU and ASC, default is 8)\n"
"  -parity             - parity: E (even), O (odd), N (none) (default is none)\n"
"  -stop (-s)          - stop bits: 1, 1.5, 2 (default is 1)\n"
"  -tfb <timeout>      - timeout first byte for RTU or ASC (millisec, default is 3000)\n"
"  -tib <timeout>      - timeout inter byte for RTU or ASC (millisec, default is 5)\n"
"  -count (-c) <count> - memory size (count of 16-bit registers, default is 16)\n"
;

void printTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Tx: " << Modbus::bytesToString(buff, size) << '\n';
}

void printRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Rx: " << Modbus::bytesToString(buff, size) << '\n';
}

void printTxAsc(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Tx: " << Modbus::asciiToString(buff, size) << '\n';
}

void printRxAsc(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Rx: " << Modbus::asciiToString(buff, size) << '\n';
}

void printNewConnection(const Modbus::Char *source)
{
    std::cout << "New connection: " << source << '\n';
}

void printCloseConnection(const Modbus::Char *source)
{
    std::cout << "Close connection: " << source << '\n';
}

class Device : public ModbusInterface
{
private:
    uint8_t m_unit;
    std::vector<uint16_t> m_buff;

public:
    Device(uint8_t unit, uint16_t regs) { m_unit = unit; m_buff.resize(regs); }

public:
    inline uint32_t regCount() const { return static_cast<uint32_t>(m_buff.size()); }
    inline uint32_t bitCount() const { return static_cast<uint32_t>(m_buff.size() * MB_REGE_SZ_BITES); }
    inline void inc() { m_buff[0]++; }

    inline Modbus::StatusCode readRegs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        if (unit != m_unit) return Modbus::Status_BadGatewayPathUnavailable;
        return Modbus::readMemRegs(offset, count, values, m_buff.data(), regCount());
    }

    Modbus::StatusCode writeRegs(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
    {
        if (unit != m_unit) return Modbus::Status_BadGatewayPathUnavailable;
        return Modbus::writeMemRegs(offset, count, values, m_buff.data(), regCount());
    }

    Modbus::StatusCode readBits(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        if (unit != m_unit) return Modbus::Status_BadGatewayPathUnavailable;
        return Modbus::readMemBits(offset, count, values, m_buff.data(), bitCount());
    }

    Modbus::StatusCode writeBits(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
    {
        if (unit != m_unit) return Modbus::Status_BadGatewayPathUnavailable;
        return Modbus::writeMemBits(offset, count, values, m_buff.data(), bitCount());
    }

    Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        return readBits(unit, offset, count, values);
    }

    Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values)
    {
        return readBits(unit, offset, count, values);
    }

    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
    {
        return readRegs(unit, offset, count, values);
    }

    Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
    {
        return readRegs(unit, offset, count, values);
    }

    Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value)
    {
        return writeBits(unit, offset, 1, &value);
    }

    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value)
    {
        return writeRegs(unit, offset, 1, &value);
    }

    Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *status)
    {
        uint16_t v = 0;
        Modbus::StatusCode s = readRegs(unit, 0, 1, &v);
        if (Modbus::StatusIsGood(s))
            *status = static_cast<uint8_t>(v);
        return s;
    }

    Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values)
    {
        return writeBits(unit, offset, count, values);
    }

    Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
    {
        return writeRegs(unit, offset, count, values);
    }

    Modbus::StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
    {
        uint16_t c, r;
        Modbus::StatusCode s = readRegs(unit, offset, 1, &c);
        if (Modbus::StatusIsBad(s))
            return s;
        r = (c & andMask) | (orMask & ~andMask);
        return writeRegs(unit, offset, 1, &r);;
    }

    Modbus::StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
    {
        Modbus::StatusCode status = writeRegs(unit, writeOffset, writeCount, writeValues);
        if (StatusIsBad(status))
            return status;
        return readRegs(unit, readOffset, readCount, readValues);
    }
};

struct Options
{
    Modbus::ProtocolType        type  ;
    uint8_t                     unit  ;
    Modbus::SerialSettings      ser   ;
    Modbus::TcpSettings         tcp   ; 
    uint16_t                    count ;

    Options()
    {
        const ModbusTcpServer ::Defaults &dTcp = ModbusTcpServer ::Defaults::instance();
        const ModbusSerialPort::Defaults &dSer = ModbusSerialPort::Defaults::instance();

        type                 = Modbus::TCP               ;
        unit                 = 1                         ;
        tcp.port             = dTcp.port                 ;
        tcp.timeout          = dTcp.timeout              ;
        tcp.maxconn          = dTcp.maxconn              ;
        ser.portName         = dSer.portName             ;
        ser.baudRate         = dSer.baudRate             ;
        ser.dataBits         = dSer.dataBits             ;
        ser.parity           = dSer.parity               ;
        ser.stopBits         = dSer.stopBits             ;
        ser.flowControl      = dSer.flowControl          ;
        ser.timeoutFirstByte = dSer.timeoutFirstByte     ;
        ser.timeoutInterByte = dSer.timeoutInterByte     ;
        count                = 16                        ;
    }
};
Options options;

void parseOptions(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        char *opt = argv[i];
        if (opt[0] != '-')
        {
            printf("Bad option: %s. Option must have '-' (dash) before its name\n", opt);
            puts(help_options);
            exit(1);
        }
        opt++; // pass '-' (dash)
        if (!strcmp(opt, "help") || !strcmp(opt, "?"))
        {
            puts(help_options);
            exit(0);
        }
        if (!strcmp(opt, "type") || !strcmp(opt, "t"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "TCP"))
                {
                    options.type = Modbus::TCP;
                    continue;
                }
                else if (!strcmp(sOptValue, "RTU"))
                {
                    options.type = Modbus::RTU;
                    continue;
                }
                else if (!strcmp(sOptValue, "ASC"))
                {
                    options.type = Modbus::ASC;
                    continue;
                }
            }
            printf("'-type' option must have a value: TCP, RTU or ASC\n");
            exit(1);
        }
        if (!strcmp(opt, "unit") || !strcmp(opt, "u"))
        {
            if (++i < argc)
            {
                options.unit = (uint8_t)atoi(argv[i]);
                continue;
            }
            printf("'-unit' option must have a value: 0-255\n");
            exit(1);
        }
        if (!strcmp(opt, "host") || !strcmp(opt, "h"))
        {
            if (++i < argc)
            {
                options.tcp.host = argv[i];
                continue;
            }
            printf("'-host' option must have a value\n");
            exit(1);
        }
        if (!strcmp(opt, "port") || !strcmp(opt, "p"))
        {
            if (++i < argc)
            {
                options.tcp.port = (uint16_t)atoi(argv[i]);
                continue;
            }
            printf("'-port' option must have a value: 0-65535\n");
            exit(1);
        }
        if (!strcmp(opt, "tm"))
        {
            if (++i < argc)
            {
                options.tcp.timeout = (uint32_t)atoi(argv[i]);
                continue;
            }
            printf("'-tm' option must have an integer value\n");
            exit(1);
        }
        if (!strcmp(opt, "maxconn"))
        {
            if (++i < argc)
            {
                options.tcp.maxconn = (uint32_t)atoi(argv[i]);
                continue;
            }
            printf("'-maxconn' option must have an integer value\n");
            exit(1);
        }
        if (!strcmp(opt, "serial") || !strcmp(opt, "sl"))
        {
            if (++i < argc)
            {
                options.ser.portName = argv[i];
                continue;
            }
            printf("'-serial' option must have a value: serial port name like 'COM1' (Windows) or /dev/ttyS0 (Unix) \n");
            exit(1);
        }
        if (!strcmp(opt, "baud") || !strcmp(opt, "b"))
        {
            if (++i < argc)
            {
                options.ser.baudRate = (int)atoi(argv[i]);
                continue;
            }
            printf("'-baud' option must have a value: 1200, 2400, 4800, 9600, 19200, 115200 etc\n");
            exit(1);
        }
        if (!strcmp(opt, "data") || !strcmp(opt, "d"))
        {
            if (++i < argc)
            {
                options.ser.dataBits = (int8_t)atoi(argv[i]);
                continue;
            }
            printf("'-data' option must have a value: 5-8\n");
            exit(1);
        }
        if (!strcmp(opt, "parity"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "N") || !strcmp(sOptValue, "no"))
                {
                    options.ser.parity = Modbus::NoParity;
                    continue;
                }
                else if (!strcmp(sOptValue, "E") || !strcmp(sOptValue, "even"))
                {
                    options.ser.parity = Modbus::EvenParity;
                    continue;
                }
                else if (!strcmp(sOptValue, "O") || !strcmp(sOptValue, "odd"))
                {
                    options.ser.parity = Modbus::OddParity;
                    continue;
                }
            }
            printf("'-parity' option must have a value: E (even), O (odd), N (none)\n");
            exit(1);
        }
        if (!strcmp(opt, "stop") || !strcmp(opt, "s"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "1"))
                {
                    options.ser.stopBits = Modbus::OneStop;
                    continue;
                }
                else if (!strcmp(sOptValue, "1.5"))
                {
                    options.ser.stopBits = Modbus::OneAndHalfStop;
                    continue;
                }
                else if (!strcmp(sOptValue, "2"))
                {
                    options.ser.stopBits = Modbus::TwoStop;
                    continue;
                }
            }
            printf("'-stop' option must have a value: 1, 1.5 or 2\n");
            exit(1);
        }
        if (!strcmp(opt, "tfb"))
        {
            if (++i < argc)
            {
                options.ser.timeoutFirstByte = (uint32_t)atoi(argv[i]);
                continue;
            }
            printf("'-tfb' option (timeout first byte) must have a value: <integer>\n");
            exit(1);
        }
        if (!strcmp(opt, "tib"))
        {
            if (++i < argc)
            {
                options.ser.timeoutInterByte = (uint32_t)atoi(argv[i]);
                continue;
            }
            printf("'-tfb' option (timeout inter byte) must have a value: <integer>\n");
            exit(1);
        }
        if (!strcmp(opt, "count") || !strcmp(opt, "c"))
        {
            if (++i < argc)
            {
                options.count = (uint16_t)atoi(argv[i]);
                if (options.count == 0)
                    options.count = 1;
                continue;
            }
            printf("'-count' option must have a value: <integer>\n");
            exit(1);
        }
        printf("Bad option: %s\n", opt);
        puts(help_options);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    parseOptions(argc, argv);
    const bool blocking = false;
    Device dev(options.unit, options.count);
    ModbusServerPort *serv;

    switch (options.type)
    {
    case Modbus::RTU:
        serv = Modbus::createServerPort(&dev, Modbus::RTU, &options.ser, blocking);
        serv->connect(&ModbusServerPort::signalTx, printTx);
        serv->connect(&ModbusServerPort::signalRx, printRx);
        break;
    case Modbus::ASC:
        serv = Modbus::createServerPort(&dev, Modbus::ASC, &options.ser, blocking);
        serv->connect(&ModbusServerPort::signalTx, printTxAsc);
        serv->connect(&ModbusServerPort::signalRx, printRxAsc);
        break;
    default:
        serv = Modbus::createServerPort(&dev, Modbus::TCP, &options.tcp, blocking);
        serv->connect(&ModbusServerPort::signalTx, printTx);
        serv->connect(&ModbusServerPort::signalRx, printRx);
        serv->connect(&ModbusTcpServer::signalNewConnection, printNewConnection);
        serv->connect(&ModbusTcpServer::signalCloseConnection, printCloseConnection);
        break;
    }

    puts("demoserver starts ...");
    Modbus::Timer tmr = Modbus::timer();
    while (1)
    {
        serv->process();
        Modbus::Timer tmrend = Modbus::timer();
        const uint32_t period = 1000;
        if ((tmrend-tmr) >= period)
        {
            dev.inc();
            tmr = tmrend;
        }
        Modbus::msleep(1);
    }
}
