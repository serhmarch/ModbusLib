#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cModbus.h>

const char* help_options =
"Usage: mbserverc [options]\n"
"\n"
"Options:\n"
"  -help (-?)        - show this help.\n"
"  -unit (-u) <unit> - modbus device address/unit (default is 1)\n"
"  -type (-t) <type> - protocol type. Can be TCP, RTU or ASC (default is TCP)\n"
"  -port (-p) <port> - remote TCP port (502 is default)\n"
"  -tm <timeout>     - timeout for TCP (millisec, default is 3000)\n"
"  -serial (-sl)     - serial port name for RTU and ASC\n"
"  -baud (-b)        - baud rate (for RTU and ASC)\n"
"  -data (-d)        - data bits (5-8, for RTU and ASC)\n"
"  -parity           - parity: E (even), O (odd), N (none) (default is none)\n"
"  -stop (-s)        - stop bits: 1, 1.5, 2\n"
"  -tfb <timeout>    - timeout first byte for RTU or ASC (millisec, default is 3000)\n"
"  -tib <timeout>    - timeout inter byte for RTU or ASC (millisec, default is 5)\n"
"  -c0 <count>       - modbus 0x-data count (default is 256)\n"
"  -c1 <count>       - modbus 1x-data count (default is 256)\n"
"  -c3 <count>       - modbus 3x-data count (default is 256)\n"
"  -c4 <count>       - modbus 4x-data count (default is 256)\n"
"\n"
"Note: ReadExceptionStatus data is located in 0x (coils) memory starting with offset 0 (000001)\n"
;

typedef struct _Options
{
    ProtocolType        type;
    uint8_t             unit;
    SerialPortSettings  ser ;
    TcpSettings         tcp ;
    uint32_t            c0  ;
    uint32_t            c1  ;
    uint32_t            c3  ;
    uint32_t            c4  ;
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
    options.c0                   = 256;
    options.c1                   = 256;
    options.c3                   = 256;
    options.c4                   = 256;
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
        if (!strcmp(opt, "c0"))
        {
            if (++i < argc)
            {
                options.c0 = atoi(argv[i]);
                continue;
            }
            printf("'-c0' option (count of 0x-data) must have a value: 0-65536\n");
            exit(1);
        }
        if (!strcmp(opt, "c1"))
        {
            if (++i < argc)
            {
                options.c1 = atoi(argv[i]);
                continue;
            }
            printf("'-c1' option (count of 1x-data) must have a value: 0-65536\n");
            exit(1);
        }
        if (!strcmp(opt, "c3"))
        {
            if (++i < argc)
            {
                options.c3 = atoi(argv[i]);
                continue;
            }
            printf("'-c3' option (count of 3x-data) must have a value: 0-65536\n");
            exit(1);
        }
        if (!strcmp(opt, "c4"))
        {
            if (++i < argc)
            {
                options.c4 = atoi(argv[i]);
                continue;
            }
            printf("'-c4' option (count of 4x-data) must have a value: 0-65536\n");
            exit(1);
        }
        printf("Bad option: %s\n", opt);
        puts(help_options);
        exit(1);
    }
}

void printTx(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("'%s' Tx: %s\n", source, sbytes(buff, size, s, sizeof(s)));
}

void printRx(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("'%s' Rx: %s\n", source, sbytes(buff, size, s, sizeof(s)));
}

void printTxAsc(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("'%s' Tx: %s\n", source, sascii(buff, size, s, sizeof(s)));
}

void printRxAsc(const Char *source, const uint8_t* buff, uint16_t size)
{
    Char s[1000];
    printf("'%s' Rx: %s\n", source, sascii(buff, size, s, sizeof(s)));
}

void printNewConnection(const Char *source)
{
    printf("New connection: %s\n", source);
}

void printCloseConnection(const Char *source)
{
    printf("Close connection: %s\n", source);
}

typedef struct _MemoryDevice
{
    uint8_t   unit;

    uint8_t  *mem0x;
    uint32_t  bit0x;

    uint8_t  *mem1x;
    uint32_t  bit1x;

    uint16_t *mem3x;
    uint32_t  reg3x;

    uint8_t  *mem4x;
    uint32_t  reg4x;
} MemoryDevice;

void device_init(MemoryDevice *dev, uint8_t unit, uint32_t c0, uint32_t c1, uint32_t c3, uint32_t c4)
{
    uint32_t bytes0 = (c0+7)/8;
    uint32_t bytes1 = (c1+7)/8;
    uint32_t bytes3 = (c3*2);
    uint32_t bytes4 = (c4*2);

    dev->unit = unit;

    if (bytes0)
    {
        dev->mem0x = malloc(bytes0);
        memset(dev->mem0x, 0, bytes0);
    }
    else
        dev->mem0x = NULL;
    dev->bit0x = c0;

    if (bytes1)
    {
        dev->mem1x = malloc(bytes1);
        memset(dev->mem1x, 0, bytes1);
    }
    else
        dev->mem1x = NULL;
    dev->bit1x = c1;

    if (bytes3)
    {
        dev->mem3x = malloc(bytes3);
        memset(dev->mem3x, 0, bytes3);
    }
    else
        dev->mem3x = NULL;
    dev->reg3x = c3;

    if (bytes4)
    {
        dev->mem4x = malloc(bytes4);
        memset(dev->mem4x, 0, bytes4);
    }
    else
        dev->mem4x = NULL;
    dev->reg4x = c4;
}

