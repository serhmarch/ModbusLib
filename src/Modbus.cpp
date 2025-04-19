#include "Modbus.h"

#include "ModbusAscPort.h"
#include "ModbusRtuPort.h"
#include "ModbusTcpPort.h"

#ifndef MB_CLIENT_DISABLE
#include "ModbusClientPort.h"
#endif // MB_CLIENT_DISABLE

#ifndef MB_SERVER_DISABLE
#include "ModbusTcpServer.h"
#include "ModbusServerResource.h"
#include "ModbusGlobal.h"
#endif // MB_SERVER_DISABLE

namespace Modbus {

static inline Char hexDigit(uint8_t value) { return value < 10 ? '0' + value : 'A' + (value - 10); }

uint32_t modbusLibVersion()
{
    return MODBUSLIB_VERSION;
}

const Char* modbusLibVersionStr()
{
    return MODBUSLIB_VERSION_STR;
}

uint16_t crc16(const uint8_t *bytes, uint32_t count)
{
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < count; i++)
    {
        crc ^= bytes[i];
        for (uint32_t j = 0; j < 8; j++)
        {
            uint16_t temp = crc & 0x0001;
            crc >>= 1;
            if (temp) crc ^= 0xA001;
        }
    }
    return crc;
}

uint8_t lrc(const uint8_t *bytes, uint32_t count)
{
    uint8_t lrc = 0x00;
    for (; count; count--)
        lrc += *bytes++;
    return static_cast<uint8_t>(-static_cast<int8_t>(lrc));
}

StatusCode readMemRegs(uint32_t offset, uint32_t count, void *values, const void *memBuff, uint32_t memRegCount, uint32_t *outCount)
{
    if (static_cast<uint32_t>(offset + count) > memRegCount)
    {
        if (outCount && (offset < memRegCount))
            count = memRegCount - offset;
        else
            return Status_BadIllegalDataAddress;
    }
    const uint16_t *mem = reinterpret_cast<const uint16_t*>(memBuff);
    memcpy(values, &mem[offset], count * MB_REGE_SZ_BYTES);
    if (outCount)
        *outCount = count;
    return Status_Good;
}

StatusCode writeMemRegs(uint32_t offset, uint32_t count, const void *values, void *memBuff, uint32_t memRegCount, uint32_t *outCount)
{
    if (static_cast<uint32_t>(offset + count) > memRegCount)
    {
        if (outCount && (offset < memRegCount))
            count = memRegCount - offset;
        else
            return Status_BadIllegalDataAddress;
    }
    uint16_t *mem = reinterpret_cast<uint16_t*>(memBuff);
    memcpy(&mem[offset], values, count * MB_REGE_SZ_BYTES);
    if (outCount)
        *outCount = count;
    return Status_Good;
}

StatusCode readMemBits(uint32_t offset, uint32_t count, void *values, const void *memBuff, uint32_t memBitCount, uint32_t *outCount)
{
    if (static_cast<uint32_t>(offset + count) > memBitCount)
    {
        if (outCount && (offset < memBitCount))
            count = memBitCount - offset;
        else
            return Status_BadIllegalDataAddress;
    }
    uint16_t byteOffset = offset/MB_BYTE_SZ_BITES;
    uint16_t bytes = count/MB_BYTE_SZ_BITES;
    uint16_t shift = offset%MB_BYTE_SZ_BITES;
    const uint8_t *mem = reinterpret_cast<const uint8_t*>(memBuff);
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
    if (outCount)
        *outCount = count;
    return Status_Good;
}

StatusCode writeMemBits(uint32_t offset, uint32_t count, const void *values, void *memBuff, uint32_t memBitCount, uint32_t *outCount)
{
    if (static_cast<uint32_t>(offset + count) > memBitCount)
    {
        if (outCount && (offset < memBitCount))
            count = memBitCount - offset;
        else
            return Status_BadIllegalDataAddress;
    }
    uint16_t byteOffset = offset/MB_BYTE_SZ_BITES;
    uint16_t bytes = count/MB_BYTE_SZ_BITES;
    uint16_t shift = offset%MB_BYTE_SZ_BITES;
    uint8_t *mem = reinterpret_cast<uint8_t*>(memBuff);
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
    if (outCount)
        *outCount = count;
    return Status_Good;
}


