#include "ecall.hpp"

// Demo payload: pop calc.exe on Windows.
//
// All work happens as RV64 guest code interpreted by the VM; every effect is a
// fine-grained ECALL into a legitimate WinAPI. No x64 shellcode ever exists in
// memory — there is no exec-shellcode channel in this build at all.
//
// WinExec takes 2 args — within host_call's 8-argument limit
// (CreateProcessA's 10 parameters would not fit).

extern "C" void janus_main() {
    void* winexec = janus_resolve(jhash_mod("kernel32.dll"), jhash_sym("WinExec"));
    if (!winexec) return;

    const char* cmd = "calc.exe";
    uintptr_t args[2] = {
        (uintptr_t)cmd,
        1,                          // SW_SHOWNORMAL
    };
    janus_host_call(winexec, args, 2);
}
