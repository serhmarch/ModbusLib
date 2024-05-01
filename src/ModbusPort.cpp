#include "ModbusPort.h"

namespace Modbus {

Port::Port(bool blocking)
{
    m_state = STATE_UNKNOWN;
    m_block = false;;
    m_unit = 0;
    m_func = 0;
    m_modeServer = false;
    m_modeSynch = blocking;
    clearChanged();
}

Port::~Port()
{
}

void Port::setNextRequestRepeated(bool /* v */)
{
}

void Port::setServerMode(bool mode)
{
    m_modeServer = mode;
}

} // namespace Modbus
