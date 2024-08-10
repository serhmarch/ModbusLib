#include "ModbusQt.h"

#include "ModbusTcpPort.h"
#include "ModbusSerialPort.h"
#include "ModbusClientPort.h"
#include "ModbusServerPort.h"
#include "ModbusServerResource.h"
#include "ModbusTcpServer.h"

namespace Modbus {

Strings::Strings() :
    unit            (QStringLiteral("unit")),
    type            (QStringLiteral("type")),
    host            (QStringLiteral("host")),
    port            (QStringLiteral("port")),
    timeout         (QStringLiteral("timeout")),
    serialPortName  (QStringLiteral("serialPortName")),
    baudRate        (QStringLiteral("baudRate")),
    dataBits        (QStringLiteral("dataBits")),
    parity          (QStringLiteral("parity")),
    stopBits        (QStringLiteral("stopBits")),
    flowControl     (QStringLiteral("flowControl")),
    timeoutFirstByte(QStringLiteral("timeoutFirstByte")),
    timeoutInterByte(QStringLiteral("timeoutInterByte"))
{
}

const Strings &Strings::instance()
{
    static Strings s;
    return s;
}

Defaults::Defaults() :
    unit            (1),
    type            (TCP),
    host            (ModbusTcpPort   ::Defaults::instance().host            ),
    port            (ModbusTcpPort   ::Defaults::instance().port            ),
    timeout         (ModbusTcpPort   ::Defaults::instance().timeout         ),
    serialPortName  (ModbusSerialPort::Defaults::instance().portName        ),
    baudRate        (ModbusSerialPort::Defaults::instance().baudRate        ),
    dataBits        (ModbusSerialPort::Defaults::instance().dataBits        ),
    parity          (ModbusSerialPort::Defaults::instance().parity          ),
    stopBits        (ModbusSerialPort::Defaults::instance().stopBits        ),
    flowControl     (ModbusSerialPort::Defaults::instance().flowControl     ),
    timeoutFirstByte(ModbusSerialPort::Defaults::instance().timeoutFirstByte),
    timeoutInterByte(ModbusSerialPort::Defaults::instance().timeoutInterByte)
{
}

const Defaults &Defaults::instance()
{
    static Defaults s;
    return s;
}

#define MB_GET_SETTING_MACRO(type, name, assign)                                    \
    type v;                                                                         \
    bool okInner = false;                                                           \
    Modbus::Settings::const_iterator it = s.find(Modbus::Strings::instance().name); \
    if (it != s.end())                                                              \
    {                                                                               \
        QVariant var = it.value();                                                  \
        assign;                                                                     \
    }                                                                               \
    if (ok)                                                                         \
        *ok = okInner;                                                              \
    if (okInner)                                                                    \
        return v;                                                                   \
    return Modbus::Defaults::instance().name;


uint8_t getSettingUnit(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint8_t, unit, v = static_cast<uint8_t>(var.toUInt(&okInner)))
}

ProtocolType getSettingType(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(Modbus::ProtocolType, type, v = Modbus::toProtocolType(var, &okInner))
}

QString getSettingHost(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(QString, host, v = var.toString(); okInner = true)
}

uint16_t getSettingPort(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint16_t, port, v = static_cast<uint16_t>(var.toUInt(&okInner)))
}

uint32_t getSettingTimeout(const Settings &s, bool *ok)
{
   MB_GET_SETTING_MACRO(uint32_t, timeout, v = static_cast<uint32_t>(var.toUInt(&okInner)))
}

QString getSettingSerialPortName(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(QString, serialPortName, v = var.toString(); okInner = true)
}

int32_t getSettingBaudRate(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(int32_t, baudRate, v = static_cast<int32_t>(var.toInt(&okInner)))
}

int8_t getSettingDataBits(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(int8_t, dataBits, v = static_cast<int8_t>(var.toInt(&okInner)))
}

Parity getSettingParity(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(Modbus::Parity, parity, v = Modbus::toParity(var, &okInner))
}

StopBits getSettingStopBits(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(Modbus::StopBits, stopBits, v = Modbus::toStopBits(var, &okInner))
}

FlowControl getSettingFlowControl(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(Modbus::FlowControl, flowControl, v = Modbus::toFlowControl(var, &okInner))
}

uint32_t getSettingTimeoutFirstByte(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint32_t, timeoutFirstByte, v = static_cast<uint32_t>(var.toUInt(&okInner)))
}

uint32_t getSettingTimeoutInterByte(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint32_t, timeoutInterByte, v = static_cast<uint32_t>(var.toUInt(&okInner)))
}

