/*!
 * \file   ModbusClientPort.h
 * \brief General file of the algorithm of the client part of the Modbus protocol port.
 *
 * \author march
 * \date   April 2024
 */
#ifndef MODBUSCLIENTPORT_H
#define MODBUSCLIENTPORT_H

#include "ModbusPort.h"

namespace Modbus {

class Port;

/*! \brief The `Modbus::ClientPort` class implements the algorithm of the client part of the Modbus communication protocol port.

    \details `Modbus::Client` contains a list of Modbus functions that are implemented by the Modbus client program.
    It implements functions for reading and writing various types of Modbus memory defined by the specification.
    The operations that return `Modbus::StatusCode` are asynchronous, that is, if the operation is not completed, it returns the intermediate status `Modbus::Status_Processing`,
    and then it must be called until it is successfully completed or returns an error status.

 */
class MODBUS_EXPORT ClientPort
{
public:
    /*! \brief Sets the status of the request for the client.
     */
    enum RequestStatus
    {
        Enable,
        Disable,
        Process
    };

public:
    /// \details Constructor of the class.
    /// \param[in]  port A pointer to the port object to which this client object belongs.
    ClientPort(Port *port);

    /// \details Destructor of the class.
    ~ClientPort();

public:
    /// \details Returns type of Modbus protocol.
    Type type() const;

    /// \details Closes connection and returns status of the operation.
    StatusCode close();

    /// \details Returns `true` if the connection with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Returns the setting of the number of repetitions of the Modbus request if it fails.
    uint32_t repeatCount() const { return m_settings.repeatCount; }

    /// \details Sets the number of times a Modbus request is repeated if it fails.
    void setRepeatCount(uint32_t v) { if (v > 0) m_settings.repeatCount = v; }

public:
    /// \details Returns a pointer to the port object that uses this algorithm.
    inline Port *port() const { return m_port; }

public:
    /// \details Returns the status of the last operation performed.
    inline StatusCode lastStatus() const { return m_lastStatus; }

    /// \details Returns the text of the last error of the performed operation.
    inline String lastErrorText() const { return m_port->lastErrorText(); }

public:
    /// \details The function performs a read/write request for a given algorithm. Returns the status of the last operation performed.
    StatusCode request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff);

    /// \details The function implements the algorithm of this object. Returns the status of the last operation performed.
    StatusCode process();

public:
    /// \cond
    struct RequestParams;
    /// \endcond
    
    /// \details Returns a pointer to the client object whose request is currently being processed by the algorithm.
    inline const void *currentClient() const { return m_currentClient; }

    /// \details Returns a pointer to the request structure for the client. `obj` - a pointer to the client, also known as a unique client identifier.\n
    /// The client usually calls this function in its own counterpart.
    RequestParams *createRequestParams(void *obj);

    /// \details Deletes the `*rp` request structure for the client.
    /// The client usually calls this function in its destructor.
    void deleteRequestParams(RequestParams *rp);

    /// \details Deletes the request structure `*rp` for the client.\n
    /// The client usually calls this function to determine whether its request is pending/finished/blocked.
    RequestStatus getRequestStatus(RequestParams *rp);

    /// \details Cancels the previous request specified by the `*rp` pointer for the client.
    void cancelRequest(RequestParams* rp);

/// \cond
protected:
    inline void setStatus(StatusCode s) { m_lastStatus = s; }

protected:
    enum State
    {
        STATE_BEGIN = 0,
        STATE_UNKNOWN = STATE_BEGIN,
        STATE_WAIT_FOR_OPEN,
        STATE_OPENED,
        STATE_BEGIN_WRITE,
        STATE_WRITE,
        STATE_BEGIN_READ,
        STATE_READ,
        STATE_WAIT_FOR_CLOSE,
        STATE_CLOSED,
        STATE_END = STATE_CLOSED
    };

protected:
    Port *m_port;
    State m_state;
    void *m_currentClient;
    uint32_t m_repeats;
    StatusCode m_lastStatus;

    struct
    {
        uint32_t repeatCount;
    } m_settings;

/// \endcond

};

} // namespace Modbus

#endif // MODBUSCLIENTPORT_H
