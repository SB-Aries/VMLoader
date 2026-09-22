#include "vm/vm.hpp"
#include "vm/config.hpp"
#include <array>
#include <cstring>

using HandlerFn = void(*)(RiscVm*, uint32_t);

// Forward-declared in each handler file
void handle_load     (RiscVm*, uint32_t);
void handle_misc_mem (RiscVm*, uint32_t);
void handle_op_imm   (RiscVm*, uint32_t);
void handle_auipc    (RiscVm*, uint32_t);
void handle_op_imm32 (RiscVm*, uint32_t);
void handle_store    (RiscVm*, uint32_t);
void handle_op       (RiscVm*, uint32_t);
void handle_lui      (RiscVm*, uint32_t);
void handle_op32     (RiscVm*, uint32_t);
void handle_branch   (RiscVm*, uint32_t);
void handle_jalr     (RiscVm*, uint32_t);
void handle_jal      (RiscVm*, uint32_t);
void handle_system   (RiscVm*, uint32_t);

static void handle_illegal(RiscVm* vm, uint32_t) { vm->halted = true; }

// Compile-time Fisher-Yates shuffle driven by VM_OPCODE_SEED.
// embed_payload.py uses the same algorithm to permute opcode fields in the payload binary.
static constexpr std::array<uint8_t, 32> gen_perm(uint32_t seed) {
    std::array<uint8_t, 32> p{};
    for (int i = 0; i < 32; i++) p[i] = (uint8_t)i;
    for (int i = 31; i > 0; i--) {
        seed = seed * 1664525u + 1013904223u;
        int j = (int)((seed >> 17) % (uint32_t)(i + 1));
        uint8_t t = p[i]; p[i] = p[j]; p[j] = t;
    }
    return p;
}

static HandlerFn g_dispatch[32];

static void init_dispatch() {
    for (auto& h : g_dispatch) h = handle_illegal;

    constexpr auto perm = gen_perm(VM_OPCODE_SEED);
    // perm[real_opcode] = slot in g_dispatch that the payload will use
    g_dispatch[perm[0x00]] = handle_load;
    g_dispatch[perm[0x03]] = handle_misc_mem;
    g_dispatch[perm[0x04]] = handle_op_imm;
    g_dispatch[perm[0x05]] = handle_auipc;
    g_dispatch[perm[0x06]] = handle_op_imm32;
    g_dispatch[perm[0x08]] = handle_store;
    g_dispatch[perm[0x0C]] = handle_op;
    g_dispatch[perm[0x0D]] = handle_lui;
    g_dispatch[perm[0x0E]] = handle_op32;
    g_dispatch[perm[0x18]] = handle_branch;
    g_dispatch[perm[0x19]] = handle_jalr;
    g_dispatch[perm[0x1B]] = handle_jal;
    g_dispatch[perm[0x1C]] = handle_system;
}

void riscvm_init(RiscVm* vm, uint8_t* mem, size_t mem_sz, SyscallTable* sc) {
    memset(vm, 0, sizeof(*vm));
    vm->mem_base = mem;
    vm->mem_size = mem_sz;
    vm->syscalls = sc;
    // pc and sp set by caller (loader) after choosing entry point
}

void riscvm_run(RiscVm* vm) {
    static bool initialised = false;
    if (!initialised) { init_dispatch(); initialised = true; }

    while (!vm->halted) {
        uint32_t instr = rv_fetch(vm);
        g_dispatch[(instr >> 2) & 0x1F](vm, instr);
    }
}
