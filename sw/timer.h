#pragma once
#include <stdint.h>

/* Usa o Global Timer 64-bit do Cortex-A9 (PERIPHBASE + 0x200).
   PERIPHCLK = CPU_CLK / 2.  Ajuste CPU_FREQ_HZ se o PLL diferir.
   Zybo Z7-20: ARM Cortex-A9 @ 667 MHz -> PERIPHCLK = 333.5 MHz. */

#define CPU_FREQ_HZ     667000000UL
#define PERIPHCLK_HZ    (CPU_FREQ_HZ / 2UL)   /* ~333 MHz */

void     timer_init(void);
uint64_t timer_us(void);
void     timer_usleep(uint32_t us);
