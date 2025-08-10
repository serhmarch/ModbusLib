#ifndef MODBUSPORT_P_H
#define MODBUSPORT_P_H

#include "Modbus.h"

class ModbusPort;

namespace ModbusPortPrivateNS {

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

} // namespace ModbusPortPrivateNS

using namespace Modbus;
using namespace ModbusPortPrivateNS;

class ModbusPortPrivate
{
public:
    ModbusPortPrivate(uint16_t maxBuffSize, bool blocking)
    {
        this->state = STATE_UNKNOWN;
        this->modeServer = false;
        this->modeBlocking = blocking;
        this->clearChanged();
        this->buff = new uint8_t[maxBuffSize];
        this->c_buffSz = maxBuffSize;
        this->sz = 0;
    }

    virtual ~ModbusPortPrivate()
    {
        delete[] this->buff;
    }

public:
    inline bool isBlocking() const { return modeBlocking; }
    inline bool isNonBlocking() const { return !modeBlocking; }
    inline bool isStateClosed() const { return state == STATE_CLOSED; }
    inline void setChanged(bool changed) { this->changed = changed; }
    inline void clearChanged() { setChanged(false); }
    inline StatusCode lastErrorStatus() { return errorStatus; }
    inline const Char *lastErrorText() { return errorText.data(); }
    inline StatusCode setError(StatusCode status, const String &text) { errorStatus = status; errorText = text; return status; }
    inline StatusCode setError(StatusCode status, String &&text) { errorStatus = status; errorText = text; return status; }

public:
    virtual Modbus::Handle handle() const = 0;
    virtual Modbus::StatusCode open() = 0;
    virtual Modbus::StatusCode close() = 0;
    virtual bool isOpen() const = 0;
    virtual Modbus::StatusCode write() = 0;
    virtual Modbus::StatusCode read() = 0;
    virtual void setNextRequestRepeated(bool v) {}

public:
    State state;
    bool changed;
    bool modeServer;
    bool modeBlocking;
    StatusCode errorStatus;
    String errorText;

    // buffer
    uint8_t *buff;
    uint16_t c_buffSz;
    uint16_t sz;

    // settings
    struct
    {
        String hostOrPortName;

        union
        {
        uint16_t port;
        int32_t baudRate;
        };

        int8_t dataBits;
        Parity parity;
        StopBits stopBits;
        FlowControl flowControl;
        uint32_t timeout;
        uint32_t timeoutInterByte;
    } settings;
};

#endif // MODBUSPORT_P_H
