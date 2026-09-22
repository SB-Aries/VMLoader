#include "janus/config.hpp"
#if JANUS_INJECT_TECHNIQUE == 2

#include "janus/inject.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

static DWORD find_pid(const char* proc_name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32A pe = {};
    pe.dwSize = sizeof(pe);
    if (Process32FirstA(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, proc_name) == 0) {
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32NextA(snap, &pe));
    }
    CloseHandle(snap);
    return 0;
}

void janus_execute(RiscVm*) {
    DWORD pid = find_pid(JANUS_TARGET_PROCESS);
    if (!pid) return;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) return;

    char dll_path[MAX_PATH];
    GetModuleFileNameA(nullptr, dll_path, MAX_PATH);
    char* slash = strrchr(dll_path, '\\');
    if (slash)
        strcpy(slash + 1, "JanusLoader.dll");

    size_t path_len = strlen(dll_path) + 1;
    LPVOID remote_buf = VirtualAllocEx(hProc, nullptr, path_len,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_buf) {
        CloseHandle(hProc);
        return;
    }

    WriteProcessMemory(hProc, remote_buf, dll_path, path_len, nullptr);

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    auto load_lib = (LPTHREAD_START_ROUTINE)GetProcAddress(k32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
                                        load_lib, remote_buf, 0, nullptr);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    VirtualFreeEx(hProc, remote_buf, 0, MEM_RELEASE);
    CloseHandle(hProc);
}

#endif
