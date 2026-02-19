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

    \details 

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
