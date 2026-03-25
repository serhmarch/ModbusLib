/*!
 * \class ModbusNetPort
 * \brief The abstract class `ModbusNetPort` is the base class for network-based implementations of the Modbus communication protocol.
 *
 * \details ModbusNetPort provides a standardized interface for Modbus communication over network transports such as TCP and UDP.
 * It extends the base ModbusPort class with network-specific functionality including host address and port number configuration.
 * This class serves as an abstract foundation that derived classes should implement to support specific network protocols
 * and connection mechanisms.
 *
 * The class manages essential network parameters required for establishing Modbus connections to remote devices.
 * Through the host() and setHost() methods, applications can specify target devices by IP address or DNS name.
 * The port() and setPort() methods allow configuration of the TCP/UDP port number used for communication.
 * These settings enable flexible network connectivity while maintaining a consistent interface across different network-based
 * Modbus implementations.
 *
 * Derived classes are expected to implement the actual network communication logic, including connection establishment,
 * data transmission, and error handling specific to their protocol (TCP, UDP, etc.). The ModbusNetPort class ensures
 * that all network-based Modbus implementations provide consistent configuration and behavior through a common interface.
 */
/*!
 * \file   ModbusNetPort.h
 * \brief  Header file of abstract class `ModbusNetPort`.
 *
 * \author serhmarch
 * \date   Feb 2026
 */
#ifndef MODBUSNETPORT_H
#define MODBUSNETPORT_H

#include <string>
#include <list>

#include "ModbusPort.h"

class ModbusNetPortPrivate;

/*! \brief The abstract class `ModbusNetPort` is the base class for a specific implementation of the Modbus communication protocol.

    \details ModbusNetPort provides a standardized interface for Modbus communication over network transports such as TCP and UDP.
    It extends the base ModbusPort class with network-specific functionality including host address and port number configuration.
    This class serves as an abstract foundation that derived classes should implement to support specific network protocols
    and connection mechanisms.
    
    The class manages essential network parameters required for establishing Modbus connections to remote devices.
    Through the host() and setHost() methods, applications can specify target devices by IP address or DNS name.
    The port() and setPort() methods allow configuration of the TCP/UDP port number used for communication.
    These settings enable flexible network connectivity while maintaining a consistent interface across different network-based
    Modbus implementations.
    
    Derived classes are expected to implement the actual network communication logic, including connection establishment,
    data transmission, and error handling specific to their protocol (TCP, UDP, etc.). The ModbusNetPort class ensures
    that all network-based Modbus implementations provide consistent configuration and behavior through a common interface.

 */
class MODBUS_EXPORT ModbusNetPort : public ModbusPort
{
public: // settings
    /// \details Returns the settings for the IP address or DNS name of the remote device.
    const Modbus::Char *host() const;

    /// \details Sets the settings for the IP address or DNS name of the remote device.
    void setHost(const Modbus::Char *host);

    /// \details Returns the setting for the TCP/UDP port number of the remote device.
    uint16_t port() const;

    /// \details Sets the settings for the TCP/UDP port number of the remote device.
    void setPort(uint16_t port);

protected:
    /// \cond
    using ModbusPort::ModbusPort;
    /// \endcond
};

#endif // MODBUSNETPORT_H
