/*!
 * \file   ModbusPort.h
 * \brief  Header file of abstract class `Modbus::Port`.
 *
 * \author march
 * \date   April 2024
 */
#ifndef MODBUSPORT_H
#define MODBUSPORT_H

#include <string>
#include <list>

#include "Modbus.h"
#include "ModbusCallback.h"

namespace Modbus {

/*! \brief The abstract class `Modbus::Port` is the base class for a specific implementation of the Modbus communication protocol.

    \details `Modbus::Port` contains general functions for working with a specific port, implementing a specific version of the Modbus communication protocol.
    For example, versions for working with a TCP port or a serial port.

 */
class MODBUS_EXPORT Port : public CallbackSignals
{
protected:
    /// \cond
    enum State
    {
        STATE_BEGIN = 0,
        STATE_UNKNOWN = STATE_BEGIN,
        STATE_WAIT_FOR_OPEN,
        STATE_OPENED,
        STATE_PREPARE_TO_READ,
        STATE_WAIT_FOR_READ,
        STATE_WAIT_FOR_READ_ALL,
        STATE_PREPARE_TO_WRITE,
        STATE_WAIT_FOR_WRITE,
        STATE_WAIT_FOR_WRITE_ALL,
        STATE_WAIT_FOR_CLOSE,
        STATE_CLOSED,
        STATE_END = STATE_CLOSED
    };
    /// \endcond

public:
    /// \details Constructor of the class.
    explicit Port(bool blocking = false);

    /// \details Destructor of the class.
    virtual ~Port();

public:
    /// \details Returns the Modbus protocol type.
    virtual Type type() const = 0;

    /// \details Returns the native handle value that depenp on OS used. For TCP it socket handle, for serial port - file handle.
    virtual Handle handle() const = 0;

    /// \details Opens port (create connection) for further operations and returns the result status.
    virtual StatusCode open() = 0;

    /// \details Closes the port (breaks the connection) and returns the status the result status.
    virtual StatusCode close() = 0;

    /// \details Returns `true` if the port is open/communication with the remote device is established, `false` otherwise.
    virtual bool isOpen() const = 0;

    /// \details For the TCP version of the Modbus protocol. The identifier of each subsequent parcel is automatically increased by 1.
    /// If you set `setNextRequestRepeated(true)` then the next ID will not be increased by 1 but for only one next parcel.
    virtual void setNextRequestRepeated(bool v);

public:
    /// \details Returns `true` if the port settings have been changed and the port needs to be reopened/reestablished communication with the remote device, `false` otherwise.
    inline bool isChanged() const { return m_changed; }

    /// \details Returns `true` if the port works in server mode, `false` otherwise.
    inline bool isServerMode() const { return m_modeServer; }

    /// \details Sets server mode if `true`, `false` for client mode.
    virtual void setServerMode(bool mode);

    /// \details Returns `true` if the port works in synch (blocking) mode, `false` otherwise.
    inline bool isBlocking() const { return m_modeSynch; }

    /// \details Returns `true` if the port works in asynch (nonblocking) mode, `false` otherwise.
    inline bool isNonBlocking() const { return !m_modeSynch; }

public: // errors
    /// \details Returns the text of the last error of the performed operation.
    inline String lastErrorText() const { return m_lastErrorText; }

    /// \details Returns the pointer to `const Char` text buffer of the last error of the performed operation.
    inline const Char *lastErrorTextData() const { return m_lastErrorText.data(); }

    /// \details Sets the error parameters of the last operation performed.
    inline StatusCode setError(StatusCode status, const String &text) { m_lastErrorText = text; return status; }

public:
    /// \details Returns `true` if the port is blocked - it has already received a request and is processing it, `false` otherwise.
    inline bool isWriteBufferBlocked() const { return m_block; }

    /// \details Unblocks the port and makes it ready to receive the next request.
    inline void freeWriteBuffer() { m_block = false; }

    /// \details The function directly generates a packet and places it in the buffer for further sending. Returns the status of the operation.
    virtual StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) = 0;

    /// \details The function parses the packet that the `read()` function puts into the buffer, checks it for correctness, extracts its parameters, and returns the status of the operation.
    virtual StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) = 0;
    
    /// \details Implements the algorithm for writing to the port and returns the status of the operation.
    virtual StatusCode write() = 0;

    /// \details Implements the algorithm for reading from the port and returns the status of the operation.
    virtual StatusCode read() = 0;

public: // SIGNALS
    /// \details Calls each callback of the original packet 'Tx' from the internal list of callbacks, passing them the original array 'buff' and its size 'size'.
    void emitTx(const uint8_t* buff, uint16_t size) { emitSignal(&Port::emitTx, buff, size); }

    /// \details Calls each callback of the incoming packet 'Rx' from the internal list of callbacks, passing them the input array 'buff' and its size 'size'.
    void emitRx(const uint8_t* buff, uint16_t size) { emitSignal(&Port::emitRx, buff, size); }

protected:
    /// \details If `setChanged(true)` - sets the sign that the port settings have been changed and it is necessary to reopen the port/reestablish communication with the remote device.
    /// If `setChanged(false)` - clears this sign.
    inline void setChanged(bool changed = true) { m_changed = changed; }

    /// \details The same as `setChanged(false)`.
    inline void clearChanged() { setChanged(false); }

protected:
    State m_state;
    bool m_block;
    uint8_t m_unit;
    uint8_t m_func;
    bool m_changed;
    bool m_modeServer;
    bool m_modeSynch;
    String m_lastErrorText;
};

} // namespace Modbus

#endif // MODBUSPORT_H
