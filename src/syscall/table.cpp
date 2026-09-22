#include "vm/syscall.hpp"

void*     vm_get_peb();
void*     vm_resolve(uint32_t mod_h, uint32_t sym_h);
uintptr_t vm_host_call(void* fn, uintptr_t* args, int argc);
void      vm_exit(int code);

SyscallTable g_win_syscalls = {
    .get_peb          = vm_get_peb,
    .host_call        = vm_host_call,
    .resolve          = vm_resolve,
    .exit_fn          = vm_exit,
};
