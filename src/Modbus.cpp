#include "Modbus.h"

namespace Modbus {

static inline Char hexDigit(uint8_t value) { return value < 10 ? '0' + value : 'A' + (value - 10); }

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

uint16_t bytesToAscii(const uint8_t *bytesBuff, uint8_t* asciiBuff, uint32_t count)
{
    uint16_t i, j, qAscii = 0;
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

uint16_t asciiToBytes(const uint8_t *asciiBuff, uint8_t* bytesBuff, uint32_t count)
{
    uint16_t i, j, qBytes = 0;
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

} //namespace Modbus
