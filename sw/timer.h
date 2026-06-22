#pragma once
#include <stdint.h>

/* Build Vitis (alvo): timer.c usa XTime_GetTime + COUNTS_PER_SECOND do BSP,
   entao estas constantes nao sao usadas. Mantidas para a build host (SDL) e
   para a build legacy raw-gcc (legacy-gcc/), que nao tem o BSP do Vitis.
   Zybo Z7-20: ARM Cortex-A9 @ 667 MHz -> PERIPHCLK = 333.5 MHz. */

#define CPU_FREQ_HZ     667000000UL
#define PERIPHCLK_HZ    (CPU_FREQ_HZ / 2UL)   /* ~333 MHz */

void     timer_init(void);
uint64_t timer_us(void);
void     timer_usleep(uint32_t us);
