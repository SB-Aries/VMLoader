#include "vm/config.hpp"
#include <cstdint>
#include <cstddef>

// LCG keystream seeded by VM_XOR_KEY — must match xor_crypt() in tools/embed_payload.py.
// Same operation decrypts (XOR is its own inverse).
void vm_xor_crypt(uint8_t* buf, size_t len) {
    uint32_t s = VM_XOR_KEY;
    for (size_t i = 0; i < len; i++) {
        s = s * 1664525u + 1013904223u;
        buf[i] ^= (uint8_t)(s >> 24);
    }
}
