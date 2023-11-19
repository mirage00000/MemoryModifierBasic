#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

struct Data {
    int intValue;
    float floatValue;
};

struct FoundAddresses {
    std::vector<UINT_PTR> Addresses;
};

bool FindAddressOfByteArray(FoundAddresses& foundAddresses, HANDLE processHandle, const BYTE* data, size_t dataSize, bool skipCompleteMatches) {
    if (!processHandle || !data || dataSize == 0) {
        return false;
    }

    DWORD dwReadableMask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
    DWORD dwProtectedMask = (PAGE_GUARD | PAGE_NOACCESS);

    SYSTEM_INFO SysInfo;
    MEMORY_BASIC_INFORMATION Mbi;

    ZeroMemory(&SysInfo, sizeof(SysInfo));
    ZeroMemory(&Mbi, sizeof(Mbi));

    GetSystemInfo(&SysInfo);

    UINT_PTR ulCurrAddr = reinterpret_cast<UINT_PTR>(SysInfo.lpMinimumApplicationAddress);

    while (sizeof(Mbi) == VirtualQueryEx(processHandle, reinterpret_cast<LPVOID>(ulCurrAddr), &Mbi, sizeof(Mbi)) &&
        ulCurrAddr <= reinterpret_cast<UINT_PTR>(SysInfo.lpMaximumApplicationAddress)) {

        if ((dwReadableMask & Mbi.Protect) && !(dwProtectedMask & Mbi.Protect)) {
            std::vector<BYTE> buffer(Mbi.RegionSize);

            SIZE_T ulBytesRead;
            if (ReadProcessMemory(processHandle, reinterpret_cast<LPVOID>(ulCurrAddr), buffer.data(), Mbi.RegionSize, &ulBytesRead) && ulBytesRead == Mbi.RegionSize) {

                for (size_t i = 0; i < Mbi.RegionSize; ++i) {
                    if (memcmp(buffer.data() + i, data, dataSize) == 0) {
                        foundAddresses.Addresses.push_back(ulCurrAddr + i);

                        if (skipCompleteMatches) {
                            i += dataSize;
                        }
                    }
                }
            }
        }

        ulCurrAddr = reinterpret_cast<UINT_PTR>(Mbi.BaseAddress) + Mbi.RegionSize;
    }

    return true;
}

void search_for_array(HANDLE hProcess) {
    try {
        std::cout << "Enter the byte array (e.g., 0x01 0x02 0x03 0xff): ";
        std::vector<BYTE> userByteArray;
        int byteValue;
        while (std::cin >> std::hex >> byteValue) {
            userByteArray.push_back(static_cast<BYTE>(byteValue));
        }

        std::cin.clear();
        std::cin.ignore(10000, '\n');

        FoundAddresses foundAddresses;
        if (!FindAddressOfByteArray(foundAddresses, hProcess, userByteArray.data(), userByteArray.size(), true)) {
            throw std::runtime_error("Could not search for the byte array!");
        }

        std::cout << "Found " << foundAddresses.Addresses.size() << " addresses:" << std::endl;
        for (const auto& address : foundAddresses.Addresses) {
            std::cout << "0x" << std::hex << address << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Welcome to the memory modification tool!\n";
    std::string winTitle;
    std::cout << "Enter the window title: ";
    std::getline(std::cin, winTitle);

    std::wstring winTitleWide(winTitle.begin(), winTitle.end());
    HWND hWnd = FindWindow(0, winTitleWide.c_str());

    try {
        if (!hWnd) {
            throw std::runtime_error("Process not found");
            getchar();
        }

        DWORD procId;
        GetWindowThreadProcessId(hWnd, &procId);
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

        if (!hProcess) {
            throw std::runtime_error("Failed to open a handle to the process");
            getchar();
        }

        std::cout << "\nBeginning of the byte array search function\n";
        search_for_array(hProcess);
        std::cout << "End of the byte search array function\n\n";

        std::string modAddress;
        std::cout << "Enter the address: ";
        std::cin >> modAddress;
        unsigned long ulModAddress = std::stoul(modAddress, nullptr, 16);

        unsigned long bytesToMod;
        std::cout << "Enter the bytes to modify (e.g., 0xff030201): ";
        std::cin >> std::hex >> bytesToMod;

        Data data;
        data.intValue = bytesToMod;
        std::cout << "\nResult: ";
        if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(ulModAddress), &data, sizeof(data), NULL)) {
            std::cout << "Memory written successfully!\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            getchar();
        }
        else {
            std::cout << "Unable to write process' memory.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            getchar();
        }

        CloseHandle(hProcess);
    }
    catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        getchar();
    }

    return 0;
}