#include <iostream>

#include <vector>
#include <thread>

#include <ModbusTcpServer.h>
#include <ModbusServerResource.h>
#include <ModbusRtuPort.h>
#include <ModbusAscPort.h>

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

class Device : public ModbusInterface
{
public:
    Device(uint16_t regs) { m_buff.resize(regs); }

public:
    inline uint32_t regCount() const { return m_buff.size(); }
    inline uint32_t bitCount() const { return m_buff.size() * MB_REGE_SZ_BITES; }

    Modbus::StatusCode readRegs(uint16_t offset, uint16_t count, void *values)
    {
        if (static_cast<uint32_t>(offset + count) > regCount())
            return Modbus::Status_BadIllegalDataAddress;
        const uint16_t *mem = reinterpret_cast<const uint16_t*>(m_buff.data());
        memcpy(values, &mem[offset], count * MB_REGE_SZ_BYTES);
        return Modbus::Status_Good;
    }

    Modbus::StatusCode writeRegs(uint16_t offset, uint16_t count, const void *values)
    {
        if (static_cast<uint32_t>(offset + count) > regCount())
            return Modbus::Status_BadIllegalDataAddress;
        uint16_t *mem = reinterpret_cast<uint16_t*>(m_buff.data());
        memcpy(&mem[offset], values, count * MB_REGE_SZ_BYTES);
        return Modbus::Status_Good;
    }

    Modbus::StatusCode readBits(uint16_t offset, uint16_t count, void *values)
    {
        if (static_cast<uint32_t>(offset + count) > bitCount())
            return Modbus::Status_BadIllegalDataAddress;
        uint16_t byteOffset = offset/MB_BYTE_SZ_BITES;
        uint16_t bytes = count/MB_BYTE_SZ_BITES;
        uint16_t shift = offset%MB_BYTE_SZ_BITES;
        const uint8_t *mem = reinterpret_cast<const uint8_t*>(m_buff.data());
        if (shift)
        {
            for (uint16_t i = 0; i < bytes; i++)
            {
                uint16_t v = *(reinterpret_cast<const uint16_t*>(&mem[byteOffset+i])) >> shift;
                reinterpret_cast<uint8_t*>(values)[i] = static_cast<uint8_t>(v);
            }
            if (uint16_t resid = count%MB_BYTE_SZ_BITES)
            {
                int8_t mask = static_cast<int8_t>(0x80);
                mask = ~(mask>>(7-resid));
                if ((shift+resid) > MB_BYTE_SZ_BITES)
                {
                    uint16_t v = ((*reinterpret_cast<const uint16_t*>(&mem[byteOffset+bytes])) >> shift) & mask;
                    reinterpret_cast<uint8_t*>(values)[bytes] = static_cast<uint8_t>(v);
                }
                else
                    reinterpret_cast<uint8_t*>(values)[bytes] = (mem[byteOffset+bytes]>>shift) & mask;
            }
        }
        else
        {
            memcpy(values, &mem[byteOffset], static_cast<size_t>(bytes));
            if (uint16_t resid = count%MB_BYTE_SZ_BITES)
            {
                int8_t mask = static_cast<int8_t>(0x80);
                mask = ~(mask>>(7-resid));
                reinterpret_cast<uint8_t*>(values)[bytes] = mem[byteOffset+bytes] & mask;
            }
        }
        return Modbus::Status_Good;
    }

    Modbus::StatusCode writeBits(uint16_t offset, uint16_t count, const void *values)
    {
        if (static_cast<uint32_t>(offset + count) > bitCount())
            return Modbus::Status_BadIllegalDataAddress;
        uint16_t byteOffset = offset/MB_BYTE_SZ_BITES;
        uint16_t bytes = count/MB_BYTE_SZ_BITES;
        uint16_t shift = offset%MB_BYTE_SZ_BITES;
        uint8_t *mem = reinterpret_cast<uint8_t*>(m_buff.data());
        if (shift)
        {
            for (uint16_t i = 0; i < bytes; i++)
            {
                uint16_t mask = static_cast<uint16_t>(0x00FF) << shift;
                uint16_t v = static_cast<uint16_t>(reinterpret_cast<const uint8_t*>(values)[i]) << shift;
                *reinterpret_cast<uint16_t*>(&mem[byteOffset+i]) &= ~mask; // zero undermask values
                *reinterpret_cast<uint16_t*>(&mem[byteOffset+i]) |= v; // set bit values
            }
            if (uint16_t resid = count%MB_BYTE_SZ_BITES)
            {
                if ((shift+resid) > MB_BYTE_SZ_BITES)
                {
                    int16_t m = static_cast<int16_t>(0x8000); // using signed mask for right shift filled by '1'-bit
                    m = m>>(resid-1);
                    uint16_t mask = *reinterpret_cast<uint16_t*>(&m);
                    mask = mask >> (MB_REGE_SZ_BITES-resid-shift);
                    uint16_t v = (static_cast<uint16_t>(reinterpret_cast<const uint8_t*>(values)[bytes]) << shift) & mask;
                    *reinterpret_cast<uint16_t*>(&mem[byteOffset+bytes]) &= ~mask; // zero undermask values
                    *reinterpret_cast<uint16_t*>(&mem[byteOffset+bytes]) |= v;
                }
                else
                {
                    int8_t m = static_cast<int8_t>(0x80); // using signed mask for right shift filled by '1'-bit
                    m = m>>(resid-1);
                    uint8_t mask = *reinterpret_cast<uint8_t*>(&m);
                    mask = mask >> (MB_BYTE_SZ_BITES-resid-shift);
                    uint8_t v = (reinterpret_cast<const uint8_t*>(values)[bytes] << shift) & mask;
                    mem[byteOffset+bytes] &= ~mask; // zero undermask values
                    mem[byteOffset+bytes] |= v;
                }
            }
        }
        else
        {
            memcpy(&mem[byteOffset], values, static_cast<size_t>(bytes));
            if (uint16_t resid = count%MB_BYTE_SZ_BITES)
            {
                int8_t mask = static_cast<int8_t>(0x80);
                mask = mask>>(7-resid);
                mem[byteOffset+bytes] &= mask;
                mask = ~mask;
                mem[byteOffset+bytes] |= (reinterpret_cast<const uint8_t*>(values)[bytes] & mask);
            }
        }
        return Modbus::Status_Good;
    }

