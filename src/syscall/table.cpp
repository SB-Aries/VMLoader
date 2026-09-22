#include "janus/syscall.hpp"

void*     janus_get_peb();
void*     janus_resolve(uint32_t mod_h, uint32_t sym_h);
uintptr_t janus_host_call(void* fn, uintptr_t* args, int argc);
void      janus_exit(int code);

SyscallTable g_win_syscalls = {
    .get_peb          = janus_get_peb,
    .host_call        = janus_host_call,
    .resolve          = janus_resolve,
    .exit_fn          = janus_exit,
};
