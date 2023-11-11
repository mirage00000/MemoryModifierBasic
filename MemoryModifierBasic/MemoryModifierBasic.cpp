#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include "findaddress.h"
#include <sstream>
#include <iomanip>

int main() {
    printf("Welcome to the memory modification tool!\n");
    std::string winTitl;
    std::string procName;
    std::string modAddress;
    int bytesToMod;
    std::cout << "Enter the window title: ";
    std::cin >> winTitl;
    std::cout << "Enter the process name: ";
    std::cin >> procName;
    findaddress(procName);
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, winTitl.c_str(), -1, nullptr, 0);

    wchar_t* wideStr = new wchar_t[bufferSize];
    MultiByteToWideChar(CP_UTF8, 0, winTitl.c_str(), -1, wideStr, bufferSize);

    LPCWSTR     winTitle = wideStr;
    printf("\n");
    std::cout << "Enter the address: ";
    std::cin >> modAddress;
    unsigned long ulModAddress = std::stoul(modAddress, nullptr, 16);
    std::cout << "Enter the bytes to modify: ";
    std::cin >> bytesToMod;

    HWND hWnd = FindWindow(0, winTitle);
    if (hWnd == NULL) {
        MessageBox(0, L"Error! The script wasn't able to find a corresponding PID for the window. Make sure you specified the title correctly.", L"Error!", MB_OK | MB_ICONERROR); // Use | instead of +
    }
    else {
        DWORD proc_id;
        GetWindowThreadProcessId(hWnd, &proc_id);
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);
        if (!hProcess) {
            MessageBox(NULL, L"Cannot open process!", L"Error!", MB_OK | MB_ICONERROR);
        }
        else {
            union Data {
                int intValue;
                float floatValue;
                // Add other data types if needed
            };
            Data data;
            data.intValue = bytesToMod;
            SIZE_T dataSize = sizeof(data);
            if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(ulModAddress), &data, sizeof(data), NULL)) {
                MessageBox(NULL, L"Done! Memory written succesfully.", L"Success!", MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(NULL, L"Error! Cannot write the memory.", L"Error!", MB_OK | MB_ICONERROR);
            }
            CloseHandle(hProcess);
        }
    }

    return 0;
}