uint32_t bytesToAscii(const uint8_t *bytesBuff, uint8_t* asciiBuff, uint32_t count)
{
    uint32_t i, j, qAscii = 0;
    uint8_t tmp;

    for (i = 0; i < count; i++)
    {
        for (j = 0; j < 2; j++)
        {
            tmp = (bytesBuff[i] >> ((1 - j) * 4)) & 0x0F;
            asciiBuff[i * 2 + j] = ((tmp < 10) ? (0x30 | tmp) : (55 + tmp));
            qAscii++;
        }
    }
    return qAscii;
}

uint32_t asciiToBytes(const uint8_t *asciiBuff, uint8_t* bytesBuff, uint32_t count)
{
    uint32_t i, j, qBytes = 0;
    uint8_t tmp;
    for (i = 0; i < count; i++)
    {
        j = i & 1;
        if (!j)
        {
            bytesBuff[i / 2] = 0;
            qBytes++;
        }
        tmp = asciiBuff[i];
        if ((tmp >= '0') && (tmp <= '9'))
            tmp &= 0x0F;
        else if ((tmp >= 'A') && (tmp <= 'F'))
            tmp -= 55;
        else
            return 0;
        bytesBuff[i / 2] |= tmp << (4 * (1 - j));
    }
    return qBytes;
}

Char *sbytes(const uint8_t* buff, uint32_t count, Char *str, uint32_t strmaxlen)
{
    String s = bytesToString(buff, count);
    strncpy(str, s.data(), strmaxlen-1);
    str[strmaxlen-1] = '\0';
    return str;
}

Char *sascii(const uint8_t* buff, uint32_t count, Char *str, uint32_t strmaxlen)
{
    String s = asciiToString(buff, count);
    strncpy(str, s.data(), strmaxlen-1);
    str[strmaxlen-1] = '\0';
    return str;
}

String bytesToString(const uint8_t* buff, uint32_t count)
{
    String str;
    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t num = buff[i];
        str += hexDigit(num >> 4);
        str += hexDigit(num & 0xF);
        str += CharLiteral(' ');
    }
    return str;
}

String asciiToString(const uint8_t* buff, uint32_t count)
{
    String str;
    bool lastHex = false;
    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t c = buff[i];
        switch (c)
        {
        case '\r':
            str += ((str.size() && (str.back() == CharLiteral(' '))) ? StringLiteral("CR ") : StringLiteral(" CR "));
            lastHex = false;
            break;
        case '\n':
            str += ((str.size() && (str.back() == CharLiteral(' '))) ? StringLiteral("LF ") : StringLiteral(" LF "));
            lastHex = false;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            str += c;
            if (lastHex)
            {
                str += " ";
                lastHex = false;
            }
            else
                lastHex = true;
            break;
        default:
            str += c;
            str += " ";
            lastHex = false;
            break;
        }
    }
    return str;
}

const Char *sprotocolType(ProtocolType type)
{
    switch (type)
    {
    case ASC: return StringLiteral("ASC");
    case RTU: return StringLiteral("RTU");
    case TCP: return StringLiteral("TCP");
    default: return nullptr;
    }
}

ProtocolType toProtocolType(const Char * s)
{
    if (strcmp(s, StringLiteral("ASC")) == 0) return ASC;
    if (strcmp(s, StringLiteral("RTU")) == 0) return RTU;
    if (strcmp(s, StringLiteral("TCP")) == 0) return TCP;
    return static_cast<ProtocolType>(-1);
}   

