OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x0;
    __base = .;

    .text   : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }

    . = ALIGN(8);
    .data   : { *(.data .data.*) }

    . = ALIGN(8);
    __bss_start = .;
    .bss    : { *(.bss .bss.*) *(COMMON) }
    __bss_end = .;
}
