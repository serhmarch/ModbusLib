/*!
 * \file   ModbusClientPort.h
 * \brief General file of the algorithm of the client part of the Modbus protocol port.
 *
 * \author march
 * \date   April 2024
 */
#ifndef MODBUSCLIENTPORT_H
#define MODBUSCLIENTPORT_H

#include "ModbusObject.h"

class ModbusPort;

/*! \brief The `ModbusClientPort` class implements the algorithm of the client part of the Modbus communication protocol port.

    \details `ModbusClient` contains a list of Modbus functions that are implemented by the Modbus client program.
    It implements functions for reading and writing various types of Modbus memory defined by the specification.
    The operations that return `Modbus::StatusCode` are asynchronous, that is, if the operation is not completed, it returns the intermediate status `Modbus::Status_Processing`,
    and then it must be called until it is successfully completed or returns an error status.

 */

class MODBUS_EXPORT ModbusClientPort : public ModbusObject
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
    ModbusClientPort(ModbusPort *port);

    /// \details Destructor of the class.
    ~ModbusClientPort();

public:
    /// \details Returns type of Modbus protocol.
    Modbus::Type type() const;

    /// \details Closes connection and returns status of the operation.
    Modbus::StatusCode close();

    /// \details Returns `true` if the connection with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Returns the setting of the number of repetitions of the Modbus request if it fails.
    uint32_t repeatCount() const;

    /// \details Sets the number of times a Modbus request is repeated if it fails.
    void setRepeatCount(uint32_t v);

public:
    /// \details Returns a pointer to the port object that uses this algorithm.
    ModbusPort *port() const;

public:
    /// \details Returns the status of the last operation performed.
    Modbus::StatusCode lastStatus() const;

    /// \details Returns the text of the last error of the performed operation.
    const Modbus::Char *lastErrorText() const;

public:
    /// \details The function performs a read/write request for a given algorithm. Returns the status of the last operation performed.
    Modbus::StatusCode request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff);

    /// \details The function implements the algorithm of this object. Returns the status of the last operation performed.
    Modbus::StatusCode process();

public:
    /// \cond
    struct RequestParams;
    /// \endcond
    
    /// \details Returns a pointer to the client object whose request is currently being processed by the algorithm.
    const ModbusObject *currentClient() const;

    /// \details Returns a pointer to the request structure for the client. `obj` - a pointer to the client, also known as a unique client identifier.\n
    /// The client usually calls this function in its own counterpart.
    RequestParams *createRequestParams(ModbusObject *object);

    /// \details Deletes the `*rp` request structure for the client.
    /// The client usually calls this function in its destructor.
    void deleteRequestParams(RequestParams *rp);

    /// \details Deletes the request structure `*rp` for the client.\n
    /// The client usually calls this function to determine whether its request is pending/finished/blocked.
    RequestStatus getRequestStatus(RequestParams *rp);

    /// \details Cancels the previous request specified by the `*rp` pointer for the client.
    void cancelRequest(RequestParams* rp);

public: // SIGNALS
    /// \details
    void signalOpened(const Modbus::Char *source);

    /// \details
    void signalClosed(const Modbus::Char *source);

    /// \details Calls each callback of the original packet 'Tx' from the internal list of callbacks, passing them the original array 'buff' and its size 'size'.
    void signalTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Calls each callback of the incoming packet 'Rx' from the internal list of callbacks, passing them the input array 'buff' and its size 'size'.
    void signalRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Calls each callback of the port when error is occured with error's status and text.
    void signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text);

};

#endif // MODBUSCLIENTPORT_H
