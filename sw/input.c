#include "input.h"

/* PIO core no LW HPS-to-FPGA bridge, configurado no Platform Designer.
   Ajuste o endereço base se o seu Qsys diferir. */
#define GPIO_PIO_BASE  0xFF200060u
#define GPIO_DATA      (*(volatile uint32_t *)(GPIO_PIO_BASE + 0x00u))
#define GPIO_DIR       (*(volatile uint32_t *)(GPIO_PIO_BASE + 0x04u))

/* Botões ativos-baixo: KEY0=esq, KEY1=dir, KEY2=fogo, KEY3=pause */
#define BTN_LEFT   (1u << 0)
#define BTN_RIGHT  (1u << 1)
#define BTN_FIRE   (1u << 2)
#define BTN_PAUSE  (1u << 3)
#define BTN_MASK   0xFu

static uint32_t prev_raw;

int input_init(void) {
    GPIO_DIR = 0x0u;            /* todos os pinos como entrada */
    prev_raw = GPIO_DATA & BTN_MASK;
    return 0;                   /* sem file descriptor em bare-metal */
}

void input_poll(int fd, InputState *state) {
    (void)fd;

    uint32_t raw         = GPIO_DATA & BTN_MASK;
    uint32_t pressed     = (~raw)      & BTN_MASK;  /* 1 = pressionado */
    uint32_t prev_pressed = (~prev_raw) & BTN_MASK;
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
