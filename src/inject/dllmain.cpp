#include "janus/config.hpp"
#if JANUS_INJECT_TECHNIQUE == 2

#include "janus/loader.hpp"
#include "janus/vm.hpp"
#include "janus/syscall.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        RiscVm vm{};
        if (janus_load(&vm, &g_win_syscalls))
            riscvm_run(&vm);
    }
    return TRUE;
}

#endif
