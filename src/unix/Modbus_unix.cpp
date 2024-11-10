#include "Modbus_unix.h"

#include <time.h>
#include <dirent.h>
#include <cerrno>   // for errno
#include <cstring>
#include <set>

namespace Modbus {

Timer timer()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
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
