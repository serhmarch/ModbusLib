#include "ModbusQt.h"

#include "ModbusClientPort.h"
#include "ModbusServerPort.h"
#include "ModbusServerResource.h"
#include "ModbusTcpServer.h"

namespace Modbus {

Strings::Strings() :
    unit              (QStringLiteral("unit")),
    type              (QStringLiteral("type")),
    tries             (QStringLiteral("tries")),
    host              (QStringLiteral("host")),
    ipaddr            (QStringLiteral("ipaddr")),
    port              (QStringLiteral("port")),
    timeout           (QStringLiteral("timeout")),
    maxconn           (QStringLiteral("maxconn")),
    serialPortName    (QStringLiteral("serialPortName")),
    baudRate          (QStringLiteral("baudRate")),
    dataBits          (QStringLiteral("dataBits")),
    parity            (QStringLiteral("parity")),
    stopBits          (QStringLiteral("stopBits")),
    flowControl       (QStringLiteral("flowControl")),
    timeoutFirstByte  (QStringLiteral("timeoutFirstByte")),
    timeoutInterByte  (QStringLiteral("timeoutInterByte")),
    isBroadcastEnabled(QStringLiteral("isBroadcastEnabled")),

    NoParity          (sparity(Modbus::NoParity   )),
    EvenParity        (sparity(Modbus::EvenParity )),
    OddParity         (sparity(Modbus::OddParity  )),
    SpaceParity       (sparity(Modbus::SpaceParity)),
    MarkParity        (sparity(Modbus::MarkParity )),

    OneStop           (sstopBits(Modbus::OneStop       )),
    OneAndHalfStop    (sstopBits(Modbus::OneAndHalfStop)),
    TwoStop           (sstopBits(Modbus::TwoStop       )),

    NoFlowControl     (sflowControl(Modbus::NoFlowControl  )),
    HardwareControl   (sflowControl(Modbus::HardwareControl)),
    SoftwareControl   (sflowControl(Modbus::SoftwareControl))
{
}

const Strings &Strings::instance()
{
    static Strings s;
    return s;
}

Defaults::Defaults() :
    unit              (1),
    type              (TCP),
    tries             (1), // TODO: initialize by constant from ModbusClientPort
    host              (Modbus::NetDefaults   ::instance().host            ),
    ipaddr            (Modbus::NetDefaults   ::instance().ipaddr          ),
    port              (Modbus::NetDefaults   ::instance().port            ),
    timeout           (Modbus::NetDefaults   ::instance().timeout         ),
    maxconn           (Modbus::NetDefaults   ::instance().maxconn         ),
    serialPortName    (Modbus::SerialDefaults::instance().portName        ),
    baudRate          (Modbus::SerialDefaults::instance().baudRate        ),
    dataBits          (Modbus::SerialDefaults::instance().dataBits        ),
    parity            (Modbus::SerialDefaults::instance().parity          ),
    stopBits          (Modbus::SerialDefaults::instance().stopBits        ),
    flowControl       (Modbus::SerialDefaults::instance().flowControl     ),
    timeoutFirstByte  (Modbus::SerialDefaults::instance().timeoutFirstByte),
    timeoutInterByte  (Modbus::SerialDefaults::instance().timeoutInterByte),
    isBroadcastEnabled(true)
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

uint32_t getSettingTries(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint32_t, tries, v = static_cast<uint32_t>(var.toUInt(&okInner)))
}

QString getSettingHost(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(QString, host, v = var.toString(); okInner = true)
}

MODBUS_EXPORT QString getSettingIpaddr(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(QString, ipaddr, v = var.toString(); okInner = true)
}

uint16_t getSettingPort(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint16_t, port, v = static_cast<uint16_t>(var.toUInt(&okInner)))
}

uint32_t getSettingTimeout(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint32_t, timeout, v = static_cast<uint32_t>(var.toUInt(&okInner)))
}

