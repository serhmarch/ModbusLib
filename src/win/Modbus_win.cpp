#include "../Modbus.h"

#include <vector>

#include <Windows.h>
#include <initguid.h>
#include <devguid.h>
#include <ntddmodm.h>
#include <setupapi.h>

#define NANOSEC100_IN_MILLISEC      10000
#define SEC_BETWEEN_1601_AND_1970   11644473600LL
#define MSEC_BETWEEN_1601_AND_1970  ((SEC_BETWEEN_1601_AND_1970)*1000)

namespace Modbus {

Timer timer()
{
    return GetTickCount();
}

Timestamp currentTimestamp()
{
    Timestamp ft;
    GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&ft));
    return ft / NANOSEC100_IN_MILLISEC - MSEC_BETWEEN_1601_AND_1970;
}

void setConsoleColor(Color color)
{
    switch(color)
    {
    case Color_Black  : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0); break;
    case Color_Red    : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED); break;
    case Color_Green  : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN); break;
    case Color_Yellow : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN); break;
    case Color_Blue   : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE); break;
    case Color_Magenta: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_BLUE); break;
    case Color_Cyan   : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_BLUE); break;
    case Color_White  :
    case Color_Default:
    default:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE); break;
    }
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

String getLastErrorText()
{
    // Retrieve the error code from the last WinAPI call
    DWORD errorMessageID = ::GetLastError();

    if (errorMessageID == 0)
    {
        // No error, return an empty string
        return String();
    }

    LPWSTR messageBuffer = nullptr;

    // Format the error message from the error code
    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorMessageID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&messageBuffer,
        0,
        NULL
        );

    if (size == 0)
        return StringLiteral("Unknown error"); // Failed to retrieve error message

    // Convert UTF-16 to UTF-8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, messageBuffer, size, NULL, 0, NULL, NULL);
    String errorMsg(utf8Size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, messageBuffer, size, &errorMsg[0], utf8Size, NULL, NULL);

    LocalFree(messageBuffer); // Free allocated memory

    return errorMsg;
}

// Note: Next list of functions are adapted from qtserialport project (qserialportinfo_win.cpp):
// * portNamesFromHardwareDeviceMap()
// * devicePortName()
// * availableSerialPorts()

static List<String> portNamesFromHardwareDeviceMap()
{
    List<String> result;

    HKEY hKey = nullptr;
    if (::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return result;

    DWORD index = 0;

    // This is a maximum length of value name, see:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872%28v=vs.85%29.aspx
    enum { MaximumValueNameInChars = 16383 };

    std::vector<char> outputValueName(MaximumValueNameInChars, 0);
    std::vector<char> outputBuffer(MAX_PATH + 1, 0);
    DWORD bytesRequired = MAX_PATH;
    for (;;)
    {
        DWORD requiredValueNameChars = MaximumValueNameInChars;
        const LONG ret = ::RegEnumValueA(hKey, index, &outputValueName[0], &requiredValueNameChars,
                                         nullptr, nullptr, reinterpret_cast<PBYTE>(&outputBuffer[0]), &bytesRequired);
        if (ret == ERROR_MORE_DATA)
        {
            outputBuffer.resize(bytesRequired+1, 0);
        }
        else if (ret == ERROR_SUCCESS)
        {
            result.push_back(outputBuffer.data());
            ++index;
        }
        else
        {
            break;
        }
    }
    ::RegCloseKey(hKey);
    return result;
}

static String devicePortName(HDEVINFO deviceInfoSet, PSP_DEVINFO_DATA deviceInfoData)
{
    const HKEY key = ::SetupDiOpenDevRegKey(deviceInfoSet, deviceInfoData, DICS_FLAG_GLOBAL,
                                            0, DIREG_DEV, KEY_READ);
    if (key == INVALID_HANDLE_VALUE)
        return String();

    static const char * const keyTokens[] = {
        "PortName\0",
        "PortNumber\0"
    };

    String portName;
    for (auto keyToken : keyTokens)
    {
        DWORD dataType = 0;
        std::vector<char> outputBuffer(MAX_PATH + 1, 0);
        DWORD bytesRequired = MAX_PATH;
        for (;;)
        {
            const LONG ret = ::RegQueryValueExA(key, keyToken, nullptr, &dataType,
                                                reinterpret_cast<PBYTE>(&outputBuffer[0]), &bytesRequired);
            if (ret == ERROR_MORE_DATA)
            {
                outputBuffer.resize(bytesRequired + 1, 0);
                continue;
            }
            else if (ret == ERROR_SUCCESS)
            {
                if (dataType == REG_SZ)
                    portName = outputBuffer.data();
                else if (dataType == REG_DWORD)
                    portName = StringLiteral("COM") + toModbusString(*(PDWORD(&outputBuffer[0])));
            }
            break;
        }

        if (!portName.empty())
            break;
    }
    ::RegCloseKey(key);
    return portName;
}

List<String> availableSerialPorts()
{
    List<String> ports;

    static const struct {
        GUID guid; DWORD flags;
    } setupTokens[] =  {
        { GUID_DEVCLASS_PORTS, DIGCF_PRESENT },
        { GUID_DEVCLASS_MODEM, DIGCF_PRESENT },
        { GUID_DEVINTERFACE_COMPORT, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE },
        { GUID_DEVINTERFACE_MODEM, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE }
    };

    for (const auto& setupToken : setupTokens)
    {
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&setupToken.guid, // Ports class GUID
                                                NULL,
                                                NULL,
                                                setupToken.flags);

        if (hDevInfo == INVALID_HANDLE_VALUE)
            return ports;

        SP_DEVINFO_DATA deviceInfoData{};
        deviceInfoData.cbSize = sizeof(deviceInfoData);

        DWORD i = 0;
        while (::SetupDiEnumDeviceInfo(hDevInfo, i++, &deviceInfoData))
        {
            const String portName = devicePortName(hDevInfo, &deviceInfoData);
            if (portName.empty() || portName.find("LPT") != std::string::npos)
                continue;
            
            if (std::find(ports.begin(), ports.end(), portName) != ports.end())
                continue;

            ports.push_back(portName);
        }
    }

    const auto portNames = portNamesFromHardwareDeviceMap();
    for (const auto &portName : portNames)
    {
        if (std::find(ports.begin(), ports.end(), portName) == ports.end())
            ports.push_back(portName);
    }

    return ports;
}

} // namespace Modbus
