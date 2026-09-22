#pragma once
typedef unsigned long long uintptr_t;
typedef unsigned long long uint64_t;
typedef unsigned int       uint32_t;

// Inline wrappers for VM ECALLs from payload code.
// These must stay header-only so the payload has no external deps.

// sdbm hashes — identical copies live in include/vm/vhash.hpp on the host side.
// With literal arguments they fold to constants at compile time, so no plaintext
// DLL/API names ever appear in the payload binary.
constexpr uint32_t vm_hash_sym(const char* s, uint32_t h = 0) {
    return *s ? vm_hash_sym(s + 1, h * 65599u + (unsigned char)*s) : h;
}
constexpr uint32_t vm_hash_mod(const char* s, uint32_t h = 0) {
    return *s ? vm_hash_mod(s + 1, h * 65599u
        + (unsigned char)(*s >= 'A' && *s <= 'Z' ? *s + 32 : *s)) : h;
}

static inline void* vm_get_peb() {
    void* r;
    asm volatile (
        "li a7, 0\n"
        "ecall\n"
        "mv %0, a0\n"
        : "=r"(r) :: "a7", "a0", "memory"
    );
    return r;
}

// fn: function pointer obtained via vm_resolve
// args: array of uintptr_t on the stack, argc: element count
static inline uintptr_t vm_host_call(void* fn, uintptr_t* args, int argc) {
    uintptr_t r;
    asm volatile (
        "li a7, 1\n"
        "mv a0, %1\n"
        "mv a1, %2\n"
        "mv a2, %3\n"
        "ecall\n"
        "mv %0, a0\n"
        : "=r"(r)
        : "r"(fn), "r"(args), "r"((uintptr_t)argc)
        : "a7", "a0", "a1", "a2", "memory"
    );
    return r;
}

// mod_h / sym_h: vm_hash_mod() / vm_hash_sym() of the names — hashes, not strings.
static inline void* vm_resolve(uint32_t mod_h, uint32_t sym_h) {
    void* r;
    asm volatile (
        "li a7, 2\n"
        "mv a0, %1\n"
        "mv a1, %2\n"
        "ecall\n"
        "mv %0, a0\n"
        : "=r"(r)
        : "r"((uintptr_t)mod_h), "r"((uintptr_t)sym_h)
        : "a7", "a0", "a1", "memory"
    );
    return r;
}

static inline void vm_exit(int code) {
    asm volatile (
        "li a7, 3\n"
        "mv a0, %0\n"
        "ecall\n"
        :: "r"((uintptr_t)code) : "a7", "a0", "memory"
    );
}
