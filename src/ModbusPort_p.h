#ifndef MODBUSPORT_P_H
#define MODBUSPORT_P_H

#include "Modbus.h"

class ModbusPort;

namespace ModbusPortPrivateNS {

enum State
{
    STATE_UNKNOWN = 0,
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
        this->changed = false;
        this->modeServer = false;
        this->modeBlocking = blocking;
        this->clearChanged();
        this->buff = new uint8_t[maxBuffSize];
        this->c_buffSz = maxBuffSize;
        this->sz = 0;
        this->errorStatus = Modbus::Status_Uncertain;
        // this->settingsBase.timeout must be initialized in derived classes
    }

    virtual ~ModbusPortPrivate()
    {
        delete[] this->buff;
    }

public: //settings
    inline const String& host            () const { return settings.hostOrPortName  ; }
    inline auto          port            () const { return settings.port            ; }
    inline auto          timeout         () const { return settings.timeout         ; }
    inline const String& portName        () const { return settings.hostOrPortName  ; }
    inline auto          baudRate        () const { return settings.baudRate        ; }
    inline auto          dataBits        () const { return settings.dataBits        ; }
    inline auto          stopBits        () const { return settings.stopBits        ; }
    inline auto          parity          () const { return settings.parity          ; }
    inline auto          flowControl     () const { return settings.flowControl     ; }
    inline auto          timeoutFirstByte() const { return settings.timeout         ; }
    inline auto          timeoutInterByte() const { return settings.timeoutInterByte; }

    inline void setHost            (const String& v) { settings.hostOrPortName   = v; }
    inline void setPort            (uint16_t      v) { settings.port             = v; }
    inline void setTimeout         (uint32_t      v) { settings.timeout          = v; }
    inline void setPortName        (const String& v) { settings.hostOrPortName   = v; }
    inline void setBaudRate        (int32_t       v) { settings.baudRate         = v; }
    inline void setDataBits        (int8_t        v) { settings.dataBits         = v; }
    inline void setStopBits        (StopBits      v) { settings.stopBits         = v; }
    inline void setParity          (Parity        v) { settings.parity           = v; }
    inline void setFlowControl     (FlowControl   v) { settings.flowControl      = v; }
    inline void setTimeoutFirstByte(uint32_t      v) { settings.timeout          = v; }
    inline void setTimeoutInterByte(uint32_t      v) { settings.timeoutInterByte = v; }

    inline void setHost    (const Char* v) { settings.hostOrPortName = v; }
    inline void setPortName(const Char* v) { settings.hostOrPortName = v; }

public:
    inline bool isBlocking() const { return modeBlocking; }
    inline bool isNonBlocking() const { return !modeBlocking; }
    inline bool isStateClosed() const { return state == STATE_CLOSED; }
    inline bool isChanged() const { return changed; }
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
    virtual void setNextRequestRepeated(bool) {}

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
