#ifndef MODBUS_UNIX_H
#define MODBUS_UNIX_H

#include <ctime>

#include "../Modbus.h"

namespace Modbus {

inline Timer timer()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

} // namespace Modbus

#endif // MODBUS_UNIX_H
