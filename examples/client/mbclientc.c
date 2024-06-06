#include <stdio.h>
#include <stdlib.h>

#include <cModbus.h>

const char* help_options =
"Usage: mbclientc [options]\n"
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
"  -func (-f) <func>        - number of modbus function (default is 3, READ_HOLDING_REGISTERS)\n"
"  -offset (-o) <offset>    - modbus function data start offset (default is 0)\n"
"  -count (-c) <count>      - modbus function data count (default is 16)\n"
"  -value (-v) <value>      - modbus function data value (default is 0)\n"
"  -n <count>               - modbus request count: >0 - count, <0 (inf) - infinite (default is 4)\n"
"  -period <period>         - request period (millisec, default is 1000)\n"
;

typedef struct _Options
{
    ProtocolType        type  ;
    uint8_t             unit  ;
    SerialPortSettings  ser   ;
    TcpSettings         tcp   ; 
    int                 func  ;
    uint16_t            offset;
    uint16_t            count ;
    uint16_t            value ;
    int                 repeat;
    uint32_t            period;
} Options;

Options options;

void initOptions()
{
    options.unit                 = 1;
    options.type                 = TCP;
    options.tcp.host             = StringLiteral("localhost");
    options.tcp.port             = STANDARD_TCP_PORT;
    options.tcp.timeout          = 3000;
    options.ser.portName         = StringLiteral("\0");;
    options.ser.baudRate         = 9600;
    options.ser.dataBits         = 8;
    options.ser.parity           = NoParity;
    options.ser.stopBits         = OneStop;
    options.ser.flowControl      = NoFlowControl;
    options.ser.timeoutFirstByte = 3000;
    options.ser.timeoutInterByte = 5;
    options.func                 = MBF_READ_HOLDING_REGISTERS;
    options.offset               = 0;
    options.count                = 16;
    options.value                = 0;
    options.repeat               = 4;
    options.period               = 1000;
}

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
        if (!strcmp(opt, "type") || !strcmp(opt, "t"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "TCP"))
                {
                    options.type = TCP;
                    continue;
                }
                else if (!strcmp(sOptValue, "RTU"))
                {
                    options.type = RTU;
                    continue;
                }
                else if (!strcmp(sOptValue, "ASC"))
                {
                    options.type = ASC;
                    continue;
                }
            }
            printf("'-type' option must have a value: TCP, RTU or ASC\n");
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
            printf("'-port' option must have a value: 0-65535\n");
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
                    options.ser.parity = NoParity;
                    continue;
                }
                else if (!strcmp(sOptValue, "E") || !strcmp(sOptValue, "even"))
                {
                    options.ser.parity = EvenParity;
                    continue;
                }
                else if (!strcmp(sOptValue, "O") || !strcmp(sOptValue, "odd"))
                {
                    options.ser.parity = OddParity;
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
                    options.ser.stopBits = OneStop;
                    continue;
                }
                else if (!strcmp(sOptValue, "1.5"))
                {
                    options.ser.stopBits = OneAndHalfStop;
                    continue;
                }
                else if (!strcmp(sOptValue, "2"))
                {
                    options.ser.stopBits = TwoStop;
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
        if (!strcmp(opt, "func") || !strcmp(opt, "f"))
        {
            if (++i < argc)
            {
                options.func = atoi(argv[i]);
                continue;
            }
            printf("'-func' option must have a suppoted func number: 1-7,15,16\n");
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
        if (!strcmp(opt, "value") || !strcmp(opt, "v"))
        {
            if (++i < argc)
            {
                options.value = (uint16_t)atoi(argv[i]);
                continue;
            }
            printf("'-value' option must have a value: <integer>\n");
            exit(1);
        }
        if (!strcmp(opt, "n"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "inf"))
                {
                    options.repeat = -1;
                    continue;
                }
                options.repeat = (uint16_t)atoi(argv[i]);
                continue;
            }
            printf("'-n' option (request repeat) must have a value: >0 - count, <0 (inf) - infinite\n");
            exit(1);
        }
        if (!strcmp(opt, "period"))
        {
            if (++i < argc)
            {
                options.period = (uint32_t)atoi(argv[i]);
                continue;
            }
            printf("'-period' option must have a value: <integer> (millisec)\n");
            exit(1);
        }
        printf("Bad option: %s\n", opt);
        puts(help_options);
        exit(1);
    }
}

uint8_t values[MB_VALUE_BUFF_SZ];
uint16_t *regs = (uint16_t*)values;

void printBits(const void *buff, int count)
{
    const uint8_t *bytes = (const uint8_t*)buff;
    for (int i = 0; i < count; i++)
    {
        int n = i / 8;
        uint8_t mask = 1 << (i % 8);
        if (bytes[n] & mask)
            putchar('1');
        else
            putchar('0');
        putchar(' ');
    }
    putchar('\n');
}

void printRegs(const void *buff, int count)
{
    const uint16_t *regs = (const uint16_t*)buff;
    for (int i = 0; i < count; i++)
        printf("%hu ", regs[i]);
    putchar('\n');
}

void printResult(StatusCode status, const Char *errorText)
{
    if (StatusIsGood(status))
        puts("Result: Good");
    else
        printf("Error: %s\n", errorText);
}