    Modbus::StatusCode readCoils(uint8_t /*unit*/, uint16_t offset, uint16_t count, void *values)
    {
        return readBits(offset, count, values);
    }

    Modbus::StatusCode readDiscreteInputs(uint8_t /*unit*/, uint16_t offset, uint16_t count, void *values)
    {
        return readBits(offset, count, values);
    }

    Modbus::StatusCode readHoldingRegisters(uint8_t /*unit*/, uint16_t offset, uint16_t count, uint16_t *values)
    {
        return readRegs(offset, count, values);
    }

    Modbus::StatusCode readInputRegisters(uint8_t /*unit*/, uint16_t offset, uint16_t count, uint16_t *values)
    {
        return readRegs(offset, count, values);
    }

    Modbus::StatusCode writeSingleCoil(uint8_t /*unit*/, uint16_t offset, bool value)
    {
        return writeBits(offset, 1, &value);
    }

    Modbus::StatusCode writeSingleRegister(uint8_t /*unit*/, uint16_t offset, uint16_t value)
    {
        return writeRegs(offset, 1, &value);
    }

    Modbus::StatusCode readExceptionStatus(uint8_t /*unit*/, uint8_t *status)
    {
        uint16_t v = 0;
        Modbus::StatusCode s = readRegs(0, 1, &v);
        if (Modbus::StatusIsGood(s))
            *status = static_cast<uint8_t>(v);
        return s;
    }

    Modbus::StatusCode writeMultipleCoils(uint8_t /*unit*/, uint16_t offset, uint16_t count, const void *values)
    {
        return writeBits(offset, count, values);
    }

    Modbus::StatusCode writeMultipleRegisters(uint8_t /*unit*/, uint16_t offset, uint16_t count, const uint16_t *values)
    {
        return writeRegs(offset, count, values);
    }

private:
    std::vector<uint16_t> m_buff;
};

struct Options
{
    Modbus::Type        type            ;
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
    uint16_t            count           ;

    Options()
    {
        const ModbusTcpServer ::Defaults &dTcp = ModbusTcpServer ::Defaults::instance();
        const ModbusSerialPort::Defaults &dSer = ModbusSerialPort::Defaults::instance();

        type            = Modbus::TCP               ;
        port            = dTcp.port                 ;
        timeout         = dTcp.timeout              ;
        portName        = dSer.portName             ;
        baudRate        = dSer.baudRate             ;
        dataBits        = dSer.dataBits             ;
        parity          = dSer.parity               ;
        stopBits        = dSer.stopBits             ;
        flowControl     = dSer.flowControl          ;
        timeoutFirstByte= dSer.timeoutFirstByte     ;
        timeoutInterByte= dSer.timeoutInterByte     ;
        count           = 1000                      ;
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
    Device dev(options.count);
    ModbusServerPort *serv;
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

        rtu->connect(&ModbusRtuPort::emitTx, printTx);
        rtu->connect(&ModbusRtuPort::emitRx, printRx);

        serv = new ModbusServerResource(rtu, &dev);
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

        asc->connect(&ModbusAscPort::emitTx, printTxAsc);
        asc->connect(&ModbusAscPort::emitRx, printRxAsc);

        serv = new ModbusServerResource(asc, &dev);
    }
        break;
    case Modbus::TCP:
    {
        ModbusTcpServer *tcp = new ModbusTcpServer(&dev);
        tcp->setPort   (options.port   );
        tcp->setTimeout(options.timeout);

        tcp->connect(&ModbusTcpServer::emitTx, printTx);
        tcp->connect(&ModbusTcpServer::emitRx, printRx);

        serv = tcp;
    }
        break;
    }

    while (1)
    {
        serv->process();
        Modbus::msleep(1);
        //using namespace std::chrono_literals;
        //std::this_thread::sleep_for(1ms);
    }
}
