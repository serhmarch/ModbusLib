#include "Modbus_unix.h"

#include <time.h>
#include <dirent.h>
#include <cerrno>   // for errno
#include <cstring>
#include <set>
#include <algorithm>
#include <iostream>

namespace Modbus {

Timer timer()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

Timestamp currentTimestamp()
{
    struct timespec ts;
    // Use CLOCK_REALTIME to get the current wall-clock time
    clock_gettime(CLOCK_REALTIME, &ts);

    // Convert seconds to milliseconds and add nanoseconds converted to milliseconds
    return static_cast<int64_t>(ts.tv_sec) * 1000 + static_cast<int64_t>(ts.tv_nsec) / 1000000;
}

void setConsoleColor(Color color)
{
    switch(color)
    {
    case Color_Black  : std::cout << "\033[30m"; break;
    case Color_Red    : std::cout << "\033[31m"; break;
    case Color_Green  : std::cout << "\033[32m"; break;
    case Color_Yellow : std::cout << "\033[33m"; break;
    case Color_Blue   : std::cout << "\033[34m"; break;
    case Color_Magenta: std::cout << "\033[35m"; break;
    case Color_Cyan   : std::cout << "\033[36m"; break; 
    case Color_White  : std::cout << "\033[37m"; break;
    case Color_Default:
    default:            std::cout << "\033[0m" ; break;
    }
}

void msleep(uint32_t msec)
{
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

String getLastErrorText()
{
    String message = std::strerror(errno);  // strerror converts errno to a readable error message
    message.erase(std::find_if(message.rbegin(), message.rend(), [](Char ch) {
                      return !std::isspace(ch);
                  }).base(), message.end());
    return message;
}

#define MB_DEV_ENTRY_IS_EQ(str) (std::strncmp(entry->d_name, str, sizeof(str)-1) == 0)

List<String> availableSerialPorts()
{
    std::set<String> portSet;

    DIR* dir = opendir("/dev");
    if (!dir)
        return List<String>();

    struct dirent* entry;
    while ((entry = readdir(dir)))
    {
        if (MB_DEV_ENTRY_IS_EQ("ttyS"  ) || // Standard UART 8250 and etc.
            MB_DEV_ENTRY_IS_EQ("ttyO"  ) || // OMAP UART 8250 and etc.
            MB_DEV_ENTRY_IS_EQ("ttyUSB") || // Usb/serial converters PL2303 and etc.
            MB_DEV_ENTRY_IS_EQ("ttyACM") || // CDC_ACM converters (i.e. Mobile Phones).
            MB_DEV_ENTRY_IS_EQ("ttyGS" ) || // Gadget serial device (i.e. Mobile Phones with gadget serial driver).
            MB_DEV_ENTRY_IS_EQ("ttyMI" ) || // MOXA pci/serial converters.
            MB_DEV_ENTRY_IS_EQ("ttymxc") || // Motorola IMX serial ports (i.e. Freescale i.MX).
            MB_DEV_ENTRY_IS_EQ("ttyAMA") || // AMBA serial device for embedded platform on ARM (i.e. Raspberry Pi).
            MB_DEV_ENTRY_IS_EQ("ttyTHS") || // Serial device for embedded platform on ARM (i.e. Tegra Jetson TK1).
            MB_DEV_ENTRY_IS_EQ("rfcomm") || // Bluetooth serial device.
            MB_DEV_ENTRY_IS_EQ("ircomm") || // IrDA serial device.
            MB_DEV_ENTRY_IS_EQ("tnt"   )  ) // Virtual tty0tty serial device.
            portSet.insert("/dev/" + String(entry->d_name));
    }

    closedir(dir);
    List<String> ports(portSet.begin(), portSet.end());
    return ports;
}

} // namespace Modbus
