#pragma once
#include <cstdint>
#include <cstddef>
#include "syscall.hpp"

// RISC-V ABI register indices
enum : int {
    RV_ZERO=0, RV_RA=1, RV_SP=2, RV_GP=3, RV_TP=4,
    RV_T0=5,  RV_T1=6,  RV_T2=7,
    RV_S0=8,  RV_FP=8,  RV_S1=9,
    RV_A0=10, RV_A1=11, RV_A2=12, RV_A3=13,
    RV_A4=14, RV_A5=15, RV_A6=16, RV_A7=17,
    RV_S2=18, RV_S3=19, RV_S4=20, RV_S5=21,
    RV_S6=22, RV_S7=23, RV_S8=24, RV_S9=25,
    RV_S10=26, RV_S11=27,
    RV_T3=28, RV_T4=29, RV_T5=30, RV_T6=31,
};

struct RiscVm {
    int64_t       regs[32];   // x0 hardwired 0
    uint64_t      pc;
    uint8_t*      mem_base;   // flat allocation; guest addrs == host addrs
    size_t        mem_size;
    SyscallTable* syscalls;   // swappable
    bool          halted;
};

// Helpers
inline void   rv_wreg(RiscVm* vm, int rd, int64_t v) { if (rd) vm->regs[rd] = v; }
inline void   rv_adv (RiscVm* vm)                     { vm->pc += 4; }
inline uint32_t rv_fetch(const RiscVm* vm) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(vm->pc));
}

void riscvm_init(RiscVm* vm, uint8_t* mem, size_t mem_sz, SyscallTable* sc);
void riscvm_run (RiscVm* vm);
