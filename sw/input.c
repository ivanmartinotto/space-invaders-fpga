#include "input.h"

/* Xilinx AXI GPIO (PL) na AXI GP0. Endereco default do IP no Vivado;
   ajuste se o seu Address Editor diferir.
   Mapa AXI GPIO: +0x00 = GPIO_DATA, +0x04 = GPIO_TRI (tri-state, 1 = entrada). */
#define GPIO_PIO_BASE  0x41200000u
#define GPIO_DATA      (*(volatile uint32_t *)(GPIO_PIO_BASE + 0x00u))
#define GPIO_TRI       (*(volatile uint32_t *)(GPIO_PIO_BASE + 0x04u))

/* Botoes Zybo Z7 sao ativos-ALTO (pressionado = 1): BTN0=esq, BTN1=dir,
   BTN2=fogo, BTN3=pause */
#define BTN_LEFT   (1u << 0)
#define BTN_RIGHT  (1u << 1)
#define BTN_FIRE   (1u << 2)
#define BTN_PAUSE  (1u << 3)
#define BTN_MASK   0xFu

static uint32_t prev_raw;

int input_init(void) {
    GPIO_TRI = BTN_MASK;        /* AXI GPIO: 1 = entrada (tri-state) */
    prev_raw = GPIO_DATA & BTN_MASK;
    return 0;                   /* sem file descriptor em bare-metal */
}

void input_poll(int fd, InputState *state) {
    (void)fd;

    uint32_t raw          = GPIO_DATA & BTN_MASK;
    uint32_t pressed      = raw      & BTN_MASK;  /* ativo-alto: 1 = pressionado */
    uint32_t prev_pressed = prev_raw & BTN_MASK;
    uint32_t just_pressed = pressed & ~prev_pressed; /* borda de subida */

    state->left  = (pressed      & BTN_LEFT)  ? 1 : 0;
    state->right = (pressed      & BTN_RIGHT) ? 1 : 0;
    state->fire  = (just_pressed & BTN_FIRE)  ? 1 : 0;
    state->pause = (just_pressed & BTN_PAUSE) ? 1 : 0;
    state->quit  = 0;
    state->reset = 0;

    prev_raw = raw;
}

void input_cleanup(int fd) {
    (void)fd;
}
