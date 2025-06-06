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

/// \brief Map for settings of Modbus protocol where key has type `QString` and value is `QVariant`.
typedef QHash<QString, QVariant> Settings;

/*! \brief Sets constant key values for the map of settings.
 */
class MODBUS_EXPORT Strings
{
public:
    const QString unit              ; ///< Setting key for the unit number of remote device
    const QString type              ; ///< Setting key for the type of Modbus protocol
    const QString tries             ; ///< Setting key for the number of tries a Modbus request is repeated if it fails
    const QString host              ; ///< Setting key for the IP address or DNS name of the remote device
    const QString port              ; ///< Setting key for the TCP port number of the remote device
    const QString timeout           ; ///< Setting key for connection timeout (milliseconds)
    const QString maxconn           ; ///< Setting key for the maximum number of simultaneous connections to the server
    const QString serialPortName    ; ///< Setting key for the serial port name
    const QString baudRate          ; ///< Setting key for the serial port's baud rate
    const QString dataBits          ; ///< Setting key for the serial port's data bits
    const QString parity            ; ///< Setting key for the serial port's parity
    const QString stopBits          ; ///< Setting key for the serial port's stop bits
    const QString flowControl       ; ///< Setting key for the serial port's flow control
    const QString timeoutFirstByte  ; ///< Setting key for the serial port's timeout waiting first byte of packet
    const QString timeoutInterByte  ; ///< Setting key for the serial port's timeout waiting next byte of packet
    const QString isBroadcastEnabled; ///< Setting key for the serial port enables broadcast mode for `0` unit address

    const QString NoParity          ; ///< String constant for repr of `NoParity` enum value
    const QString EvenParity        ; ///< String constant for repr of `EvenParity` enum value
    const QString OddParity         ; ///< String constant for repr of `OddParity` enum value
    const QString SpaceParity       ; ///< String constant for repr of `SpaceParity` enum value
    const QString MarkParity        ; ///< String constant for repr of `MarkParity` enum value

    const QString OneStop           ; ///< String constant for repr of `OneStop` enum value
    const QString OneAndHalfStop    ; ///< String constant for repr of `OneAndHalfStop` enum value
    const QString TwoStop           ; ///< String constant for repr of `TwoStop` enum value

    const QString NoFlowControl     ; ///< String constant for repr of `NoFlowControl` enum value
    const QString HardwareControl   ; ///< String constant for repr of `HardwareControl` enum value
    const QString SoftwareControl   ; ///< String constant for repr of `SoftwareControl` enum value

    /// \details Constructor ot the class.
    Strings();

    /// \details Returns a reference to the global `Modbus::Strings` object.
    static const Strings &instance();
};

/*! \brief Holds the default values of the settings.
*/
class MODBUS_EXPORT Defaults
{
public:
    const uint8_t      unit              ; ///< Default value for the unit number of remote device
    const ProtocolType type              ; ///< Default value for the type of Modbus protocol
    const uint32_t     tries             ; ///< Default value for number of tries a Modbus request is repeated if it fails
    const QString      host              ; ///< Default value for the IP address or DNS name of the remote device
    const uint16_t     port              ; ///< Default value for the TCP port number of the remote device
    const uint32_t     timeout           ; ///< Default value for connection timeout (milliseconds)
    const uint32_t     maxconn           ; ///< Default value for the maximum number of simultaneous connections to the server
    const QString      serialPortName    ; ///< Default value for the serial port name
    const int32_t      baudRate          ; ///< Default value for the serial port's baud rate
    const int8_t       dataBits          ; ///< Default value for the serial port's data bits
    const Parity       parity            ; ///< Default value for the serial port's parity
    const StopBits     stopBits          ; ///< Default value for the serial port's stop bits
    const FlowControl  flowControl       ; ///< Default value for the serial port's flow control
    const uint32_t     timeoutFirstByte  ; ///< Default value for the serial port's timeout waiting first byte of packet
    const uint32_t     timeoutInterByte  ; ///< Default value for the serial port's timeout waiting next byte of packet
    const bool         isBroadcastEnabled; ///< Default value for the serial port enables broadcast mode for `0` unit address

    /// \details Constructor ot the class.
    Defaults();

