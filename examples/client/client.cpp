#include <iostream>

#include <vector>
#include <thread>

#include <ModbusClient.h>
#include <ModbusClientPort.h>
#include <ModbusTcpPort.h>
#include <ModbusRtuPort.h>
#include <ModbusAscPort.h>

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

void printTx(const uint8_t* buff, uint16_t size)
{
    std::cout << "Tx: " << Modbus::bytesToString(buff, size) << '\n';
}

void printRx(const uint8_t* buff, uint16_t size)
{
    std::cout << "Rx: " << Modbus::bytesToString(buff, size) << '\n';
}

void printTxAsc(const uint8_t* buff, uint16_t size)
{
    std::cout << "Tx: " << Modbus::asciiToString(buff, size) << '\n';
}

void printRxAsc(const uint8_t* buff, uint16_t size)
{
    std::cout << "Rx: " << Modbus::asciiToString(buff, size) << '\n';
}

struct Options
{
    Modbus::Type        type            ;
    Modbus::String      host            ;
    uint16_t            port            ;
    uint16_t            timeout         ;
    Modbus::String      portName        ;
    int32_t             baudRate        ;
    int8_t              dataBits        ;
    Modbus::Parity      parity          ;
    Modbus::StopBits    stopBits        ;
    Modbus::FlowControl flowControl     ;
    uint32_t            timeoutFirstByte;
    uint32_t            timeoutInterByte;
    uint8_t             unit            ;
    uint16_t            offset          ;
    uint16_t            count           ;

    Options()
    {
        const ModbusTcpPort   ::Defaults &dTCP = ModbusTcpPort   ::Defaults::instance();
        const ModbusSerialPort::Defaults &dSer = ModbusSerialPort::Defaults::instance();

        type            = Modbus::TCP;
        host            = dTCP.host             ;
        port            = dTCP.port             ;
        timeout         = dTCP.timeout          ;
        portName        = dSer.portName         ;
        baudRate        = dSer.baudRate         ;
        dataBits        = dSer.dataBits         ;
        parity          = dSer.parity           ;
        stopBits        = dSer.stopBits         ;
        flowControl     = dSer.flowControl      ;
        timeoutFirstByte= dSer.timeoutFirstByte ;
        timeoutInterByte= dSer.timeoutInterByte ;
        unit            = 1                     ;
        offset          = 0                     ;
        count           = 16                    ;
    }
};
Options options;

void parseOptions(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        char *opt = argv[i];
        // clear dashes
        while (*opt == '-')
            opt++;
        if (!strcmp(opt, "type"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "TCP"))
                    options.type = Modbus::TCP;
                else if (!strcmp(sOptValue, "RTU"))
                    options.type = Modbus::RTU;
                else if (!strcmp(sOptValue, "ASC"))
                    options.type = Modbus::ASC;
                else
                    std::cerr << "Unknown Modbus::Type: " << sOptValue << '\n';
            }
            continue;
        }
        if (!strcmp(opt, "host"))
        {
            if (++i < argc)
                options.host = argv[i];
            continue;
        }
        if (!strcmp(opt, "port"))
        {
            if (++i < argc)
                options.port = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "timeout"))
        {
            if (++i < argc)
                options.timeout = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "portname"))
        {
            if (++i < argc)
                options.portName = argv[i];
            continue;
        }
        if (!strcmp(opt, "baudrate"))
        {
            if (++i < argc)
                options.baudRate = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "databits"))
        {
            if (++i < argc)
                options.dataBits = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "parity"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "N"))
                    options.parity = Modbus::NoParity;
                else if (!strcmp(sOptValue, "E"))
                    options.parity = Modbus::EvenParity;
                else if (!strcmp(sOptValue, "O"))
                    options.parity = Modbus::OddParity;
                else
                    std::cerr << "Unknown Modbus::Parity: " << sOptValue << '\n';
            }
            continue;
        }
        if (!strcmp(opt, "stopbits"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "1"))
                    options.stopBits = Modbus::OneStop;
                else if (!strcmp(sOptValue, "1.5"))
                    options.stopBits = Modbus::OneAndHalfStop;
                else if (!strcmp(sOptValue, "2"))
                    options.stopBits = Modbus::TwoStop;
                else
                    std::cerr << "Unknown Modbus::StopBits: " << sOptValue << '\n';
            }
            continue;
        }
        if (!strcmp(opt, "flowcontrol"))
        {
            if (++i < argc)
            {
                char *sOptValue = argv[i];
                if (!strcmp(sOptValue, "N"))
                    options.flowControl = Modbus::NoFlowControl;
                else if (!strcmp(sOptValue, "H"))
                    options.flowControl = Modbus::HardwareControl;
                else if (!strcmp(sOptValue, "S"))
                    options.flowControl = Modbus::SoftwareControl;
                else
                    std::cerr << "Unknown Modbus::FlowControl: " << sOptValue << '\n';
            }
            continue;
        }
        if (!strcmp(opt, "timeoutfb"))
        {
            if (++i < argc)
                options.timeoutInterByte = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "timeoutib"))
        {
            if (++i < argc)
                options.timeoutFirstByte = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "unit"))
        {
            if (++i < argc)
                options.unit = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "offset"))
        {
            if (++i < argc)
                options.offset = atoi(argv[i]);
            continue;
        }
        if (!strcmp(opt, "count"))
        {
            if (++i < argc)
                options.count = atoi(argv[i]);
            continue;
        }
        std::cerr << "Unknown option: " << opt << '\n';
    }
}

