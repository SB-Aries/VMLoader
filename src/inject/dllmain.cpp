#include "vm/config.hpp"
#if VM_INJECT_TECHNIQUE == 2

#include "vm/loader.hpp"
#include "vm/vm.hpp"
#include "vm/syscall.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        RiscVm vm{};
        if (vm_load(&vm, &g_win_syscalls))
            riscvm_run(&vm);
    }
    return TRUE;
}

#endif