    /// \details Returns a reference to the global `Modbus::Defaults` object.
    static const Defaults &instance();
};

/// \details Get settings value for the unit number of remote device.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint8_t getSettingUnit(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the type of Modbus protocol.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT ProtocolType getSettingType(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for number of tries a Modbus request is repeated if it fails.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint32_t getSettingTries(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the IP address or DNS name of the remote device.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT QString getSettingHost(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the TCP port of the remote device.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint16_t getSettingPort(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for connection timeout (milliseconds).
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint32_t getSettingTimeout(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the maximum number of simultaneous connections to the server.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint32_t getSettingMaxconn(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port name.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT QString getSettingSerialPortName(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's baud rate.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT int32_t getSettingBaudRate(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's data bits.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT int8_t getSettingDataBits(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's parity.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT Parity getSettingParity(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's stop bits.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT StopBits getSettingStopBits(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's flow control.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT FlowControl getSettingFlowControl(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's timeout waiting first byte of packet.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint32_t getSettingTimeoutFirstByte(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port's timeout waiting next byte of packet.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT uint32_t getSettingTimeoutInterByte(const Settings &s, bool *ok = nullptr);

/// \details Get settings value for the serial port enables broadcast mode for `0` unit address.
/// If value can't be retrieved that default value is returned and *ok = false (if provided).
MODBUS_EXPORT bool getSettingBroadcastEnabled(const Settings &s, bool *ok = nullptr);

/// \details Set settings value for the unit number of remote device.
MODBUS_EXPORT void setSettingUnit(Settings &s, uint8_t v);

/// \details Set settings value the type of Modbus protocol.
MODBUS_EXPORT void setSettingType(Settings &s, ProtocolType v);

/// \details Set settings value for number of tries a Modbus request is repeated if it fails.
MODBUS_EXPORT void setSettingTries(Settings &s, uint32_t);

/// \details Set settings value for the IP address or DNS name of the remote device.
MODBUS_EXPORT void setSettingHost(Settings &s, const QString &v);

/// \details Set settings value for the TCP port number of the remote device.
MODBUS_EXPORT void setSettingPort(Settings &s, uint16_t v);

/// \details Set settings value for connection timeout (milliseconds).
MODBUS_EXPORT void setSettingTimeout(Settings &s, uint32_t v);

/// \details Set settings value for maximum number of simultaneous connections to the server.
MODBUS_EXPORT void setSettingMaxconn(Settings &s, uint32_t v);

/// \details Set settings value for the serial port name.
MODBUS_EXPORT void setSettingSerialPortName(Settings &s, const QString&v);

/// \details Set settings value for the serial port's baud rate.
MODBUS_EXPORT void setSettingBaudRate(Settings &s, int32_t v);

/// \details Set settings value for the serial port's data bits.
MODBUS_EXPORT void setSettingDataBits(Settings &s, int8_t v);

/// \details Set settings value for the serial port's parity.
MODBUS_EXPORT void setSettingParity(Settings &s, Parity v);

/// \details Set settings value for the serial port's stop bits.
MODBUS_EXPORT void setSettingStopBits(Settings &s, StopBits v);

/// \details Set settings value for the serial port's flow control.
MODBUS_EXPORT void setSettingFlowControl(Settings &s, FlowControl v);

/// \details Set settings value for the serial port's timeout waiting first byte of packet.
MODBUS_EXPORT void setSettingTimeoutFirstByte(Settings &s, uint32_t v);

/// \details Set settings value for the serial port's timeout waiting next byte of packet.
MODBUS_EXPORT void setSettingTimeoutInterByte(Settings &s, uint32_t v);

/// \details Set settings value for the serial port enables broadcast mode for `0` unit address.
MODBUS_EXPORT void setSettingBroadcastEnabled(Settings &s, bool v);

/// \details Convert String repr to Modbus::Address
inline Address addressFromQString(const QString &s) { return Address::fromString(s); }

/// \details Convert value to QString key for type
template <class EnumType>
inline QString enumKey(int value)
{
    const QMetaEnum me = QMetaEnum::fromType<EnumType>();
    return QString(me.valueToKey(value));
}

/// \details Convert value to QString key for type
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

/// \details Convert key to value for enumeration by QString key
template <class EnumType>
inline EnumType enumValue(const QString& key, bool* ok = nullptr, EnumType defaultValue = static_cast<EnumType>(-1))
{
    bool okInner;
    const QMetaEnum me = QMetaEnum::fromType<EnumType>();
    EnumType v = static_cast<EnumType>(me.keyToValue(key.toLatin1().constData(), &okInner));
    if (ok)
        *ok = okInner;
    if (okInner)
        return v;
    return defaultValue;
}

/// \details Convert `QVariant` value to enumeration value (int - value, string - key).
/// Stores result of convertion in output parameter `ok`.
/// If `value` can't be converted, `defaultValue` is returned.
template <class EnumType>
inline EnumType enumValue(const QVariant& value, bool *ok = nullptr, EnumType defaultValue = static_cast<EnumType>(-1))
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
        return defaultValue;
    }
    return enumValue<EnumType>(value.toString(), ok, defaultValue);
}

/// \details Convert `QVariant` value to enumeration value (int - value, string - key).
/// If `value` can't be converted, `defaultValue` is returned.
template <class EnumType>
inline EnumType enumValue(const QVariant& value, EnumType defaultValue)
{
    return enumValue<EnumType>(value, nullptr, defaultValue);
}

/// \details Convert `QVariant` value to enumeration value (int - value, string - key).
template <class EnumType>
inline EnumType enumValue(const QVariant& value)
{
    return enumValue<EnumType>(value, nullptr);
}

/// \details Converts string representation to `ProtocolType` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT ProtocolType toProtocolType(const QString &s, bool *ok = nullptr);

/// \details Converts QVariant value to `ProtocolType` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT ProtocolType toProtocolType(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `BaudRate` value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT int32_t toBaudRate(const QString &s, bool *ok = nullptr);

/// \details Converts QVariant value to `DataBits` value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT int32_t toBaudRate(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `DataBits` value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT int8_t toDataBits(const QString &s, bool *ok = nullptr);

/// \details Converts QVariant value to `DataBits` value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT int8_t toDataBits(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `Parity` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT Parity toParity(const QString &s, bool *ok = nullptr);

/// \details Converts QVariant value to `Parity` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT Parity toParity(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `StopBits` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT StopBits toStopBits(const QString &s, bool *ok = nullptr);

/// \details Converts QVariant value to `StopBits` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT StopBits toStopBits(const QVariant &v, bool *ok = nullptr);

/// \details Converts string representation to `FlowControl` enum value.
/// If ok is not nullptr, failure is reported by setting *ok to false, and success by setting *ok to true.
MODBUS_EXPORT FlowControl toFlowControl(const QString &s, bool *ok = nullptr);

/// \details Converts QVariant value to `FlowControl` enum value.
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

/// \details Make string representation of bytes array and separate bytes by space
inline QString bytesToString(const QByteArray &v) { return bytesToString(reinterpret_cast<const uint8_t*>(v.constData()), v.size()).data(); }

/// \details Make string representation of ASCII array and separate bytes by space
inline QString asciiToString(const QByteArray &v) { return asciiToString(reinterpret_cast<const uint8_t*>(v.constData()), v.size()).data(); }

/// \details Returns list of string that represent names of serial ports
MODBUS_EXPORT QStringList availableSerialPortList();

/// \details Same as `Modbus::createPort(ProtocolType type, const void *settings, bool blocking)`
/// but `ProtocolType type` and `const void *settings` are defined by `Modbus::Settings` key-value map.
MODBUS_EXPORT ModbusPort *createPort(const Settings &settings, bool blocking = false);

/// \details Same as `Modbus::createClientPort(ProtocolType type, const void *settings, bool blocking)`
/// but `ProtocolType type` and `const void *settings` are defined by `Modbus::Settings` key-value map.
MODBUS_EXPORT ModbusClientPort *createClientPort(const Settings &settings, bool blocking = false);

/// \details Same as `Modbus::createServerPort(ProtocolType type, const void *settings, bool blocking)`
/// but `ProtocolType type` and `const void *settings` are defined by `Modbus::Settings` key-value map.
MODBUS_EXPORT ModbusServerPort *createServerPort(ModbusInterface *device, const Settings &settings, bool blocking = false);

} // namespace Modbus

#endif // MODBUSQT_H
