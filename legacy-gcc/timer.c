#include "timer.h"

/* Cortex-A9 Global Timer — Zynq-7000 PERIPHBASE 0xF8F00000 + offset 0x200 */
#define GT_BASE      0xF8F00200u
#define GT_LO        (*(volatile uint32_t *)(GT_BASE + 0x00u))
#define GT_HI        (*(volatile uint32_t *)(GT_BASE + 0x04u))
#define GT_CTRL      (*(volatile uint32_t *)(GT_BASE + 0x08u))

void timer_init(void) {
    GT_CTRL = 0x1u;   /* enable */
}

uint64_t timer_us(void) {
    uint32_t hi1, lo, hi2;
    /* Leitura atômica: repete se upper mudou durante leitura do lower */
    do {
        hi1 = GT_HI;
        lo  = GT_LO;
        hi2 = GT_HI;
    } while (hi1 != hi2);
    uint64_t ticks = ((uint64_t)hi1 << 32) | lo;
    return ticks / (PERIPHCLK_HZ / 1000000UL);
}

void timer_usleep(uint32_t us) {
    uint64_t start = timer_us();
    while ((timer_us() - start) < (uint64_t)us);
}
