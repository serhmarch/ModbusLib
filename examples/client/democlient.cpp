#include <iostream>

#include <vector>

#include <ModbusClientPort.h>
#include <ModbusClient.h>
#include <ModbusTcpPort.h>
#include <ModbusSerialPort.h>

const char* help_options =
"Usage: democlient [options]\n"
"\n"
"Options:\n"
"  -help (-?)               - show this help.\n"
"  -unit (-u) <unit>        - modbus device remote address/unit (default is 1)\n"
"  -type (-t) <type>        - protocol type. Can be TCP, RTU or ASC (default is TCP)\n"
"  -host (-h) <host>        - dns name or ip address for TCP (default is localhost)\n"
"  -port (-p) <port>        - remote TCP port (502 is default)\n"
"  -tm <timeout>            - timeout for TCP (millisec, default is 3000)\n"
"  -serial (-sl)            - serial port name for RTU and ASC\n"
"  -baud (-b)               - baud rate (for RTU and ASC)\n"
"  -data (-d)               - data bits (5-8, for RTU and ASC)\n"
"  -parity                  - parity: E (even), O (odd), N (none) (default is none)\n"
"  -stop (-s)               - stop bits: 1, 1.5, 2\n"
"  -tfb <timeout>           - timeout first byte for RTU or ASC (millisec, default is 3000)\n"
"  -tib <timeout>           - timeout inter byte for RTU or ASC (millisec, default is 5)\n"
"  -offset (-o) <offset>    - modbus function data start offset (default is 0)\n"
"  -count (-c) <count>      - modbus function data count (default is 16)\n"
;

void printRegs(int count, const void *buff)
{
    for (int i = 0; i < count; i++)
        std::cout << reinterpret_cast<const uint16_t*>(buff)[i] << ' ';
    std::cout << '\n';
}

void printBools(int count, const void *buff)
{
    for (int i = 0; i < count; i++)
        std::cout << reinterpret_cast<const bool*>(buff)[i] << ' ';
    std::cout << '\n';
}

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

struct Options
{
    Modbus::ProtocolType        type  ;
    uint8_t                     unit  ;
    Modbus::SerialPortSettings  ser   ;
    Modbus::TcpSettings         tcp   ; 
    uint16_t                    offset;
    uint16_t                    count ;

