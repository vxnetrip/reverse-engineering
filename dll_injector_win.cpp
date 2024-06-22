#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <libloaderapi.h>
#include <tlhelp32.h>

using namespace std;

// By vxhelper
// e-z.bio/vxhelper

void error(const char* error_title, const char* error_message)
{
    MessageBox(NULL, error_message, error_title, NULL);
    exit(-1);
}

bool file_exists(const string& file_name)
{
    return (GetFileAttributes(file_name.c_str()) != INVALID_FILE_ATTRIBUTES);
}

DWORD get_process_id(const string& process_name)
{
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(entry);
        if (Process32First(snapshot, &entry))
        {
            do
            {
                if (string(entry.szExeFile) == process_name)
                {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

int main()
{
    DWORD id_process = NULL;
    char path_dll[MAX_PATH];
    const char* name_dll = "hook.dll"; // !!!
    const string process_name = "app.exe"; // !!!

    if (!file_exists(name_dll))
    {
        error("file_exists", "File doesn't exist");
    }

    if (!GetFullPathName(name_dll, MAX_PATH, path_dll, nullptr))
    {
        error("GetFullPathName", "Failed to get full path");
    }

    id_process = get_process_id(process_name);
    if (id_process == NULL)
    {
        error("get_process_id", "Failed to get process ID");
    }

    HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, NULL, id_process);
    if (!h_process)
    {
        error("OpenProcess", "Failed to open a handle to process");
    }

    void* allocated_memory = VirtualAllocEx(h_process, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!allocated_memory)
    {
        error("VirtualAllocEx", "Failed to allocate memory in process");
    }

    if (!WriteProcessMemory(h_process, allocated_memory, path_dll, strlen(path_dll) + 1, nullptr))
    {
        error("WriteProcessMemory", "Failed to write process memory");
    }

    HANDLE h_thread = CreateRemoteThread(h_process, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated_memory, NULL, nullptr);
    if (!h_thread)
    {
        error("CreateRemoteThread", "Failed to create remote thread");
    }

    WaitForSingleObject(h_thread, INFINITE);

    CloseHandle(h_process);
    VirtualFreeEx(h_process, allocated_memory, NULL, MEM_RELEASE);
    MessageBox(0, "Successfully Injected!", "Success", 0);

    return 0;
}
