#include "../Modbus.h"

#include <Windows.h>

namespace Modbus {

void msleep(uint32_t msec)
{
    timeBeginPeriod(1);
    Sleep(msec);
    timeEndPeriod(1);
}

List<String> availableSerialPorts()
{
    const ULONG PortNumbersCount = 100;
    List<String> ports;

#if 0 //NTDDI_VERSION >= NTDDI_WIN10_RS4
    ULONG PortNumbers[PortNumbersCount];
    ULONG PortCount = 0;
    // TODO:
    GetCommPorts(PortNumbers, PortNumbersCount, &PortCount);
    for (ULONG portNumber : PortNumbers)
        ports.push_back(StringLiteral("COM")+std::to_string(portNumber));
#else
    for (ULONG i = 1; i <= PortNumbersCount; i++)
    {
        String portName = StringLiteral("COM")+std::to_string(i);
        HANDLE serialPort = CreateFileA(
            portName.data(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (serialPort != INVALID_HANDLE_VALUE)
        {
            ports.push_back(portName);
            CloseHandle(serialPort);
        }
    }
#endif
    return ports;
}

} // namespace Modbus
