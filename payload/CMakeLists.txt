cmake_minimum_required(VERSION 3.20)
project(VMPayload C CXX ASM)

# Requires a RISC-V cross-compiler:
#   apt install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu
# Or pass -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv64.cmake

set(CMAKE_CXX_STANDARD 17)

add_executable(payload
    crt0.S
    main.cpp
)

target_compile_options(payload PRIVATE
    -nostdlib
    -fno-exceptions
    -fno-rtti
    -fPIC
    -Os
    # VM implements RV64IM only — the C extension would emit 16-bit compressed
    # instructions that the interpreter (fixed 32-bit fetch, pc += 4) cannot run.
    -march=rv64im
    -mabi=lp64
)

target_link_options(payload PRIVATE
    -nostdlib
    -static
    -fuse-ld=lld      # requires: sudo apt install lld
    -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld
)

# After build, generate flat binary + run embed_payload.py
add_custom_command(TARGET payload POST_BUILD
    COMMAND python3 ${CMAKE_SOURCE_DIR}/../tools/embed_payload.py
            $<TARGET_FILE:payload>
            ${CMAKE_SOURCE_DIR}/../src/loader/payload_blob.cpp
    COMMENT "Embedding payload into loader"
)