void setSettingUnit(Settings &s, uint8_t v)
{
    s[Modbus::Strings::instance().unit] = v;
}

void setSettingType(Settings &s, ProtocolType v)
{
    s[Modbus::Strings::instance().type] = Modbus::toString(v);
}

void setSettingHost(Settings &s, const QString &v)
{
    s[Modbus::Strings::instance().host] = v;
}

void setSettingPort(Settings &s, uint16_t v)
{
    s[Modbus::Strings::instance().port] = v;
}

void setSettingTimeout(Settings &s, uint32_t v)
{
    s[Modbus::Strings::instance().timeout] = v;
}

void setSettingSerialPortName(Settings &s, const QString& v)
{
    s[Modbus::Strings::instance().serialPortName] = v;
}

void setSettingBaudRate(Settings &s, int32_t v)
{
    s[Modbus::Strings::instance().baudRate] = v;
}

void setSettingDataBits(Settings &s, int8_t v)
{
    s[Modbus::Strings::instance().dataBits] = v;
}

void setSettingParity(Settings &s, Parity v)
{
    s[Modbus::Strings::instance().parity] = Modbus::toString(v);
}

void setSettingStopBits(Settings &s, StopBits v)
{
    s[Modbus::Strings::instance().stopBits] = Modbus::toString(v);
}

void setSettingFlowControl(Settings &s, FlowControl v)
{
    s[Modbus::Strings::instance().flowControl] = Modbus::toString(v);
}

void setSettingTimeoutFirstByte(Settings &s, uint32_t v)
{
    s[Modbus::Strings::instance().timeoutFirstByte] = v;
}

void setSettingTimeoutInterByte(Settings &s, uint32_t v)
{
    s[Modbus::Strings::instance().timeoutInterByte] = v;
}


Address::Address()
{
    m_type = Memory_Unknown;
    m_offset = 0;
}

Address::Address(MemoryType type, quint16 offset) :
    m_type(type),
    m_offset(offset)
{
}

Address::Address(quint32 adr)
{
    this->operator=(adr);
}

QString Address::toString() const
{
    if (isValid())
        return QString("%1%2").arg(m_type).arg(number(), 5, 10, QChar('0'));
    else
        return QString();
}

Address &Address::operator=(quint32 v)
{
    quint32 number = v % 100000;
    if ((number < 1) || (number > 65536))
    {
        m_type = Memory_Unknown;
        m_offset = 0;
        return *this;
    }
    quint16 type = static_cast<quint16>(v/100000);
    switch(type)
    {
    case Memory_0x:
    case Memory_1x:
    case Memory_3x:
    case Memory_4x:
        m_type = type;
        m_offset = static_cast<quint16>(number-1);
        break;
    default:
        m_type = Memory_Unknown;
        m_offset = 0;
        break;
    }
    return *this;
}

ProtocolType toProtocolType(const QString &v, bool *ok)
{
    return enumValue<ProtocolType>(v, ok);
}

ProtocolType toProtocolType(const QVariant &v, bool *ok)
{
    return enumValue<ProtocolType>(v, ok);
}

int32_t toBaudRate(const QString &s, bool *ok)
{
    bool okInner;
    int32_t r = static_cast<int8_t>(s.toInt(&okInner));
    if (ok)
        *ok = okInner;
    if (okInner)
        return r;
    return Defaults::instance().baudRate;
}

int32_t toBaudRate(const QVariant &v, bool *ok)
{
    bool okInner;
    int32_t r = static_cast<int32_t>(v.toInt(&okInner));
    if (ok)
        *ok = okInner;
    if (okInner)
        return r;
    return Defaults::instance().baudRate;
}

int8_t toDataBits(const QString &s, bool *ok)
{
    bool okInner;
    int8_t r = static_cast<int32_t>(s.toInt(&okInner));
    if (!okInner)
    {
        okInner = true;
        if      (s == QStringLiteral("Data8"))
            r = 8;
        else if (s == QStringLiteral("Data7"))
            r = 7;
        else if (s == QStringLiteral("Data6"))
            r = 6;
        else if (s == QStringLiteral("Data5"))
            r = 5;
        else
        {
            r = Defaults::instance().dataBits;
            okInner = false;
        }
    }
    if (ok)
        *ok = okInner;
    return r;
}

int8_t toDataBits(const QVariant &v, bool *ok)
{
    bool okInner;
    int8_t r = static_cast<int8_t>(v.toInt(&okInner));
    if (!okInner)
        r = toDataBits(v.toString(), &okInner);
    if (ok)
        *ok = okInner;
    return r;
}

Parity toParity(const QString &v, bool *ok)
{
    return enumValue<Parity>(v, ok, Defaults::instance().parity);
}