int main(int argc, char **argv)
{
    parseOptions(argc, argv);

    bool synch = false;
    ModbusPort *port;
    switch (options.type)
    {
    case Modbus::RTU:
    {
        ModbusRtuPort *rtu = new ModbusRtuPort(synch);
        rtu->setPortName        (options.portName        .data());
        rtu->setBaudRate        (options.baudRate        );
        rtu->setDataBits        (options.dataBits        );
        rtu->setParity          (options.parity          );
        rtu->setStopBits        (options.stopBits        );
        rtu->setFlowControl     (options.flowControl     );
        rtu->setTimeoutFirstByte(options.timeoutFirstByte);
        rtu->setTimeoutInterByte(options.timeoutInterByte);
        port = rtu;
    }
        break;
    case Modbus::ASC:
    {
        ModbusAscPort *asc = new ModbusAscPort(synch);
        asc->setPortName        (options.portName        .data());
        asc->setBaudRate        (options.baudRate        );
        asc->setDataBits        (options.dataBits        );
        asc->setParity          (options.parity          );
        asc->setStopBits        (options.stopBits        );
        asc->setFlowControl     (options.flowControl     );
        asc->setTimeoutFirstByte(options.timeoutFirstByte);
        asc->setTimeoutInterByte(options.timeoutInterByte);
        port = asc;
    }
        break;
    case Modbus::TCP:
    {
        ModbusTcpPort *tcp = new ModbusTcpPort(synch);
        tcp->setHost   (options.host       .data());
        tcp->setPort   (options.port       );
        tcp->setTimeout(options.timeout    );
        port = tcp;
    }
        break;
    }
    ModbusClientPort clientPort(port);
    ModbusClient client(options.unit, &clientPort);

    if (port->type() == Modbus::ASC)
    {
        port->connect(&ModbusPort::emitTx, printTxAsc);
        port->connect(&ModbusPort::emitRx, printRxAsc);
    }
    else
    {
        port->connect(&ModbusPort::emitTx, printTx);
        port->connect(&ModbusPort::emitRx, printRx);
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

    for (const RequestParams &req : requests)
    {
        Modbus::StatusCode status = Modbus::Status_Processing;
        if (buff.size() < req.count)
            buff.resize(req.count);

        switch (req.func)
        {
        case MBF_READ_COILS:
            printf("READ_COILS(offset=%hu,count=%hu)\n", req.offset, req.count);
            while (Modbus::StatusIsProcessing(status))
                status = client.readCoilsAsBoolArray(req.offset, req.count, reinterpret_cast<bool*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printBools(req.count, buff.data());
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_READ_DISCRETE_INPUTS:
            printf("READ_DISCRETE_INPUTS(offset=%hu,count=%hu)\n", req.offset, req.count);
            while (Modbus::StatusIsProcessing(status))
                status = client.readDiscreteInputsAsBoolArray(req.offset, req.count, reinterpret_cast<bool*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printBools(req.count, buff.data());
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_READ_HOLDING_REGISTERS:
            printf("READ_HOLDING_REGISTERS(offset=%hu,count=%hu)\n", req.offset, req.count);
            while (Modbus::StatusIsProcessing(status))
                status = client.readHoldingRegisters(req.offset, req.count, reinterpret_cast<uint16_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printRegs(req.count, buff.data());
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_READ_INPUT_REGISTERS:
            printf("READ_INPUT_REGISTERS(offset=%hu,count=%hu)\n", req.offset, req.count);
            while (Modbus::StatusIsProcessing(status))
                status = client.readInputRegisters(req.offset, req.count, reinterpret_cast<uint16_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printRegs(req.count, buff.data());
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_WRITE_SINGLE_COIL:
            printf("WRITE_SINGLE_COILS(offset=%hu)\n", req.offset);
            printBools(1, buff.data());
            while (Modbus::StatusIsProcessing(status))
                status = client.writeSingleCoil(req.offset, buff[0]);
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_WRITE_SINGLE_REGISTER:
            printf("WRITE_SINGLE_REGISTE(offset=%hu)\n", req.offset);
            printRegs(1, buff.data());
            while (Modbus::StatusIsProcessing(status))
                status = client.writeSingleRegister(req.offset, buff[0]);
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_READ_EXCEPTION_STATUS:
            printf("READ_EXCEPTION_STATUS\n");
            buff[0] = 0;
            while (Modbus::StatusIsProcessing(status))
                status = client.readExceptionStatus(reinterpret_cast<uint8_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                printRegs(1, buff.data());
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_WRITE_MULTIPLE_COILS:
            printf("WRITE_MULTIPLE_COILS(offset=%hu,count=%hu)\n", req.offset, req.count);
            printBools(req.count, buff.data());
            while (Modbus::StatusIsProcessing(status))
                status = client.writeMultipleCoilsAsBoolArray(req.offset, req.count, reinterpret_cast<bool*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        case MBF_WRITE_MULTIPLE_REGISTERS:
            printf("WRITE_MULTIPLE_REGISTERS(offset=%hu,count=%hu)\n", req.offset, req.count);
            printRegs(req.count, buff.data());
            while (Modbus::StatusIsProcessing(status))
                status = client.writeMultipleRegisters(req.offset, req.count, reinterpret_cast<uint16_t*>(buff.data()));
            if (Modbus::StatusIsGood(status))
                std::cout << "Good\n";
            else
                std::cout << clientPort.lastErrorText() << "\n";
            break;
        }
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
    }
}