const Char* sbaudRate(int32_t baudRate)
{
    switch (baudRate)
    {
    case 1200  : return StringLiteral("1200"  );
    case 2400  : return StringLiteral("2400"  );
    case 4800  : return StringLiteral("4800"  );
    case 9600  : return StringLiteral("9600"  );
    case 19200 : return StringLiteral("19200" );
    case 38400 : return StringLiteral("38400" );
    case 57600 : return StringLiteral("57600" );
    case 115200: return StringLiteral("115200");
    default: return nullptr;
    }
}

int32_t toBaudRate(const Char * s)
{
    if (strcmp(s, StringLiteral("1200"  )) == 0) return 1200  ;
    if (strcmp(s, StringLiteral("2400"  )) == 0) return 2400  ;
    if (strcmp(s, StringLiteral("4800"  )) == 0) return 4800  ;
    if (strcmp(s, StringLiteral("9600"  )) == 0) return 9600  ;
    if (strcmp(s, StringLiteral("19200" )) == 0) return 19200 ;
    if (strcmp(s, StringLiteral("38400" )) == 0) return 38400 ;
    if (strcmp(s, StringLiteral("57600" )) == 0) return 57600 ;
    if (strcmp(s, StringLiteral("115200")) == 0) return 115200;
    return -1;
}   

const Char *sdataBits(int8_t dataBits)
{
    switch (dataBits)
    {
    case 5: return StringLiteral("5");
    case 6: return StringLiteral("6");
    case 7: return StringLiteral("7");
    case 8: return StringLiteral("8");
    default: return nullptr;
    }
}

int8_t toDataBits(const Char * s)
{
    if (strcmp(s, StringLiteral("5")) == 0) return 5;
    if (strcmp(s, StringLiteral("6")) == 0) return 6;
    if (strcmp(s, StringLiteral("7")) == 0) return 7;
    if (strcmp(s, StringLiteral("8")) == 0) return 8;
    return -1;
}   

const Char *sparity(Parity parity)
{
    switch (parity)
    {
    case NoParity   : return StringLiteral("No"   );
    case EvenParity : return StringLiteral("Even" );
    case OddParity  : return StringLiteral("Odd"  );
    case SpaceParity: return StringLiteral("Space");
    case MarkParity : return StringLiteral("Mark" );        
    default: return nullptr;
    }
}

Parity toParity(const Char* s)
{
    if (strcmp(s, StringLiteral("No"   )) == 0 || strcmp(s, StringLiteral("N")) == 0) return NoParity   ;
    if (strcmp(s, StringLiteral("Even" )) == 0 || strcmp(s, StringLiteral("E")) == 0) return EvenParity ;
    if (strcmp(s, StringLiteral("Odd"  )) == 0 || strcmp(s, StringLiteral("O")) == 0) return OddParity  ;
    if (strcmp(s, StringLiteral("Space")) == 0 || strcmp(s, StringLiteral("S")) == 0) return SpaceParity;
    if (strcmp(s, StringLiteral("Mark" )) == 0 || strcmp(s, StringLiteral("M")) == 0) return MarkParity ;
    return static_cast<Parity>(-1);
}   

const Char *sstopBits(StopBits stopBits)
{
    switch (stopBits)
    {
    case OneStop       : return StringLiteral("1"  );
    case OneAndHalfStop: return StringLiteral("1.5");
    case TwoStop       : return StringLiteral("2"  );
    default: return nullptr;
    }
}

StopBits toStopBits(const Char* s)
{
    if (strcmp(s, StringLiteral("1"  )) == 0) return OneStop       ;
    if (strcmp(s, StringLiteral("1.5")) == 0) return OneAndHalfStop;
    if (strcmp(s, StringLiteral("2"  )) == 0) return TwoStop       ;
    return static_cast<StopBits>(-1);
}   

const Char *sflowControl(FlowControl flowControl)
{
    switch (flowControl)
    {
    case NoFlowControl  : return StringLiteral("No"  );
    case HardwareControl: return StringLiteral("Hard");
    case SoftwareControl: return StringLiteral("Soft");
    default: return nullptr;
    }
}

