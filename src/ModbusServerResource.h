#ifndef MODBUSSERVERRESOURCE_H
#define MODBUSSERVERRESOURCE_H

#include <ModbusServerPort.h>

class ModbusPort;

class MODBUS_EXPORT ModbusServerResource : public ModbusServerPort
{
public:
    ModbusServerResource(ModbusPort *port, ModbusInterface *device);

public:
    ModbusPort *port() const;

public: // server port interface
    Modbus::Type type() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;
    Modbus::StatusCode process() override;

protected:
    virtual Modbus::StatusCode processInputData(const uint8_t *buff, uint16_t sz);
    virtual Modbus::StatusCode processDevice();
    virtual Modbus::StatusCode processOutputData(uint8_t *buff, uint16_t &sz);

protected:
    using ModbusServerPort::ModbusServerPort;
};

#endif // MODBUSSERVERRESOURCE_H
