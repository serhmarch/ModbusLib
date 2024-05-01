#ifndef MODBUS_SERVERRESOURCE_H
#define MODBUS_SERVERRESOURCE_H

#include <ModbusServerPort.h>

#define MBSERVER_SZ_VALUE_BUFF MB_VALUE_BUFF_SZ

namespace Modbus {

class Port;

class MODBUS_EXPORT ServerResource : public ServerPort
{
public:
    ServerResource(Port *port, Interface *device);
    ~ServerResource();

public:
    inline Port *port() const { return m_port; }

public: // server port interface
    Type type() const override;
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;
    StatusCode process() override;

protected:
    virtual StatusCode processInputData(const uint8_t *buff, uint16_t sz);
    virtual StatusCode processDevice();
    virtual StatusCode processOutputData(uint8_t *buff, uint16_t &sz);

protected:
    Port *m_port;
    uint8_t m_unit;
    uint8_t m_func;
    uint16_t m_offset;
    uint16_t m_count;
    uint8_t m_valueBuff[MBSERVER_SZ_VALUE_BUFF];
};

} // namespace Modbus

#endif // MODBUS_SERVERRESOURCE_H
