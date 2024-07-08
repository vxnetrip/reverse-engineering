// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#define TARGET_MODULE L"app.exe"

DWORD WINAPI MainThread(HMODULE hModule)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    printf("");

    //Sleep(2000);
    //FreeConsole();

    DWORD moduleBase = (DWORD)GetModuleHandle(TARGET_MODULE);

    // std::cout << std::hex << moduleBase << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());

    if (hProcess == NULL)
    {
        printf("Error opening process: %d\n", GetLastError());
        return 1;
    }

    DWORD dwAddress = moduleBase + 0x15D9;

    // je value 2370315380 found with cheat engine 

    DWORD patchValue = 2370315381; // jne value 2370315381

    if (!WriteProcessMemory(hProcess, (LPVOID)dwAddress, &patchValue, sizeof(patchValue), 0))
    {
        printf("Error writing to process memory: %d\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }
    CloseHandle(hProcess);

    return 0;
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}