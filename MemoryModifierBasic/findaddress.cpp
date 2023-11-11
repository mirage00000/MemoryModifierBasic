#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include "findaddress.h"
#include <sstream>
#include <iomanip>

BOOLEAN
FindProcessIdByName(
    LPCWSTR     ProcessName,
    PDWORD      ProcessId
) {
    if ((NULL == ProcessName) || (L'\0' == *ProcessName) || (NULL == ProcessId)) {
        return FALSE;
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (NULL == hSnap) {
        return FALSE;
    }

    BOOLEAN             bFoundProcess = FALSE;
    BOOLEAN             bFoundNextProcess = FALSE;

    PROCESSENTRY32W     PE32;
    ZeroMemory(&PE32, sizeof(PE32));
    PE32.dwSize = sizeof(PE32);

    bFoundNextProcess = Process32FirstW(hSnap, &PE32);
    while (TRUE == bFoundNextProcess) {

        if (NULL == _wcsicmp(ProcessName, PE32.szExeFile)) {
            *ProcessId = PE32.th32ProcessID;
            bFoundProcess = TRUE;
            break;
        }

        bFoundNextProcess = Process32NextW(hSnap, &PE32);
    }

    CloseHandle(hSnap);

    if (TRUE == bFoundProcess) {
        return TRUE;
    }

    return FALSE;
}

typedef struct _FOUND_ADDRESSES {
    UINT_PTR* Addresses;
    UINT_PTR    NumberOfAddresses;
} FOUND_ADDRESSES, * PFOUND_ADDRESSES;

BOOLEAN
FindAddressOfByteArray(
    PFOUND_ADDRESSES    FoundAddresses,
    HANDLE              ProcessHandle,
    BYTE* Data,
    INT                 DataSize,
    BOOLEAN             SkipCompleteMatches
)
{
    if ((NULL == ProcessHandle) || (NULL == Data) || (NULL == DataSize)) {
        return FALSE;
    }

    DWORD dwReadableMask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY
        | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE
        | PAGE_EXECUTE_WRITECOPY);
    DWORD dwProtectedMask = (PAGE_GUARD | PAGE_NOACCESS);

    INT                         iFoundSize = 10;
    UINT_PTR                    ulCurrAddr = NULL;
    BYTE* lpBuff = NULL;
    SIZE_T                      ulBytesRead = NULL;

    SYSTEM_INFO                 SysInfo;
    MEMORY_BASIC_INFORMATION    Mbi;

    ZeroMemory(&SysInfo, sizeof(SysInfo));
    ZeroMemory(&Mbi, sizeof(Mbi));

    FoundAddresses->Addresses = (UINT_PTR*)(malloc(iFoundSize * sizeof(UINT_PTR)));

    GetSystemInfo(&SysInfo);

    ulCurrAddr = (UINT_PTR)(SysInfo.lpMinimumApplicationAddress);

    while ((sizeof(Mbi) == VirtualQueryEx(ProcessHandle, (LPVOID)(ulCurrAddr), &Mbi, sizeof(Mbi)),
        (ulCurrAddr <= (UINT_PTR)(SysInfo.lpMaximumApplicationAddress)))) {

        if ((dwReadableMask & Mbi.Protect) && (FALSE == (dwProtectedMask & Mbi.Protect))) {
            lpBuff = (BYTE*)(malloc(Mbi.RegionSize));

            if (TRUE == ReadProcessMemory(ProcessHandle, (LPVOID)(ulCurrAddr), lpBuff, Mbi.RegionSize,
                &ulBytesRead)) {

                if (ulBytesRead == Mbi.RegionSize) {

                    for (UINT i = 0; i < Mbi.RegionSize; ++i) {

                        if (0 == memcmp((LPCVOID)(lpBuff + i), Data, DataSize)) {
                            if (iFoundSize == (FoundAddresses->NumberOfAddresses + 1)) {
                                LPVOID lpTemp = realloc(FoundAddresses->Addresses, (iFoundSize += 50)
                                    * sizeof(UINT_PTR));

                                if (NULL == lpTemp) {
                                    free(FoundAddresses->Addresses);
                                    free(lpBuff);

                                    return FALSE;
                                }

                                FoundAddresses->Addresses = (UINT_PTR*)(lpTemp);
                            }

                            FoundAddresses->Addresses[++FoundAddresses->NumberOfAddresses]
                                = (ulCurrAddr + i);

                            if (TRUE == SkipCompleteMatches) {
                                i += DataSize;
                            }
                        }
                    }
                }
            }

            free(lpBuff);
        }

        ulCurrAddr = (UINT_PTR)(Mbi.BaseAddress) + Mbi.RegionSize;
    }

    return TRUE;
}


int findaddress(const std::string& str)
{

    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);

    wchar_t* wideStr = new wchar_t[bufferSize];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wideStr, bufferSize);

    LPCWSTR     szProcessName = wideStr;
    DWORD       dwProcessId = NULL;

    if (FALSE == FindProcessIdByName(szProcessName, &dwProcessId)) {
        printf("Could not find the process! Press any key to exit...\n");

        getchar();

        return 0;
    }
    // beginning of user input
    std::cout << "Enter the size of the byte array (e.g., 3): ";
    int size;
    std::cin >> size;

    BYTE* lpData = new BYTE[size];

    std::cout << "Enter the bytes (e.g., 01 01 01): ";
    for (int i = 0; i < size; ++i) {
        int byteValue;
        std::cin >> std::hex >> byteValue;
        lpData[i] = static_cast<BYTE>(byteValue);
    }
    printf("You entered: ");
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", static_cast<int>(lpData[i]));
    }
    printf("\n");
    //end of user input

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

    if (NULL == hProc) {
        printf("Could not open a handle to the process! Press any key to exit...\n");

        getchar();

        return 0;
    }

    FOUND_ADDRESSES faFound;
    ZeroMemory(&faFound, sizeof(faFound));
    if (FALSE == FindAddressOfByteArray(&faFound, hProc, lpData, sizeof(lpData), TRUE)) {
        printf("Could not search for the byte array!\n");
    }

    CloseHandle(hProc);

    printf("Found %d addresses! Press any key to print all of them...\n", faFound.NumberOfAddresses);

    getchar();

    for (UINT i = NULL; i < faFound.NumberOfAddresses; ++i) {
        printf("0x%08X\n", faFound.Addresses[i]);
    }

    free(faFound.Addresses);
    printf("Press any key to exit...");

    getchar();

    return 0;
}