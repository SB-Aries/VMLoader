#include "vm/vm.hpp"
#include "vm/decode.hpp"
#include <cstring>

// Shared address space: guest pointers are valid host pointers — no translation needed.


void handle_load(RiscVm* vm, uint32_t i) {
    int      rd   = rv_rd(i), rs1 = rv_rs1(i), f3 = rv_f3(i);
    uint8_t* addr = reinterpret_cast<uint8_t*>((uintptr_t)(vm->regs[rs1] + rv_imm_i(i)));
    int64_t  res  = 0;

    switch (f3) {
        case 0: { int8_t   v; memcpy(&v, addr, 1); res = v; break; }  // LB
        case 1: { int16_t  v; memcpy(&v, addr, 2); res = v; break; }  // LH
        case 2: { int32_t  v; memcpy(&v, addr, 4); res = v; break; }  // LW
        case 3: { int64_t  v; memcpy(&v, addr, 8); res = v; break; }  // LD
        case 4: { uint8_t  v; memcpy(&v, addr, 1); res = (int64_t)v; break; } // LBU
        case 5: { uint16_t v; memcpy(&v, addr, 2); res = (int64_t)v; break; } // LHU
        case 6: { uint32_t v; memcpy(&v, addr, 4); res = (int64_t)v; break; } // LWU
    }
    rv_wreg(vm, rd, res);
    rv_adv(vm);
}


void handle_store(RiscVm* vm, uint32_t i) {
    int      rs1  = rv_rs1(i), rs2 = rv_rs2(i), f3 = rv_f3(i);
    uint8_t* addr = reinterpret_cast<uint8_t*>((uintptr_t)(vm->regs[rs1] + rv_imm_s(i)));
    uint64_t src  = (uint64_t)vm->regs[rs2];

    switch (f3) {
        case 0: memcpy(addr, &src, 1); break;  // SB
        case 1: memcpy(addr, &src, 2); break;  // SH
        case 2: memcpy(addr, &src, 4); break;  // SW
        case 3: memcpy(addr, &src, 8); break;  // SD
    }
    rv_adv(vm);
}


void handle_misc_mem(RiscVm* vm, uint32_t) { rv_adv(vm); }
