#include "../Modbus.h"

#include <vector>

#include <Windows.h>
#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

namespace Modbus {

Timer timer()
{
    return GetTickCount();
}

void msleep(uint32_t msec)
{
    timeBeginPeriod(1);
    Sleep(msec);
    timeEndPeriod(1);
    /*
    // Create a waitable timer
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer == NULL)
        return; // Handle error if needed
    // Set the timer to the specified interval
    LARGE_INTEGER li;
    li.QuadPart = -(static_cast<LONGLONG>(msec) * 10000); // Convert milliseconds to 100-nanosecond intervals
    if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE))
    {
        CloseHandle(timer);
        return; // Handle error if needed
    }
    // Wait for the timer
    WaitForSingleObject(timer, INFINITE);
    // Clean up
    CloseHandle(timer);
    */
}

List<String> availableSerialPorts()
{
    List<String> ports;

#if 0 //NTDDI_VERSION >= NTDDI_WIN10_RS4
    const ULONG PortNumbersCount = 100;
    ULONG PortNumbers[PortNumbersCount];
    ULONG PortCount = 0;
    // TODO:
    GetCommPorts(PortNumbers, PortNumbersCount, &PortCount);
    for (ULONG i = 0; i < PortCount; i++)
        ports.push_back(StringLiteral("COM")+std::to_string(PortNumbers[i]));
#else
//    for (ULONG i = 1; i <= PortNumbersCount; i++)
//    {
//        String portName = StringLiteral("COM")+std::to_string(i);
//        HANDLE serialPort = CreateFileA(
//            portName.data(),
//            GENERIC_READ | GENERIC_WRITE,
//            0,
//            NULL,
//            OPEN_EXISTING,
//            0,
//            NULL);
//        if (serialPort != INVALID_HANDLE_VALUE)
//        {
//            ports.push_back(portName);
//            CloseHandle(serialPort);
//        }
//    }
    HDEVINFO hDevInfo = SetupDiGetClassDevs(
        &GUID_DEVINTERFACE_COMPORT, // Ports class GUID
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE)
        return ports;

    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    DWORD i = 0;
    while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_COMPORT, i, &DeviceInterfaceData))
    {
        DWORD requiredSize = 0;

        // First, get the size of the buffer needed
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, NULL, 0, &requiredSize, NULL);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            break;

        std::vector<BYTE> buffer(requiredSize);
        SP_DEVICE_INTERFACE_DETAIL_DATA* DeviceInterfaceDetailData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(&buffer[0]);
        DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        SP_DEVINFO_DATA DeviceInfoData;
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        // Now, get the device interface detail
        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, DeviceInterfaceDetailData, requiredSize, NULL, &DeviceInfoData))
            break;

        // Get the friendly name of the device
        char buffer2[256];
        if (SetupDiGetDeviceRegistryPropertyA(
                hDevInfo,
                &DeviceInfoData,
                SPDRP_FRIENDLYNAME,
                NULL,
                (PBYTE)buffer2,
                sizeof(buffer2),
                NULL))
        {
            std::string deviceName = buffer2;

            // Check if the device name contains "COM"
            size_t pos = deviceName.find("(COM");
            if (pos != std::string::npos)
            {
                // Extract the COM port number from the device name
                size_t endPos = deviceName.find(")", pos);
                if (endPos != std::string::npos)
                {
                    std::string comPort = deviceName.substr(pos + 1, endPos - pos - 1);
                    ports.push_back(comPort);
                }
            }
        }

        i++;
    }
#endif
    return ports;
}

} // namespace Modbus
