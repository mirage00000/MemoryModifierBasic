#include <Windows.h>
#include <iostream>

int main() {
    printf("Welcome to the memory modification tool!");
    HWND hWnd = FindWindow(0, L"WindowTitle"); // Put the window title here
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
            data.intValue = 0x00000001; // Put the bytes here
            SIZE_T dataSize = sizeof(data);
            if (WriteProcessMemory(hProcess, (LPVOID)0x0000000, &data, sizeof(data), NULL)) { // Put the address here
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
