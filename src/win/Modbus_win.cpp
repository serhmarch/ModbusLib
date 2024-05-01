#include "../Modbus.h"

#include <Windows.h>

namespace Modbus {

void msleep(uint32_t msec)
{
    timeBeginPeriod(1);
    Sleep(msec);
    timeEndPeriod(1);
}

} // namespace Modbus
