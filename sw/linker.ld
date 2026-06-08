/* Zynq-7000 (Zybo Z7-20) — código carregado em DDR3 via JTAG (XSCT) ou FSBL.
   DDR: 1 GB @ 0x00000000. ORIGIN deixa os primeiros 16 MB livres
   (vetores, FSBL, stack do bootloader). */
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x01000000, LENGTH = 48M
}

_stack_size = 0x10000;   /* 64 KB */

SECTIONS {
    . = ORIGIN(RAM);

    .text : {
        *(.text.start)   /* crt0 entra primeiro */
        *(.text*)
        *(.rodata*)
    } > RAM

    . = ALIGN(4);
    .data : {
        *(.data*)
    } > RAM

    . = ALIGN(4);
    .bss : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > RAM

    . = ALIGN(8);
    _stack_top = . + _stack_size;
}