FlowControl toFlowControl(const Char* s)
{
    if (strcmp(s, StringLiteral("No"  )) == 0) return NoFlowControl  ;
    if (strcmp(s, StringLiteral("Hard")) == 0) return HardwareControl;
    if (strcmp(s, StringLiteral("Soft")) == 0) return SoftwareControl;
    return static_cast<FlowControl>(-1);
}   

ModbusPort *createPort(ProtocolType type, const void *settings, bool blocking)
{
    ModbusPort *port = nullptr;
    switch (type)
    {
    case RTU:
    {
        ModbusRtuPort *rtu = new ModbusRtuPort(blocking);
        const SerialSettings *s = reinterpret_cast<const SerialSettings*>(settings);
        rtu->setPortName        (s->portName        );
        rtu->setBaudRate        (s->baudRate        );
        rtu->setDataBits        (s->dataBits        );
        rtu->setParity          (s->parity          );
        rtu->setStopBits        (s->stopBits        );
        rtu->setFlowControl     (s->flowControl     );
        rtu->setTimeoutFirstByte(s->timeoutFirstByte);
        rtu->setTimeoutInterByte(s->timeoutInterByte);
        port = rtu;
    }
        break;
    case ASC:
    {
        ModbusAscPort *asc = new ModbusAscPort(blocking);
        const SerialSettings *s = reinterpret_cast<const SerialSettings*>(settings);
        asc->setPortName        (s->portName        );
        asc->setBaudRate        (s->baudRate        );
        asc->setDataBits        (s->dataBits        );
        asc->setParity          (s->parity          );
        asc->setStopBits        (s->stopBits        );
        asc->setFlowControl     (s->flowControl     );
        asc->setTimeoutFirstByte(s->timeoutFirstByte);
        asc->setTimeoutInterByte(s->timeoutInterByte);
        port = asc;
    }
        break;
    case TCP:
    {
        ModbusTcpPort *tcp = new ModbusTcpPort(blocking);
        const TcpSettings *s = reinterpret_cast<const TcpSettings*>(settings);
        tcp->setHost   (s->host   );
        tcp->setPort   (s->port   );
        tcp->setTimeout(s->timeout);
        port = tcp;
    }
        break;
    }
    return port;
}

#ifndef MB_CLIENT_DISABLE
ModbusClientPort *createClientPort(ProtocolType type, const void *settings, bool blocking)
{
    ModbusPort *port = createPort(type, settings, blocking);
    ModbusClientPort *clientPort = new ModbusClientPort(port);
    return clientPort;
}
#endif // MB_CLIENT_DISABLE

#ifndef MB_SERVER_DISABLE
ModbusServerPort *createServerPort(ModbusInterface *device, ProtocolType type, const void *settings, bool blocking)
{
    ModbusServerPort *serv = nullptr;
    switch (type)
    {
    case RTU:
    case ASC:
    {
        ModbusPort *port = createPort(type, settings, blocking);
        serv = new ModbusServerResource(port, device);
    }
        break;
    case TCP:
    {
        ModbusTcpServer *tcp = new ModbusTcpServer(device);
        const TcpSettings *s = reinterpret_cast<const TcpSettings*>(settings);
        tcp->setPort          (s->port   );
        tcp->setTimeout       (s->timeout);
        tcp->setMaxConnections(s->maxconn);
        serv = tcp;
    }
        break;
    }
    return serv;
}
#endif // MB_SERVER_DISABLE

List<int32_t> availableBaudRate()
{
    List<int32_t> ls;
    ls.push_back(1200);
    ls.push_back(2400);
    ls.push_back(4800);
    ls.push_back(9600);
    ls.push_back(19200);
    ls.push_back(38400);
    ls.push_back(57600);
    ls.push_back(115200);
    return ls;
}

List<int8_t> availableDataBits()
{
    List<int8_t> ls;
    ls.push_back(5);
    ls.push_back(6);
    ls.push_back(7);
    ls.push_back(8);
    return ls;
}

