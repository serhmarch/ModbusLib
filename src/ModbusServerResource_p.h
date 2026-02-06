#ifndef MODBUSSERVERRESOURCE_P_H
#define MODBUSSERVERRESOURCE_P_H

#include "ModbusPort.h"
#include "ModbusServerPort_p.h"

#define MBSERVER_SZ_VALUE_BUFF MB_VALUE_BUFF_SZ

class ModbusServerResourcePrivate : public ModbusServerPortPrivate
{
public:
    ModbusServerResourcePrivate(ModbusPort *port, ModbusInterface *device) :
        ModbusServerPortPrivate(device)
    {
        this->port = port;
        setPortError(port->lastErrorStatus());
        port->setServerMode(true);
    }

    ~ModbusServerResourcePrivate()
    {
        delete this->port;
    }

public:
    inline bool isBroadcast() const { return (unit == 0) && isBroadcastEnabled(); }
    inline StatusCode lastPortErrorStatus() const { return port->lastErrorStatus(); }
    inline const Char *lastPortErrorText() const { return port->lastErrorText(); }
    inline const Char *getLastErrorText() const
    {
        if (isLastPortError)
            return lastPortErrorText();
        return lastErrorText.data();
    }

    inline StatusCode setError(StatusCode status, const Char *text)
    {
        isLastPortError = false;
        return setErrorBase(status, text);
    }

    inline StatusCode setError(StatusCode status, const String &text)
    {
        return setError(status, text.data());
    }

    inline StatusCode setPortError(StatusCode status)
    {
        lastErrorStatus = status;
        isLastPortError = true;
        return status;
    }

public:
    ModbusPort *port;
    uint8_t unit;
    uint8_t func;

    union {
    uint16_t offset;
    uint16_t subfunc;
    uint16_t status;
    };

    union {
    uint16_t count;
    uint8_t byteCount;
    };

    union  {
    uint16_t messageCount;
    uint16_t andMask;
    uint16_t writeOffset;
    };
    
    union  {
    uint16_t orMask;
    uint16_t writeCount;
    uint8_t outByteCount;
    };

    uint8_t valueBuff[MBSERVER_SZ_VALUE_BUFF];
    bool isLastPortError;

};

#endif // MODBUSSERVERRESOURCE_P_H
