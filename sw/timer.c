#include "timer.h"

/* Vitis standalone — usa o Global Timer 64-bit do Cortex-A9 via a API do BSP.
   COUNTS_PER_SECOND (xtime_l.h) ja vem com a frequencia correta do PS,
   entao nao dependemos de CPU_FREQ_HZ hardcoded. usleep() vem de sleep.h. */
#include "xtime_l.h"
#include "sleep.h"

void timer_init(void) {
    /* O BSP standalone ja inicializa o Global Timer no boot. */
}

uint64_t timer_us(void) {
    XTime t;
    XTime_GetTime(&t);
    return (uint64_t)t / (COUNTS_PER_SECOND / 1000000ULL);
}

void timer_usleep(uint32_t us) {
    usleep(us);
}