    Options()
    {
        const ModbusTcpPort   ::Defaults &dTcp = ModbusTcpPort   ::Defaults::instance();
        const ModbusSerialPort::Defaults &dSer = ModbusSerialPort::Defaults::instance();

        type                 = Modbus::TCP               ;
        unit                 = 1                         ;
        tcp.host             = StringLiteral("localhost");
        tcp.port             = dTcp.port                 ;
        tcp.timeout          = dTcp.timeout              ;
        ser.portName         = dSer.portName             ;
        ser.baudRate         = dSer.baudRate             ;
        ser.dataBits         = dSer.dataBits             ;
        ser.parity           = dSer.parity               ;
        ser.stopBits         = dSer.stopBits             ;
        ser.flowControl      = dSer.flowControl          ;
        ser.timeoutFirstByte = dSer.timeoutFirstByte     ;
        ser.timeoutInterByte = dSer.timeoutInterByte     ;
        offset               = 0                         ;
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
        if (!strcmp(opt, "offset") || !strcmp(opt, "o"))
        {
            if (++i < argc)
            {
                options.offset = (uint16_t)atoi(argv[i]);
                continue;
            }
            printf("'-offset' option must have a value: <integer>\n");
            exit(1);
        }
        if (!strcmp(opt, "count") || !strcmp(opt, "c"))
        {
            if (++i < argc)
            {
                options.count = (uint16_t)atoi(argv[i]);
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

    const bool blocking = true;
    ModbusClientPort *clientPort;

    switch (options.type)
    {
    case Modbus::RTU:
        clientPort = Modbus::createClientPort(Modbus::RTU, &options.ser, blocking);
        clientPort->connect(&ModbusClientPort::signalTx, printTx);
        clientPort->connect(&ModbusClientPort::signalRx, printRx);
        break;
    case Modbus::ASC:
        clientPort = Modbus::createClientPort(Modbus::ASC, &options.ser, blocking);
        clientPort->connect(&ModbusClientPort::signalTx, printTxAsc);
        clientPort->connect(&ModbusClientPort::signalRx, printRxAsc);
        break;
    default:
        clientPort = Modbus::createClientPort(Modbus::TCP, &options.tcp, blocking);
        clientPort->connect(&ModbusClientPort::signalTx, printTx);
        clientPort->connect(&ModbusClientPort::signalRx, printRx);
        break;
    }

    struct RequestParams { uint8_t func; uint16_t offset; uint16_t count; };

    std::list<RequestParams> requests {
                                        { MBF_READ_COILS              , options.offset, options.count},
                                        { MBF_READ_DISCRETE_INPUTS    , options.offset, options.count},
                                        { MBF_READ_HOLDING_REGISTERS  , options.offset, options.count},
                                        { MBF_READ_INPUT_REGISTERS    , options.offset, options.count},
                                        { MBF_WRITE_SINGLE_COIL       , options.offset, 0 },
                                        { MBF_WRITE_SINGLE_REGISTER   , options.offset, 0 },
                                        { MBF_READ_EXCEPTION_STATUS   , options.offset, 0 },
                                        { MBF_WRITE_MULTIPLE_COILS    , options.offset, options.count},
                                        { MBF_WRITE_MULTIPLE_REGISTERS, options.offset, options.count},
                                      };
    std::vector<uint16_t> buff;
    ModbusClient client(options.unit, clientPort);
    client.setObjectName(StringLiteral("Client"));

    for (const RequestParams &req : requests)
    {
        Modbus::Timer tmr = Modbus::timer();
        Modbus::StatusCode status = Modbus::Status_Processing;
        if (buff.size() < req.count)
            buff.resize(req.count);

        switch (req.func)
        {
        case MBF_READ_COILS:
            printf("READ_COILS(offset=%hu,count=%hu)\n", req.offset, req.count);
            status = client.readCoilsAsBoolArray(req.offset, req.count, reinterpret_cast<bool*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printBools(req.count, buff.data());
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_READ_DISCRETE_INPUTS:
            printf("READ_DISCRETE_INPUTS(offset=%hu,count=%hu)\n", req.offset, req.count);
            status = client.readDiscreteInputsAsBoolArray(req.offset, req.count, reinterpret_cast<bool*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printBools(req.count, buff.data());
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_READ_HOLDING_REGISTERS:
            printf("READ_HOLDING_REGISTERS(offset=%hu,count=%hu)\n", req.offset, req.count);
            status = client.readHoldingRegisters(req.offset, req.count, reinterpret_cast<uint16_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printRegs(req.count, buff.data());
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_READ_INPUT_REGISTERS:
            printf("READ_INPUT_REGISTERS(offset=%hu,count=%hu)\n", req.offset, req.count);
            status = client.readInputRegisters(req.offset, req.count, reinterpret_cast<uint16_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printRegs(req.count, buff.data());
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_WRITE_SINGLE_COIL:
            printf("WRITE_SINGLE_COILS(offset=%hu)\n", req.offset);
            printBools(1, buff.data());
            status = client.writeSingleCoil(req.offset, buff[0]);
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_WRITE_SINGLE_REGISTER:
            printf("WRITE_SINGLE_REGISTE(offset=%hu)\n", req.offset);
            printRegs(1, buff.data());
            status = client.writeSingleRegister(req.offset, buff[0]);
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_READ_EXCEPTION_STATUS:
            printf("READ_EXCEPTION_STATUS\n");
            buff[0] = 0;
            status = client.readExceptionStatus(reinterpret_cast<uint8_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printRegs(1, buff.data());
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_WRITE_MULTIPLE_COILS:
            printf("WRITE_MULTIPLE_COILS(offset=%hu,count=%hu)\n", req.offset, req.count);
            printBools(req.count, buff.data());
            status = client.writeMultipleCoilsAsBoolArray(req.offset, req.count, reinterpret_cast<bool*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        case MBF_WRITE_MULTIPLE_REGISTERS:
            printf("WRITE_MULTIPLE_REGISTERS(offset=%hu,count=%hu)\n", req.offset, req.count);
            printRegs(req.count, buff.data());
            status = client.writeMultipleRegisters(req.offset, req.count, reinterpret_cast<uint16_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort->lastErrorText() << "\n";
            break;
        }
        Modbus::Timer tmexec = Modbus::timer()-tmr;
        const uint32_t period = 1000;
        if (tmexec < period)
            Modbus::msleep(period-tmexec);
        else
            Modbus::msleep(1);
    }
    delete clientPort;
}
