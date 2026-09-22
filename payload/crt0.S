.section .text
.global _start
_start:
    # Set up global pointer (linker relaxation)
    .option push
    .option norelax
    la      gp, __global_pointer$
    .option pop

    # Zero .bss
    la      a0, __bss_start
    la      a1, __bss_end
.Lbss_loop:
    bge     a0, a1, .Lbss_done
    sb      zero, 0(a0)
    addi    a0, a0, 1
    j       .Lbss_loop
.Lbss_done:

    # sp already set by loader (top of stack region)
    call    janus_main

    li      a7, 3          # ECALL_EXIT
    li      a0, 0
    ecall
