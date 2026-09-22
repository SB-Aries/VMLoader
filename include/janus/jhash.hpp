#pragma once
#include <cstdint>

// sdbm string hash. payload/ecall.hpp carries identical copies for the guest side —
// with literal arguments these fold to constants at compile time, so no plaintext
// DLL/API names ever appear in either binary.
constexpr uint32_t jhash_sym(const char* s, uint32_t h = 0) {
    return *s ? jhash_sym(s + 1, h * 65599u + (uint8_t)*s) : h;
}

// Module-name variant: case-insensitive (PEB stores DLL names in varying case).
constexpr uint32_t jhash_mod(const char* s, uint32_t h = 0) {
    return *s ? jhash_mod(s + 1, h * 65599u
        + (uint8_t)(*s >= 'A' && *s <= 'Z' ? *s + ('a' - 'A') : *s)) : h;
}
