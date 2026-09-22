#include "janus/vm.hpp"
#include "janus/decode.hpp"


void handle_jal(RiscVm* vm, uint32_t i) {
    int64_t target = (int64_t)vm->pc + rv_imm_j(i);
    rv_wreg(vm, rv_rd(i), (int64_t)(vm->pc + 4));
    vm->pc = (uint64_t)target;
}


void handle_jalr(RiscVm* vm, uint32_t i) {
    uint64_t target = (uint64_t)(vm->regs[rv_rs1(i)] + rv_imm_i(i)) & ~(uint64_t)1;
    rv_wreg(vm, rv_rd(i), (int64_t)(vm->pc + 4));
    vm->pc = target;
}


void handle_branch(RiscVm* vm, uint32_t i) {
    int     f3 = rv_f3(i), rs1 = rv_rs1(i), rs2 = rv_rs2(i);
    int64_t a  = vm->regs[rs1], b = vm->regs[rs2];
    bool    taken = false;

    switch (f3) {
        case 0: taken = a == b; break;                          // BEQ
        case 1: taken = a != b; break;                          // BNE
        case 4: taken = a  <  b; break;                         // BLT
        case 5: taken = a  >= b; break;                         // BGE
        case 6: taken = (uint64_t)a  <  (uint64_t)b; break;    // BLTU
        case 7: taken = (uint64_t)a  >= (uint64_t)b; break;    // BGEU
    }
    vm->pc = taken ? (uint64_t)((int64_t)vm->pc + rv_imm_b(i)) : vm->pc + 4;
}