void device_del(MemoryDevice *dev)
{
    if (dev->mem0x) free(dev->mem0x);
    if (dev->mem1x) free(dev->mem1x);
    if (dev->mem3x) free(dev->mem3x);
    if (dev->mem4x) free(dev->mem4x);
}

StatusCode device_checkUnit(MemoryDevice *dev, uint8_t unit)
{
    if (dev->unit != unit)
        return Status_BadGatewayPathUnavailable;
    return Status_Good;
}

StatusCode device_readCoils(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return readMemBits(offset, count, values, dev->mem0x, dev->bit0x);
}

StatusCode device_readDiscreteInputs(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return readMemBits(offset, count, values, dev->mem1x, dev->bit1x);
}

StatusCode device_readHoldingRegisters(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return readMemRegs(offset, count, values, dev->mem4x, dev->reg4x);
}

StatusCode device_readInputRegisters(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return readMemRegs(offset, count, values, dev->mem3x, dev->reg3x);
}

StatusCode device_writeSingleCoil(cModbusDevice d, uint8_t unit, uint16_t offset, bool value)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return writeMemBits(offset, 1, &value, dev->mem0x, dev->bit0x);
}

StatusCode device_writeSingleRegister(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t value)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return writeMemRegs(offset, 1, &value, dev->mem4x, dev->reg4x);
}

StatusCode device_readExceptionStatus(cModbusDevice d, uint8_t unit, uint8_t *excstatus)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    *excstatus = dev->mem0x[0];
    return Status_Good;
}

StatusCode device_writeMultipleCoils(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return writeMemBits(offset, count, values, dev->mem0x, dev->bit0x);
}

StatusCode device_writeMultipleRegisters(cModbusDevice d, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    MemoryDevice *dev = (MemoryDevice*)d;
    StatusCode status = device_checkUnit(dev, unit);
    if (StatusIsBad(status))
        return status;
    return writeMemRegs(offset, count, values, dev->mem4x, dev->reg4x);
}

int main(int argc, char **argv)
{
    initOptions();
    parseOptions(argc, argv);
    MemoryDevice dev;
    device_init(&dev, options.unit, options.c0, options.c1, options.c3, options.c4);

    pfReadCoils              fReadCoils              = NULL;
    pfReadDiscreteInputs     fReadDiscreteInputs     = NULL;
    pfReadHoldingRegisters   fReadHoldingRegisters   = NULL;
    pfReadInputRegisters     fReadInputRegisters     = NULL;
    pfWriteSingleCoil        fWriteSingleCoil        = NULL;
    pfWriteSingleRegister    fWriteSingleRegister    = NULL;
    pfReadExceptionStatus    fReadExceptionStatus    = NULL;
    pfWriteMultipleCoils     fWriteMultipleCoils     = NULL;
    pfWriteMultipleRegisters fWriteMultipleRegisters = NULL;

    if (dev.mem0x)
    {
        fReadCoils           = device_readCoils          ;
        fWriteSingleCoil     = device_writeSingleCoil    ;
        fReadExceptionStatus = device_readExceptionStatus;
        fWriteMultipleCoils  = device_writeMultipleCoils ;
    }

    if (dev.mem1x)
    {
        fReadDiscreteInputs  = device_readDiscreteInputs;
    }

    if (dev.mem3x)
    {
        fReadInputRegisters  = device_readInputRegisters;
    }

    if (dev.mem4x)
    {
        fReadHoldingRegisters   = device_readHoldingRegisters  ;
        fWriteSingleRegister    = device_writeSingleRegister   ;
        fWriteMultipleRegisters = device_writeMultipleRegisters;
    }

    cModbusInterface cdev = cCreateModbusDevice(&dev,
                                                fReadCoils             ,
                                                fReadDiscreteInputs    ,
                                                fReadHoldingRegisters  ,
                                                fReadInputRegisters    ,
                                                fWriteSingleCoil       ,
                                                fWriteSingleRegister   ,
                                                fReadExceptionStatus   ,
                                                fWriteMultipleCoils    ,
                                                fWriteMultipleRegisters
                                                );

    const bool blocking = false; // use async/non blocking mode
    cModbusServerPort serv;
    switch (options.type)
    {
    case RTU:
        serv = cSpoCreate(cdev, options.type, &options.ser, blocking);
        cSpoConnectTx(serv, printTx);
        cSpoConnectRx(serv, printRx);
        break;
    case ASC:
        serv = cSpoCreate(cdev, options.type, &options.ser, blocking);
        cSpoConnectTx(serv, printTxAsc);
        cSpoConnectRx(serv, printRxAsc);
        break;
    default:
        serv = cSpoCreate(cdev, TCP, &options.tcp, blocking);
        cSpoConnectTx(serv, printTx);
        cSpoConnectRx(serv, printRx);
        cSpoConnectNewConnection(serv, printNewConnection);
        cSpoConnectCloseConnection(serv, printCloseConnection);
        break;
    }

    puts("Server starts ...\n");
    while (1)
    {
        cSpoProcess(serv);
        msleep(1);
    }
    cSpoDelete(serv);
    cDeleteModbusDevice(cdev);
    device_del(&dev);
    return 0;
}
