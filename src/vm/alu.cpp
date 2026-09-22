#include "vm/vm.hpp"
#include "vm/decode.hpp"
#include <cstdint>

void handle_lui(RiscVm* vm, uint32_t i) {
    rv_wreg(vm, rv_rd(i), rv_imm_u(i));
    rv_adv(vm);
}

void handle_auipc(RiscVm* vm, uint32_t i) {
    rv_wreg(vm, rv_rd(i), (int64_t)vm->pc + rv_imm_u(i));
    rv_adv(vm);
}

void handle_op_imm(RiscVm* vm, uint32_t i) {
    int     rd  = rv_rd(i), rs1 = rv_rs1(i), f3 = rv_f3(i);
    int64_t src = vm->regs[rs1];
    int64_t imm = rv_imm_i(i);
    int64_t res = 0;

    switch (f3) {
        case 0: res = src + imm; break;                          // ADDI
        case 1: res = src << (imm & 0x3F); break;               // SLLI
        case 2: res = (int64_t)(src < imm); break;              // SLTI
        case 3: res = (int64_t)((uint64_t)src < (uint64_t)imm); break; // SLTIU
        case 4: res = src ^ imm; break;                          // XORI
        case 5:                                                   // SRLI / SRAI
            if (i >> 26 & 1) res = src >> (imm & 0x3F);         // SRAI (arith)
            else             res = (int64_t)((uint64_t)src >> (imm & 0x3F)); // SRLI
            break;
        case 6: res = src | imm; break;                          // ORI
        case 7: res = src & imm; break;                          // ANDI
    }
    rv_wreg(vm, rd, res);
    rv_adv(vm);
}


void handle_op_imm32(RiscVm* vm, uint32_t i) {
    int     rd  = rv_rd(i), rs1 = rv_rs1(i), f3 = rv_f3(i);
    int32_t src = (int32_t)vm->regs[rs1];
    int64_t imm = rv_imm_i(i);
    int32_t res = 0;

    switch (f3) {
        case 0: res = src + (int32_t)imm; break;                // ADDIW
        case 1: res = src << (imm & 0x1F); break;               // SLLIW
        case 5:                                                   // SRLIW / SRAIW
            if (i >> 30 & 1) res = src >> (imm & 0x1F);         // SRAIW
            else             res = (int32_t)((uint32_t)src >> (imm & 0x1F)); // SRLIW
            break;
    }
    rv_wreg(vm, rd, (int64_t)res);
    rv_adv(vm);
}


void handle_op(RiscVm* vm, uint32_t i) {
    int     rd = rv_rd(i), rs1 = rv_rs1(i), rs2 = rv_rs2(i);
    int     f3 = rv_f3(i), f7 = rv_f7(i);
    int64_t a  = vm->regs[rs1], b = vm->regs[rs2];
    int64_t res = 0;

    if (f7 == 1) {  // M extension
        switch (f3) {
            case 0: res = a * b; break;                                        // MUL
            case 1: { // MULH — signed * signed, upper 64 bits
                __int128 r = (__int128)a * (__int128)b;
                res = (int64_t)(r >> 64); break;
            }
            case 2: { // MULHSU — signed * unsigned, upper 64 bits
                __int128 r = (__int128)a * (__int128)(uint64_t)b;
                res = (int64_t)(r >> 64); break;
            }
            case 3: { // MULHU
                unsigned __int128 r = (unsigned __int128)(uint64_t)a * (uint64_t)b;
                res = (int64_t)(r >> 64); break;
            }
            case 4: // DIV
                if (!b)                             { res = -1; }
                else if (a == INT64_MIN && b == -1) { res = INT64_MIN; }
                else                                { res = a / b; }
                break;
            case 5: // DIVU
                res = b ? (int64_t)((uint64_t)a / (uint64_t)b) : (int64_t)UINT64_MAX; break;
            case 6: // REM
                if (!b)                             { res = a; }
                else if (a == INT64_MIN && b == -1) { res = 0; }
                else                                { res = a % b; }
                break;
            case 7: // REMU
                res = b ? (int64_t)((uint64_t)a % (uint64_t)b) : a; break;
        }
    } else {
        bool sub_sra = (f7 >> 5) & 1;
        switch (f3) {
            case 0: res = sub_sra ? (a - b) : (a + b); break;   // ADD / SUB
            case 1: res = a << (b & 0x3F); break;                // SLL
            case 2: res = (int64_t)(a < b); break;               // SLT
            case 3: res = (int64_t)((uint64_t)a < (uint64_t)b); break; // SLTU
            case 4: res = a ^ b; break;                          // XOR
            case 5:                                               // SRL / SRA
                if (sub_sra) res = a >> (b & 0x3F);
                else         res = (int64_t)((uint64_t)a >> (b & 0x3F));
                break;
            case 6: res = a | b; break;                          // OR
            case 7: res = a & b; break;                          // AND
        }
    }
    rv_wreg(vm, rd, res);
    rv_adv(vm);
}


void handle_op32(RiscVm* vm, uint32_t i) {
    int     rd = rv_rd(i), rs1 = rv_rs1(i), rs2 = rv_rs2(i);
    int     f3 = rv_f3(i), f7 = rv_f7(i);
    int32_t a  = (int32_t)vm->regs[rs1];
    int32_t b  = (int32_t)vm->regs[rs2];
    int32_t res = 0;
    bool sub_sra = (f7 >> 5) & 1;

    if (f7 == 1) {  // M extension 32-bit
        switch (f3) {
            case 0: res = a * b; break;                // MULW
            case 4: // DIVW
                if (!b)                             { res = -1; }
                else if (a == INT32_MIN && b == -1) { res = INT32_MIN; }
                else                                { res = a / b; }
                break;
            case 5: // DIVUW
                res = b ? (int32_t)((uint32_t)a / (uint32_t)b) : (int32_t)UINT32_MAX; break;
            case 6: // REMW
                if (!b)                             { res = a; }
                else if (a == INT32_MIN && b == -1) { res = 0; }
                else                                { res = a % b; }
                break;
            case 7: // REMUW
                res = b ? (int32_t)((uint32_t)a % (uint32_t)b) : a; break;
        }
    } else {
        switch (f3) {
            case 0: res = sub_sra ? (a - b) : (a + b); break;   // ADDW / SUBW
            case 1: res = a << (b & 0x1F); break;                // SLLW
            case 5:                                               // SRLW / SRAW
                if (sub_sra) res = a >> (b & 0x1F);
                else         res = (int32_t)((uint32_t)a >> (b & 0x1F));
                break;
        }
    }
    rv_wreg(vm, rd, (int64_t)res);
    rv_adv(vm);
}
