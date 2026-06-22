#include "framebuffer.h"
#include "game.h"

/* Vitis standalone (bare-metal) — Zynq-7000 / Zybo Z7-20.
   Os dois framebuffers ficam em DDR (lidos pelo AXI VDMA via porta AXI HP).
   CTRL_REG e um slave AXI-Lite custom na AXI GP0.

   IMPORTANTE — coerencia de cache:
   No BSP standalone do Vitis a MMU fica ligada com a DDR CACHEAVEL. As escritas
   de pixel ficam em L1/L2 e o VDMA, que le a DDR direto, veria dados velhos.
   Antes de cada swap fazemos Xil_DCacheFlushRange() no buffer recem-desenhado.
   (Alternativa: marcar a regiao como nao-cacheavel com Xil_SetTlbAttributes.) */
#include "xil_cache.h"
#include "xil_io.h"

/* Ajuste estes enderecos para coincidir com o Address Editor do block design.
   Se voce gerou xparameters.h, pode trocar pelos XPAR_* correspondentes. */
#define FRAMEBUFFER_BASE  0x20000000u
#define FRAMEBUFFER_SIZE  (SCREEN_W * SCREEN_H * 2u)
#define CTRL_REG_BASE     0x40000000u
#define CTRL_VSYNC_FLAG   0x2u
#define CTRL_BUF_SELECT   0x1u

static volatile uint16_t * const fb[2] = {
    (volatile uint16_t *)FRAMEBUFFER_BASE,
    (volatile uint16_t *)(FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE),
};
static int active_fb;

void fb_init(void) {
    active_fb = 0;
}

void fb_cleanup(void) {
    /* nada a liberar em bare-metal */
}

void fb_swap(void) {
    /* Buffer recem-desenhado = back buffer = fb[1 - active_fb].
       Garante que ele esta na DDR antes do VDMA ler. */
    Xil_DCacheFlushRange((INTPTR)fb[1 - active_fb], FRAMEBUFFER_SIZE);

    /* Espera o flag de vsync da PL (bit 1) */
    while (!(Xil_In32(CTRL_REG_BASE) & CTRL_VSYNC_FLAG));

    active_fb = 1 - active_fb;

    uint32_t v = Xil_In32(CTRL_REG_BASE);
    v = (v & ~CTRL_BUF_SELECT) | (uint32_t)active_fb;  /* seleciona buffer ativo */
    v &= ~CTRL_VSYNC_FLAG;                              /* limpa o flag de vsync */
    Xil_Out32(CTRL_REG_BASE, v);
}

void put_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return;
    fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x] = color;
}

uint16_t get_pixel(int x, int y) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return COLOR_BLACK;
    return fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x];
}