Parity toParity(const QVariant &v, bool *ok)
{
    return enumValue<Parity>(v, ok, Defaults::instance().parity);
}

StopBits toStopBits(const QString &v, bool *ok)
{
    return enumValue<StopBits>(v, ok, Defaults::instance().stopBits);
}

StopBits toStopBits(const QVariant &v, bool *ok)
{
    return enumValue<StopBits>(v, ok, Defaults::instance().stopBits);
}

FlowControl toFlowControl(const QString &v, bool *ok)
{
    return enumValue<FlowControl>(v, ok, Defaults::instance().flowControl);
}

FlowControl toFlowControl(const QVariant &v, bool *ok)
{
    return enumValue<FlowControl>(v, ok, Defaults::instance().flowControl);
}

QString toString(StatusCode v)
{
    static const int index = QString("Status_").size();

    QString s = enumKey(v);
    if (s.size())
        return s.mid(index);
    return s;
}

QString toString(ProtocolType v)
{
    return enumKey(v);
}

QString toString(Parity v)
{
    return enumKey(v);
}

QString toString(StopBits v)
{
    return enumKey(v);
}

QString toString(FlowControl v)
{
    return enumKey(v);
}

QStringList availableSerialPortList()
{
    List<String> ports = availableSerialPorts();
    QStringList portList;
    for (const String &s : ports)
        portList.append(QString(s.data()));
    return portList;
}

ModbusPort *createPort(const Settings &settings, bool blocking)
{
    const Strings &s = Strings::instance();
    const Settings::const_iterator it = settings.constFind(s.type);
    if (it != settings.constEnd())
    {
        bool ok;
        ProtocolType type = toProtocolType(it.value(), &ok);
        if (ok)
        {
            switch (type)
            {
            case Modbus::TCP:
            {
                const ModbusTcpPort::Defaults &d = ModbusTcpPort::Defaults::instance();
                QByteArray host = settings.value(s.host, d.host).toString().toLatin1();
                Modbus::TcpSettings tc;
                tc.host = host.data();
                tc.port = settings.value(s.port, d.port).toUInt();
                tc.timeout = settings.value(s.timeout, d.timeout).toUInt();
                return Modbus::createPort(type, &tc, blocking);
            }
            break;
            case Modbus::RTU:
            case Modbus::ASC:
            {
                const ModbusSerialPort::Defaults &d = ModbusSerialPort::Defaults::instance();
                QByteArray portName = settings.value(s.serialPortName, d.portName).toString().toLatin1();
                Modbus::SerialSettings sl;
                sl.portName = portName.data();
                sl.baudRate = toBaudRate(settings.value(s.baudRate));
                sl.dataBits = toDataBits(settings.value(s.dataBits));
                sl.parity   = toParity(settings.value(s.parity));
                sl.stopBits = toStopBits(settings.value(s.stopBits));
                sl.flowControl = toFlowControl(settings.value(s.flowControl));
                sl.timeoutFirstByte = settings.value(s.timeoutFirstByte, d.timeoutFirstByte).toUInt();
                sl.timeoutInterByte = settings.value(s.timeoutInterByte, d.timeoutInterByte).toUInt();
                return Modbus::createPort(type, &sl, blocking);
            }
            break;
            default:
                return nullptr;
            }
        }
    }
    return nullptr;
}

ModbusClientPort *createClientPort(const Settings &settings, bool blocking)
{
    ModbusPort *port = createPort(settings, blocking);
    if (port)
        return new ModbusClientPort(port);
    return nullptr;
}

ModbusServerPort *createServerPort(ModbusInterface *device, const Settings &settings, bool blocking)
{
    const Strings &s = Strings::instance();
    const Settings::const_iterator it = settings.constFind(s.type);
    if (it != settings.constEnd())
    {
        bool ok;
        ProtocolType type = toProtocolType(it.value(), &ok);
        if (ok)
        {
            switch (type)
            {
            case Modbus::TCP:
            {
                const ModbusTcpPort::Defaults &d = ModbusTcpPort::Defaults::instance();
                ModbusTcpServer *tcp = new ModbusTcpServer(device);
                tcp->setPort   (settings.value(s.port, d.port).toUInt());
                tcp->setTimeout(settings.value(s.timeout, d.timeout).toUInt());
                return tcp;
            }
            break;
            case Modbus::RTU:
            case Modbus::ASC:
            {
                ModbusPort *port = createPort(settings, blocking);
                return new ModbusServerResource(port, device);
            }
            break;
            default:
                return nullptr;
            }
        }
    }
    return nullptr;
}

} // namespace Modbus