List<Parity> availableParity()
{
    List<Parity> ls;
    ls.push_back(NoParity   );
    ls.push_back(EvenParity );
    ls.push_back(OddParity  );
    ls.push_back(SpaceParity);
    ls.push_back(MarkParity );
    return ls;
}

List<StopBits> availableStopBits()
{
    List<StopBits> ls;
    ls.push_back(OneStop       );
    ls.push_back(OneAndHalfStop);
    ls.push_back(TwoStop       );
    return ls;
}

List<FlowControl> availableFlowControl()
{
    List<FlowControl> ls;
    ls.push_back(NoFlowControl  );
    ls.push_back(HardwareControl);
    ls.push_back(SoftwareControl);
    return ls;
}

#ifndef MB_ADDRESS_CLASS_DISABLE

const Char *sIEC61131Prefix0x  = StringLiteral("%Q") ;
const Char *sIEC61131Prefix1x  = StringLiteral("%I") ;
const Char *sIEC61131Prefix3x  = StringLiteral("%IW");
const Char *sIEC61131Prefix4x  = StringLiteral("%MW");   
const Char  cIEC61131SuffixHex = CharLiteral('h');

Address Address::fromString(const String &s)
{
    if (s.size() && s.at(0) == '%')
    {
        Address adr;
        size_t i;
        // Note: 3x (%IW) handled before 1x (%I)
        if (s.find(sIEC61131Prefix3x, 0) == 0) // Check if string starts with sIEC61131Prefix3x
        {
            adr.m_type = Modbus::Memory_3x;
            i = std::strlen(sIEC61131Prefix3x);
        }
        else if (s.find(sIEC61131Prefix4x, 0) == 0) // Check if string starts with sIEC61131Prefix4x
        {
            adr.m_type = Modbus::Memory_4x;
            i = std::strlen(sIEC61131Prefix4x);
        }
        else if (s.find(sIEC61131Prefix0x, 0) == 0) // Check if string starts with sIEC61131Prefix0x
        {
            adr.m_type = Modbus::Memory_0x;
            i = std::strlen(sIEC61131Prefix0x);
        }
        else if (s.find(sIEC61131Prefix1x, 0) == 0) // Check if string starts with sIEC61131Prefix1x
        {
            adr.m_type = Modbus::Memory_1x;
            i = std::strlen(sIEC61131Prefix1x);
        }
        else
            return Address();

        Char suffix = s.back();
        if (suffix == cIEC61131SuffixHex)
        {
            char* end = nullptr;
            adr.m_offset = static_cast<uint16_t>(std::strtol(s.substr(i, s.size() - i - 1).data(), &end, 16));
        }
        else
        {
            adr.m_offset = static_cast<uint16_t>(std::atoi(s.substr(i).data()));
        }
        return adr;
    }
    return Address(std::atoi(s.data()));
}

Address::Address()
{
    m_type = Modbus::Memory_Unknown;
    m_offset = 0;
}

Address::Address(Modbus::MemoryType type, uint16_t offset) :
    m_type(type),
    m_offset(offset)
{
}

Address::Address(uint32_t adr)
{
    this->operator=(adr);
}

String Address::toString(Notation notation) const
{
    if (isValid())
    {
        switch (notation)
        {
        case Notation_IEC61131:
            switch (m_type)
            {
            case Modbus::Memory_0x:
                return String(sIEC61131Prefix0x) + std::to_string(number());
            case Modbus::Memory_1x:
                return String(sIEC61131Prefix1x) + std::to_string(number());
            case Modbus::Memory_3x:
                return String(sIEC61131Prefix3x) + std::to_string(number());
            case Modbus::Memory_4x:
                return String(sIEC61131Prefix4x) + std::to_string(number());
            default:
                return String();;
            }
            break;
        case Notation_IEC61131Hex:
        {
            const char *prefix;
            switch (m_type)
            {
            case Modbus::Memory_0x:
                prefix = sIEC61131Prefix0x;
                break;
            case Modbus::Memory_1x:
                prefix = sIEC61131Prefix0x;
                break;
            case Modbus::Memory_3x:
                prefix = sIEC61131Prefix0x;
                break;
            case Modbus::Memory_4x:
                prefix = sIEC61131Prefix0x;
                break;
            default:
                return String();
            }
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%s%04X%c", prefix, number(), cIEC61131SuffixHex);
            return String(buffer);
        }
            break;
        default:
            return std::to_string(m_type) + std::to_string(number()).insert(0, 5 - std::to_string(number()).length(), '0');
        }
    }
    else
        return String();
}

