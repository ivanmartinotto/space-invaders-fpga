/* Minimal bare-metal startup for Cortex-A9 (Cyclone V HPS) */
    .section .text.start, "ax"
    .global _start
    .type   _start, %function
_start:
    ldr  sp, =_stack_top

    /* Zero BSS */
    ldr  r0, =_bss_start
    ldr  r1, =_bss_end
    mov  r2, #0
.bss_loop:
    cmp  r0, r1
    strlo r2, [r0], #4
    blo  .bss_loop

    bl   main

.hang:
    b    .hang
    .size _start, . - _start
