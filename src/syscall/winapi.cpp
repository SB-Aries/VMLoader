#include "vm/syscall.hpp"
#include "vm/vhash.hpp"
#include <cstdint>
#include <cstring>
#include <intrin.h>   // __readgsqword


// sdbm over exact-case export names — must match vm_hash_sym().
static uint32_t hash_str(const char* s) {
    uint32_t h = 0;
    while (*s) h = h * 65599u + (uint8_t)*s++;
    return h;
}

// Case-insensitive sdbm over a wide DLL name (Length is in bytes) — matches vm_hash_mod().
static uint32_t hash_wname_ci(const wchar_t* wname, uint16_t len_bytes) {
    uint32_t h = 0;
    uint16_t n = len_bytes / 2;
    for (uint16_t i = 0; i < n; i++) {
        wchar_t c = wname[i];
        if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
        h = h * 65599u + (uint16_t)c;
    }
    return h;
}


static void* get_export(void* base, uint32_t sym_h) {
    uint8_t* b     = (uint8_t*)base;
    uint32_t pe_off = *(uint32_t*)(b + 0x3C);
    uint8_t* pe    = b + pe_off;

    // DataDirectory[0] = Export directory, at OptionalHeader64 + 0x70
    // OptionalHeader64 starts at pe + 0x18 (after Signature + FileHeader)
    uint32_t exp_rva = *(uint32_t*)(pe + 0x18 + 0x70);
    if (!exp_rva) return nullptr;

    uint8_t*  ed       = b + exp_rva;
    uint32_t  n_names  = *(uint32_t*)(ed + 24);
    uint32_t* names    = (uint32_t*)(b + *(uint32_t*)(ed + 32));
    uint16_t* ordinals = (uint16_t*)(b + *(uint32_t*)(ed + 36));
    uint32_t* funcs    = (uint32_t*)(b + *(uint32_t*)(ed + 28));

    for (uint32_t i = 0; i < n_names; i++) {
        if (hash_str((const char*)(b + names[i])) == sym_h)
            return b + funcs[ordinals[i]];
    }
    return nullptr;
}

//PEB WALK

void* vm_get_peb() {
    return (void*)__readgsqword(0x60);
}

// Resolves by hash only — no plaintext module/symbol strings cross the ECALL boundary.
void* vm_resolve(uint32_t mod_h, uint32_t sym_h) {
    uint8_t* peb  = (uint8_t*)vm_get_peb();
    uint8_t* ldr  = *(uint8_t**)(peb + 0x18);
    void*    head = *(void**)(ldr + 0x10);   // InLoadOrderModuleList.Flink
    void*    cur  = head;

    do {
        uint8_t*  entry   = (uint8_t*)cur;
        void*     dll_base = *(void**)(entry + 0x30);
        uint16_t  name_len = *(uint16_t*)(entry + 0x58);    // BaseDllName.Length (bytes)
        wchar_t*  name_buf = *(wchar_t**)(entry + 0x60);    // BaseDllName.Buffer

        if (dll_base && name_len && hash_wname_ci(name_buf, name_len) == mod_h) {
            void* fn = get_export(dll_base, sym_h);
            if (fn) return fn;
        }
        cur = *(void**)entry;  // follow Flink
    } while (cur != head);

    return nullptr;
}


uintptr_t vm_host_call(void* fn, uintptr_t* a, int argc) {
    using F0 = uintptr_t(*)();
    using F1 = uintptr_t(*)(uintptr_t);
    using F2 = uintptr_t(*)(uintptr_t,uintptr_t);
    using F3 = uintptr_t(*)(uintptr_t,uintptr_t,uintptr_t);
    using F4 = uintptr_t(*)(uintptr_t,uintptr_t,uintptr_t,uintptr_t);
    using F5 = uintptr_t(*)(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
    using F6 = uintptr_t(*)(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
    using F7 = uintptr_t(*)(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
    using F8 = uintptr_t(*)(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);

    switch (argc) {
        case 0: return ((F0)fn)();
        case 1: return ((F1)fn)(a[0]);
        case 2: return ((F2)fn)(a[0],a[1]);
        case 3: return ((F3)fn)(a[0],a[1],a[2]);
        case 4: return ((F4)fn)(a[0],a[1],a[2],a[3]);
        case 5: return ((F5)fn)(a[0],a[1],a[2],a[3],a[4]);
        case 6: return ((F6)fn)(a[0],a[1],a[2],a[3],a[4],a[5]);
        case 7: return ((F7)fn)(a[0],a[1],a[2],a[3],a[4],a[5],a[6]);
        case 8: return ((F8)fn)(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
        default: return 0;
    }
}

void vm_exit(int code) {
    // Resolve ExitProcess from kernel32 and call it — no import needed
    void* exit_fn = vm_resolve(vm_hash_mod("kernel32.dll"), vm_hash_sym("ExitProcess"));
    if (exit_fn) ((void(*)(unsigned int))exit_fn)((unsigned int)code);
    __assume(0);  // unreachable
}
