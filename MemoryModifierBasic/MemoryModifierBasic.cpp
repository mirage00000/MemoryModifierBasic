#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

struct Data {
    int intValue;
    float floatValue;
};

int main() {
    std::cout << "Welcome to the memory modification tool!\n";

    std::string winTitle;
    std::cout << "Enter the window title: ";
    std::getline(std::cin, winTitle);

    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, winTitle.c_str(), -1, nullptr, 0);
    wchar_t* wideStr = new wchar_t[bufferSize];
    MultiByteToWideChar(CP_UTF8, 0, winTitle.c_str(), -1, wideStr, bufferSize);
    LPCWSTR winTitleWide = wideStr;

    std::string modAddress;
    std::cout << "Enter the address: ";
    std::cin >> modAddress;
    unsigned long ulModAddress = std::stoul(modAddress, nullptr, 16);

    int bytesToMod;
    std::cout << "Enter the bytes to modify: ";
    std::cin >> std::hex >> bytesToMod;

    HWND hWnd = FindWindow(0, winTitleWide);
    if (hWnd == NULL) {
        MessageBox(0, L"Error! The script couldn't find a corresponding PID for the window. Make sure you specified the title correctly.", L"Error!", MB_OK | MB_ICONERROR);
    }
    else {
        DWORD procId;
        GetWindowThreadProcessId(hWnd, &procId);
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
        if (!hProcess) {
            MessageBox(NULL, L"Cannot open process!", L"Error!", MB_OK | MB_ICONERROR);
        }
        else {
            Data data;
            data.intValue = bytesToMod;

            std::cout << "You entered: " << bytesToMod << std::endl;

            if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(ulModAddress), &data, sizeof(data), NULL)) {
                MessageBox(NULL, L"Done! Memory written successfully.", L"Success!", MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(NULL, L"Error! Cannot write to memory.", L"Error!", MB_OK | MB_ICONERROR);
            }

            CloseHandle(hProcess);
        }
    }

    delete[] wideStr;
    return 0;
}
