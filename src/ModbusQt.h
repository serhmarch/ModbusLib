/*!
 * \file   ModbusQt.h
 * \brief  Qt support file for ModbusLib.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSQT_H
#define MODBUSQT_H

#include "Modbus.h"

#include <QMetaEnum>
#include <QHash>
#include <QVariant>

namespace Modbus {

typedef QHash<QString, QVariant> Settings;

class MODBUS_EXPORT Strings
{
public:
    const QString unit            ;
    const QString type            ;
    const QString host            ;
    const QString port            ;
    const QString timeout         ;
    const QString serialPortName  ;
    const QString baudRate        ;
    const QString dataBits        ;
    const QString parity          ;
    const QString stopBits        ;
    const QString flowControl     ;
    const QString timeoutFirstByte;
    const QString timeoutInterByte;

    Strings();
    static const Strings &instance();
};

class MODBUS_EXPORT Defaults
{
public:
    const uint8_t      unit            ;
    const ProtocolType type            ;
    const QString      host            ;
    const uint16_t     port            ;
    const uint32_t     timeout         ;
    const QString      serialPortName  ;
    const int32_t      baudRate        ;
    const int8_t       dataBits        ;
    const Parity       parity          ;
    const StopBits     stopBits        ;
    const FlowControl  flowControl     ;
    const uint32_t     timeoutFirstByte;
    const uint32_t     timeoutInterByte;

    Defaults();
    static const Defaults &instance();
};

class MODBUS_EXPORT Address
{
public:
    Address();
    Address(Modbus::MemoryType, quint16 offset);
    Address(quint32 adr);

public:
    inline bool isValid() const { return m_type != Memory_Unknown; }
    inline MemoryType type() const { return static_cast<MemoryType>(m_type); }
    inline quint16 offset() const { return m_offset; }
    inline quint32 number() const { return m_offset+1; }
    QString toString() const;
    inline operator quint32 () const { return number() | (m_type<<16);  }
    Address &operator= (quint32 v);

private:
    quint16 m_type;
    quint16 m_offset;
};

// Convert String repr to Modbus::Address
inline Address addressFromString(const QString &s) { return Address(s.toUInt()); }

// convert value to QString key for type
template <class EnumType>
inline QString enumKey(int value)
{
    const QMetaEnum me = QMetaEnum::fromType<EnumType>();
    return QString(me.valueToKey(value));
}

// convert value to QString key for type
template <class EnumType>
inline QString enumKey(EnumType value, const QString &byDef = QString())
{
    const QMetaEnum me = QMetaEnum::fromType<EnumType>();
    const char *key = me.valueToKey(value);
    if (key)
        return QString(me.valueToKey(value));
    else
        return byDef;
}

// convert key to value for enumeration by QString key
template <class EnumType>
inline EnumType enumValue(const QString& key, bool* ok = nullptr)
{
    const QMetaEnum me = QMetaEnum::fromType<EnumType>();
    return static_cast<EnumType>(me.keyToValue(key.toLatin1().constData(), ok));

}

// Convert value for enumeration by QVariant (int - value, string - key)
template <class EnumType>
inline EnumType enumValue(const QVariant& value, bool *ok)
{
    bool okInner;
    int v = value.toInt(&okInner);
    if (okInner)
    {
        const QMetaEnum me = QMetaEnum::fromType<EnumType>();
        if (me.valueToKey(v)) // check value exists
        {
            if (ok)
                *ok = true;
            return static_cast<EnumType>(v);
        }
        if (ok)
            *ok = false;
        return static_cast<EnumType>(-1);
    }
    return enumValue<EnumType>(value.toString(), ok);
}

template <class EnumType>
inline EnumType enumValue(const QVariant& value, EnumType defaultValue)
{
    bool okInner;
    EnumType v = enumValue<EnumType>(value, &okInner);
    if (okInner)
        return v;
    return defaultValue;
}

template <class EnumType>
inline EnumType enumValue(const QVariant& value)
{
    return enumValue<EnumType>(value, nullptr);
}

/// \details Converts string representation to `ProtocolType` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT ProtocolType toProtocolType(const QString &s, bool *ok = nullptr);

/// \details Converts string representation to `ProtocolType` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT ProtocolType toProtocolType(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `Parity` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT Parity toParity(const QString &s, bool *ok = nullptr);

/// \details Converts string representation to `Parity` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT Parity toParity(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `StopBits` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT StopBits toStopBits(const QString &s, bool *ok = nullptr);

/// \details Converts string representation to `StopBits` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT StopBits toStopBits(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `FlowControl` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT FlowControl toFlowControl(const QString &s, bool *ok = nullptr);

/// \details Converts string representation to `FlowControl` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT FlowControl toFlowControl(const QVariant &v, bool *ok = nullptr);

/// \details Returns string representation of `StatusCode` enum value
MODBUS_EXPORT QString toString(StatusCode v);

/// \details Returns string representation of `ProtocolType` enum value
MODBUS_EXPORT QString toString(ProtocolType v);

/// \details Returns string representation of `Parity` enum value
MODBUS_EXPORT QString toString(Parity v);

/// \details Returns string representation of `StopBits` enum value
MODBUS_EXPORT QString toString(StopBits v);

/// \details Returns string representation of `FlowControl` enum value
MODBUS_EXPORT QString toString(FlowControl v);

/// \details Returns list of string that represent names of serial ports
MODBUS_EXPORT QStringList availableSerialPortList();

/// \details
MODBUS_EXPORT ModbusPort *createPort(const Settings &settings, bool blocking = false);

/// \details
MODBUS_EXPORT ModbusClientPort *createClientPort(const Settings &settings, bool blocking = false);

/// \details
MODBUS_EXPORT ModbusServerPort *createServerPort(ModbusInterface *device, const Settings &settings, bool blocking = false);

} // namespace Modbus

#endif // MODBUSQT_H