Address &Address::operator=(uint32_t v)
{
    uint32_t number = v % 100000;
    if ((number < 1) || (number > 65536))
    {
        m_type = Modbus::Memory_Unknown;
        m_offset = 0;
        return *this;
    }
    uint16_t type = static_cast<uint16_t>(v/100000);
    switch(type)
    {
    case Modbus::Memory_0x:
    case Modbus::Memory_1x:
    case Modbus::Memory_3x:
    case Modbus::Memory_4x:
        m_type = type;
        m_offset = static_cast<uint16_t>(number-1);
        break;
    default:
        m_type = Modbus::Memory_Unknown;
        m_offset = 0;
        break;
    }
    return *this;
}

#endif // MB_ADDRESS_CLASS_DISABLE

} //namespace Modbus

#ifndef MBF_READ_COILS_DISABLE
Modbus::StatusCode ModbusInterface::readCoils(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*count*/, void */*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
Modbus::StatusCode ModbusInterface::readDiscreteInputs(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*count*/, void */*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
Modbus::StatusCode ModbusInterface::readHoldingRegisters(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*count*/, uint16_t */*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
Modbus::StatusCode ModbusInterface::readInputRegisters(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*count*/, uint16_t */*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
Modbus::StatusCode ModbusInterface::writeSingleCoil(uint8_t /*unit*/, uint16_t /*offset*/, bool /*value*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
Modbus::StatusCode ModbusInterface::writeSingleRegister(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*value*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
Modbus::StatusCode ModbusInterface::readExceptionStatus(uint8_t /*unit*/, uint8_t * /*status*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_DIAGNOSTICS_DISABLE
Modbus::StatusCode ModbusInterface::diagnostics(uint8_t /*unit*/, uint16_t /*subfunc*/, uint8_t /*insize*/, const uint8_t * /*indata*/, uint8_t * /*outsize*/, uint8_t * /*outdata*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
Modbus::StatusCode ModbusInterface::getCommEventCounter(uint8_t /*unit*/, uint16_t * /*status*/, uint16_t * /*eventCount*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
Modbus::StatusCode ModbusInterface::getCommEventLog(uint8_t /*unit*/, uint16_t * /*status*/, uint16_t * /*eventCount*/, uint16_t * /*messageCount*/, uint8_t * /*eventBuffSize*/, uint8_t * /*eventBuff*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
Modbus::StatusCode ModbusInterface::writeMultipleCoils(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*count*/, const void * /*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
Modbus::StatusCode ModbusInterface::writeMultipleRegisters(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*count*/, const uint16_t * /*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_REPORT_SERVER_ID_DISABLE
Modbus::StatusCode ModbusInterface::reportServerID(uint8_t /*unit*/, uint8_t * /*count*/, uint8_t * /*data*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
Modbus::StatusCode ModbusInterface::maskWriteRegister(uint8_t /*unit*/, uint16_t /*offset*/, uint16_t /*andMask*/, uint16_t /*orMask*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
Modbus::StatusCode ModbusInterface::readWriteMultipleRegisters(uint8_t /*unit*/, uint16_t /*readOffset*/, uint16_t /*readCount*/, uint16_t */*readValues*/, uint16_t /*writeOffset*/, uint16_t /*writeCount*/, const uint16_t */*writeValues*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
Modbus::StatusCode ModbusInterface::readFIFOQueue(uint8_t /*unit*/, uint16_t /*fifoadr*/, uint16_t */*count*/, uint16_t */*values*/)
{
    return Modbus::Status_BadIllegalFunction;
}
#endif