void printTx(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("Tx: %s\n", sbytes(buff, size, s, sizeof(s)));
}

void printRx(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("Rx: %s\n", sbytes(buff, size, s, sizeof(s)));
}

void printTxAsc(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("Tx: %s\n", sascii(buff, size, s, sizeof(s)));
}

void printRxAsc(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("Rx: %s\n", sascii(buff, size, s, sizeof(s)));
}

void fillValues()
{
    switch (options.func)
    {
    case MBF_WRITE_SINGLE_COIL:
        values[0] = (options.value ? 0xFF : 0x00);
        break;
    case MBF_WRITE_SINGLE_REGISTER:
        regs[0] = options.value;
        break;
    case MBF_WRITE_MULTIPLE_COILS:
    {
        uint16_t v = (options.value ? 0xFFFF : 0x00);
        for (int i = 0; i < MB_VALUE_BUFF_SZ/2; i++)
            regs[i] = v;
    }
        break;
    case MBF_WRITE_MULTIPLE_REGISTERS:
        for (int i = 0; i < MB_VALUE_BUFF_SZ/2; i++)
            regs[i] = options.value;
        break;
    }
}

int main(int argc, char **argv)
{
    initOptions();
    parseOptions(argc, argv);

    cModbusClient client;
    switch (options.type)
    {
    case RTU:
    case ASC:
        client = cCliCreate(options.unit, options.type, &options.ser, true);
        break;
    default:
        client = cCliCreate(options.unit, options.type, &options.tcp, true);
        break;
    }

    cModbusClientPort cpo = cCliGetPort(client);
    ProtocolType cpoType = cCpoGetType(cpo);
    switch (cpoType)
    {
    case ASC:
        cCpoConnectTx(cpo, printTxAsc);
        cCpoConnectRx(cpo, printRxAsc);
        break;
    default:
        cCpoConnectTx(cpo, printTx);
        cCpoConnectRx(cpo, printRx);
        break;
    }

    int c = 0;
    StatusCode status;
    while (1)
    {
        Timer tmr = timer();
        switch (options.func)
        {
        case MBF_READ_COILS:
            printf("READ_COILS(offset=%hu,count=%hu)\n", options.offset, options.count);
            status = cReadCoils(client, options.offset, options.count, values);
            if (StatusIsGood(status))
                printBits(values, options.count);
            else
                printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_READ_DISCRETE_INPUTS:
            printf("READ_DISCRETE_INPUTS(offset=%hu,count=%hu)\n", options.offset, options.count);
            status = cReadDiscreteInputs(client, options.offset, options.count, values);
            if (StatusIsGood(status))
                printBits(values, options.count);
            else
                printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_READ_HOLDING_REGISTERS:
            printf("READ_HOLDING_REGISTERS(offset=%hu,count=%hu)\n", options.offset, options.count);
            status = cReadHoldingRegisters(client, options.offset, options.count, regs);
            if (StatusIsGood(status))
                printRegs(values, options.count);
            else
                printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_READ_INPUT_REGISTERS:
            printf("READ_INPUT_REGISTERS(offset=%hu,count=%hu)\n", options.offset, options.count);
            status = cReadInputRegisters(client, options.offset, options.count, regs);
            if (StatusIsGood(status))
                printRegs(values, options.count);
            else
                printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_WRITE_SINGLE_COIL:
            fillValues();
            printf("WRITE_SINGLE_COILS(offset=%hu)\n", options.offset);
            printBits(values, 1);
            status = cWriteSingleCoil(client, options.offset, values[0]);
            printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_WRITE_SINGLE_REGISTER:
            fillValues();
            printf("WRITE_SINGLE_REGISTE(offset=%hu)\n", options.offset);
            printRegs(values, 1);
            status = cWriteSingleRegister(client, options.offset, regs[0]);
            printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_READ_EXCEPTION_STATUS:
            printf("READ_EXCEPTION_STATUS\n");
            regs[0] = 0;
            status = cReadExceptionStatus(client, values);
            if (StatusIsGood(status))
                printRegs(values, 1);
            else
                printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_WRITE_MULTIPLE_COILS:
            fillValues();
            printf("WRITE_MULTIPLE_COILS(offset=%hu,count=%hu)\n", options.offset, options.count);
            printBits(values, options.count);
            status = cWriteMultipleCoils(client, options.offset, options.count, values);
            printResult(status, cCliGetLastPortErrorText(client));
            break;
        case MBF_WRITE_MULTIPLE_REGISTERS:
            fillValues();
            printf("WRITE_MULTIPLE_REGISTERS(offset=%hu,count=%hu)\n", options.offset, options.count);
            printRegs(values, options.count);
            status = cWriteMultipleRegisters(client, options.offset, options.count, regs);
            printResult(status, cCliGetLastPortErrorText(client));
            break;
        default:
            printf("Unknown function: %d", options.func);
            exit(1);
            break;
        }
        c++;
        if ((options.repeat < 0) || (c < options.repeat))
        {
            Timer tmexec = timer()-tmr;
            if (tmexec < options.period)
                msleep(options.period-tmexec);
            else
                msleep(1);
            continue;
        }
        break;
    }
    return 0;
}
