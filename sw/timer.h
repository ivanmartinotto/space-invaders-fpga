#pragma once
#include <stdint.h>

/* Usa o Global Timer 64-bit do Cortex-A9 (PERIPHBASE + 0x200).
   PERIPHCLK = CPU_CLK / 2.  Ajuste CPU_FREQ_HZ se o PLL diferir. */

#define CPU_FREQ_HZ     800000000UL
#define PERIPHCLK_HZ    (CPU_FREQ_HZ / 2UL)   /* 400 MHz */

void     timer_init(void);
uint64_t timer_us(void);
void     timer_usleep(uint32_t us);
