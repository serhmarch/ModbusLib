#include "../Modbus.h"

#include <time.h>

namespace Modbus {

void msleep(uint32_t msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

} // namespace Modbus
