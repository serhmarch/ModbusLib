/*!
 * \file   ModbusPortTCP.h
 * \brief  Header file of class `Modbus::PortTCP`.
 *
 * \author march
 * \date   April 2024
 */
#ifndef MODBUSPORTTCP_H
#define MODBUSPORTTCP_H

#include "ModbusPort.h"

#define MBCLIENTTCP_BUFF_SZ MB_TCP_IO_BUFF_SZ

namespace Modbus {

class TCPSocket;

/*! \brief Class `Modbus::PortTCP` implements TCP version of Modbus protocol.

    \details `Modbus::Port` contains function to work with TCP-port (connection).

 */
class MODBUS_EXPORT PortTCP : public Port
{
public:
    struct MODBUS_EXPORT Defaults
    {
        const String   host   ;
        const uint16_t port   ;
        const uint32_t timeout;

        Defaults();
        static const Defaults &instance();
    };

public:
    /// \details Constructor of the class.
    PortTCP(TCPSocket *socket, bool blocking = false);

    /// \details Constructor of the class.
    PortTCP(bool synch = false);

    /// \details Destructor of the class.
    ~PortTCP();

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::TCP`.
    Type type() const override { return TCP; }
    Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;

public:
    ///  \details Returns the settings for the IP address or DNS name of the remote device.
    inline String host() const { return m_host; }

    ///  \details Sets the settings for the IP address or DNS name of the remote device.
    void setHost(const String &host);

    ///  \details Returns the setting for the TCP port number of the remote device.
    inline uint16_t port() const { return m_port; }

    ///  \details Sets the settings for the TCP port number of the remote device.
    void setPort(uint16_t port);

    ///  \details Returns the setting for the connection timeout of the remote device.
    inline uint32_t timeout() const { return m_timeout; }

    ///  \details Sets the setting for the connection timeout of the remote device.
    void setTimeout(uint32_t timeout);

    void setNextRequestRepeated(bool v) override;

    /// \details Returns `true' if the identifier of each subsequent parcel is automatically incremented by 1, `false' otherwise.
    inline bool autoIncrement() const { return m_autoIncrement; }

protected:
    StatusCode write() override;
    StatusCode read() override;
    StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) override;
    StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

private:
    void constructorPrivate(TCPSocket *socket);
    void destructorPrivate();

private:
    String m_host;
    uint16_t m_port;
    uint16_t m_transaction;
    bool m_autoIncrement;
    uint32_t m_timeout;
    uint8_t m_buff[MBCLIENTTCP_BUFF_SZ];
    uint16_t m_sz;

private: // Platform specific data
    struct PlatformData;
    PlatformData *m_platformData;
};

} // namespace Modbus

#endif // MODBUSPORTTCP_H