uint32_t getSettingMaxconn(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(uint32_t, maxconn, v = static_cast<uint32_t>(var.toUInt(&okInner)))
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

bool getSettingBroadcastEnabled(const Settings &s, bool *ok)
{
    MB_GET_SETTING_MACRO(bool, isBroadcastEnabled, v = var.toBool(); okInner = true)
}

void setSettingUnit(Settings &s, uint8_t v)
{
    s[Modbus::Strings::instance().unit] = v;
}

void setSettingType(Settings &s, ProtocolType v)
{
    s[Modbus::Strings::instance().type] = Modbus::toString(v);
}

void setSettingTries(Settings &s, uint32_t v)
{
    s[Modbus::Strings::instance().tries] = v;
}

void setSettingHost(Settings &s, const QString &v)
{
    s[Modbus::Strings::instance().host] = v;
}

MODBUS_EXPORT void setSettingIpaddr(Settings &s, const QString &v)
{
    s[Modbus::Strings::instance().ipaddr] = v;
}

void setSettingPort(Settings &s, uint16_t v)
{
    s[Modbus::Strings::instance().port] = v;
}

void setSettingTimeout(Settings &s, uint32_t v)
{
    s[Modbus::Strings::instance().timeout] = v;
}

void setSettingMaxconn(Settings &s, uint32_t v)
{
    s[Modbus::Strings::instance().maxconn] = v;
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

void setSettingBroadcastEnabled(Settings &s, bool v)
{
    s[Modbus::Strings::instance().isBroadcastEnabled] = v;
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
    const Strings &s = Strings::instance();
    Parity d;
    bool okInner = true;
    if (v == s.NoParity)
        d = NoParity;
    else if (v == s.EvenParity)
        d = EvenParity;
    else if (v == s.OddParity)
        d = OddParity;
    else if (v == s.SpaceParity)
        d = SpaceParity;
    else if (v == s.MarkParity)
        d = MarkParity;
    else
        okInner = false;
    if (okInner)
    {
        if (ok)
            *ok = okInner;
        return d;
    }
    return enumValue<Parity>(v, ok, Defaults::instance().parity);
}

Parity toParity(const QVariant &v, bool *ok)
{
    if (v.type() == QVariant::String)
        return toParity(v.toString(), ok);
    return enumValue<Parity>(v, ok, Defaults::instance().parity);
}

StopBits toStopBits(const QString &v, bool *ok)
{
    const Strings &s = Strings::instance();
    StopBits d;
    bool okInner = true;
    if (v == s.OneStop)
        d = OneStop;
    else if (v == s.OneAndHalfStop)
        d = OneAndHalfStop;
    else if (v == s.TwoStop)
        d = TwoStop;
    else
        okInner = false;
    if (okInner)
    {
        if (ok)
            *ok = okInner;
        return d;
    }
    return enumValue<StopBits>(v, ok, Defaults::instance().stopBits);
}

StopBits toStopBits(const QVariant &v, bool *ok)
{
    if (v.type() == QVariant::String)
        return toStopBits(v.toString(), ok);
    return enumValue<StopBits>(v, ok, Defaults::instance().stopBits);
}

FlowControl toFlowControl(const QString &v, bool *ok)
{
    const Strings &s = Strings::instance();
    FlowControl d;
    bool okInner = true;
    if (v == s.NoFlowControl)
        d = NoFlowControl;
    else if (v == s.HardwareControl)
        d = HardwareControl;
    else if (v == s.SoftwareControl)
        d = SoftwareControl;
    else
        okInner = false;
    if (okInner)
    {
        if (ok)
            *ok = okInner;
        return d;
    }
    return enumValue<FlowControl>(v, ok, Defaults::instance().flowControl);
}

FlowControl toFlowControl(const QVariant &v, bool *ok)
{
    if (v.type() == QVariant::String)
        return toFlowControl(v.toString(), ok);
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
    const Strings &s = Strings::instance();
    switch (v)
    {
    case NoParity   : return s.NoParity   ;
    case EvenParity : return s.EvenParity ;
    case OddParity  : return s.OddParity  ;
    case SpaceParity: return s.SpaceParity;
    case MarkParity : return s.MarkParity ;
    }
    return enumKey(v);
}

QString toString(StopBits v)
{
    const Strings &s = Strings::instance();
    switch (v)
    {
    case OneStop       : return s.OneStop       ;
    case OneAndHalfStop: return s.OneAndHalfStop;
    case TwoStop       : return s.TwoStop       ;
    }
    return enumKey(v);
}

QString toString(FlowControl v)
{
    const Strings &s = Strings::instance();
    switch (v)
    {
    case NoFlowControl  : return s.NoFlowControl  ;
    case HardwareControl: return s.HardwareControl;
    case SoftwareControl: return s.SoftwareControl;
    }
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
            case Modbus::ASC:
            case Modbus::RTU:
            {
                const Modbus::SerialDefaults &d = Modbus::SerialDefaults::instance();
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
            {
                const Modbus::NetDefaults &d = Modbus::NetDefaults::instance();
                QByteArray host = settings.value(s.host, d.host).toString().toLatin1();
                Modbus::NetSettings nc;
                nc.host    = host.data();
                nc.port    = settings.value(s.port, d.port).toUInt();
                nc.timeout = settings.value(s.timeout, d.timeout).toUInt();
                return Modbus::createPort(type, &nc, blocking);
            }
                break;
            }
        }
    }
    return nullptr;
}

ModbusClientPort *createClientPort(const Settings &settings, bool blocking)
{
    ModbusPort *port = createPort(settings, blocking);
    if (port)
    {
        ModbusClientPort *cp = new ModbusClientPort(port);
        cp->setTries(getSettingTries(settings));
        return cp;
    }
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
            case Modbus::ASCvTCP:
            case Modbus::RTUvTCP:
            {
                const auto &d = ModbusTcpServer::Defaults::instance();
                QByteArray ipaddr = settings.value(s.ipaddr, d.ipaddr).toString().toLatin1();

                Modbus::NetSettings net;
                net.ipaddr  = ipaddr.data();
                net.port    = (settings.value(s.port   , d.port   ).toUInt());
                net.timeout = (settings.value(s.timeout, d.timeout).toUInt());
                net.maxconn = (settings.value(s.maxconn, d.maxconn).toUInt());
                return createServer(device, type, &net, blocking);
            }
                break;
            default:
            {
                ModbusPort *port = createPort(settings, blocking);
                return new ModbusServerResource(port, device);
            }
                break;
            }
        }
    }
    return nullptr;
}

} // namespace Modbus
