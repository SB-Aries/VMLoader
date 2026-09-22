#pragma once
#include <cstdint>

// Field extractors — no bit-twiddling at call sites
inline int rv_rd (uint32_t i) { return (i >> 7)  & 0x1F; }
inline int rv_rs1(uint32_t i) { return (i >> 15) & 0x1F; }
inline int rv_rs2(uint32_t i) { return (i >> 20) & 0x1F; }
inline int rv_f3 (uint32_t i) { return (i >> 12) & 0x7;  }
inline int rv_f7 (uint32_t i) { return (i >> 25) & 0x7F; }
inline int rv_op (uint32_t i) { return (i >> 2)  & 0x1F; }

// Sign-extended immediates
inline int64_t rv_imm_i(uint32_t i) {
    return (int64_t)(int32_t)i >> 20;
}
inline int64_t rv_imm_u(uint32_t i) {
    return (int64_t)(int32_t)(i & 0xFFFFF000u);
}
inline int64_t rv_imm_s(uint32_t i) {
    // imm[11:5] at bits 31:25, imm[4:0] at bits 11:7
    return ((int64_t)(int32_t)i >> 20 & ~(int64_t)0x1F) | ((i >> 7) & 0x1F);
}
inline int64_t rv_imm_b(uint32_t i) {
    // {imm[12], imm[11], imm[10:5], imm[4:1], 0}
    uint32_t raw = ((i >> 19) & 0x1000u)  // bit 31 → bit 12
                 | ((i << 4)  & 0x0800u)  // bit  7 → bit 11
                 | ((i >> 20) & 0x07E0u)  // bits 30:25 → bits 10:5
                 | ((i >> 7)  & 0x001Eu); // bits 11:8  → bits 4:1
    return (int64_t)(int32_t)(raw << 19) >> 19;  // sign-extend from bit 12
}
inline int64_t rv_imm_j(uint32_t i) {
    // {imm[20], imm[19:12], imm[11], imm[10:1], 0}
    uint32_t raw = ((i >> 11) & 0x100000u) // bit 31 → bit 20
                 | (i         & 0x0FF000u)  // bits 19:12 stay
                 | ((i >> 9)  & 0x000800u)  // bit 20 → bit 11
                 | ((i >> 20) & 0x0007FEu); // bits 30:21 → bits 10:1
    return (int64_t)(int32_t)(raw << 11) >> 11;  // sign-extend from bit 20
}
