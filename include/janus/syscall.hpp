#pragma once
#include <cstdint>
#include <cstddef>

// ECALL numbers (payload convention — a7 register)
enum JanusEcall : uint64_t {
    ECALL_GET_PEB   = 0,  // () -> void* peb
    ECALL_HOST_CALL = 1,  // (void* fn, uintptr_t* args, int argc) -> uintptr_t
    ECALL_RESOLVE   = 2,  // (uint32_t mod_hash, uint32_t sym_hash) -> void*
    ECALL_EXIT      = 3,  // (int code) -> noreturn
};

// Swap this struct to retarget the VM (different OS, sandbox environment, etc.)
struct SyscallTable {
    void*      (*get_peb)();
    uintptr_t  (*host_call)(void* fn, uintptr_t* args, int argc);
    void*      (*resolve)(uint32_t mod_hash, uint32_t sym_hash);
    void       (*exit_fn)(int code);
};

// Default Windows implementation (defined in src/syscall/table.cpp)
extern SyscallTable g_win_syscalls;
