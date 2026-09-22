#include "janus/vm.hpp"
#include "janus/decode.hpp"

// ── SYSTEM (0x73): ECALL / EBREAK ────────────────────────────────────────────
//
// ECALL convention:
//   a7 = JanusEcall number
//   a0 = arg0 / return value
//   a1 = arg1
//   a2 = arg2
//
// EBREAK halts the VM (used by payload to signal completion).

void handle_system(RiscVm* vm, uint32_t i) {
    int f3  = rv_f3(i);
    int imm = (int)(i >> 20) & 0xFFF;

    // CSR instructions (funct3 != 0) — treat as NOP
    if (f3 != 0) { rv_adv(vm); return; }

    if (imm == 1) {  // EBREAK
        vm->halted = true;
        return;
    }

    // ECALL
    rv_adv(vm);  // advance before dispatch so return address is correct
    auto& r = vm->regs;
    switch ((JanusEcall)r[RV_A7]) {
        case ECALL_GET_PEB:
            r[RV_A0] = (int64_t)(uintptr_t)vm->syscalls->get_peb();
            break;
        case ECALL_HOST_CALL: {
            void*      fn   = (void*)(uintptr_t)r[RV_A0];
            uintptr_t* args = (uintptr_t*)(uintptr_t)r[RV_A1];
            int        argc = (int)r[RV_A2];
            r[RV_A0] = (int64_t)vm->syscalls->host_call(fn, args, argc);
            break;
        }
        case ECALL_RESOLVE: {
            uint32_t mod_h = (uint32_t)r[RV_A0];
            uint32_t sym_h = (uint32_t)r[RV_A1];
            r[RV_A0] = (int64_t)(uintptr_t)vm->syscalls->resolve(mod_h, sym_h);
            break;
        }
        case ECALL_EXIT:
            vm->syscalls->exit_fn((int)r[RV_A0]);
            vm->halted = true;
            break;
        default:
            // Unknown syscall — silently ignore, return 0
            r[RV_A0] = 0;
            break;
    }
}
