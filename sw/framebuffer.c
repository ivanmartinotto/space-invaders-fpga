#include "framebuffer.h"
#include "game.h"

/* Vitis standalone (bare-metal) — Zynq-7000 / Zybo Z7-20, pipeline de video
   AXI VDMA + rgb2dvi (HDMI TX). Veja vivado/build_hdmi.tcl e vivado/LEIAME.txt.

   DIFERENCAS vs. o design antigo (CTRL_REG custom):
   - O FRAMEBUFFER E 32 bits por pixel (0x00RRGGBB / XRGB8888). E o formato que o
     AXI VDMA (c_m_axis_mm2s_tdata_width = 32) + Subset Converter esperam.
     O jogo continua usando RGB565 (uint16_t) na API publica; a conversao para
     XRGB8888 acontece aqui, em put_pixel/get_pixel.
   - NAO existe CTRL_REG. O double buffer e o vsync sao feitos pelo proprio
     AXI VDMA (2 frame stores) operando em MODO PARK: o ponteiro de leitura
     (PARK_PTR_REG[4:0]) seleciona qual frame a PL exibe; trocamos esse ponteiro
     no swap. O flag de fim-de-frame (MM2S_VDMASR bit 12) faz o papel de vsync.

   COERENCIA DE CACHE: a DDR e cacheavel no BSP standalone. As escritas de pixel
   ficam em L1/L2 e o VDMA le a DDR direto — veria dados velhos. Antes de cada
   swap, Xil_DCacheFlushRange() no buffer recem-desenhado. */
#include "xil_cache.h"
#include "xil_io.h"

/* ── Framebuffers em DDR (32 bpp, lidos pelo AXI VDMA via porta AXI HP0) ─────
   Ajuste FRAMEBUFFER_BASE/VDMA_BASE/GPIO p/ o Address Editor do Vivado (ou use
   os XPAR_* de xparameters.h: XPAR_AXI_VDMA_0_BASEADDR). Os 2 frames precisam
   caber na DDR (Zybo Z7-20 = 1 GB), fora da regiao de codigo/heap/stack. */
#define FB_BPP            4u                                   /* XRGB8888       */
#define FRAMEBUFFER_BASE  0x20000000u
#define FRAMEBUFFER_SIZE  ((unsigned)SCREEN_W * SCREEN_H * FB_BPP) /* 1.228.800 B */

static uint32_t * const fb[2] = {
    (uint32_t *)FRAMEBUFFER_BASE,
    (uint32_t *)(FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE),
};
static int active_fb;     /* frame EXIBIDO pelo VDMA; back buffer = 1-active_fb */

/* ── Registradores do AXI VDMA (canal MM2S) ─────────────────────────────────
   Offsets do mapa de registradores do AXI VDMA (PG020). Base padrao p/ a 1a
   VDMA na GP0 costuma ser 0x4300_0000 — confirme no Address Editor. */
#define VDMA_BASE                0x43000000u
#define VDMA_MM2S_VDMACR         0x00u   /* control                            */
#define VDMA_MM2S_VDMASR         0x04u   /* status                             */
#define VDMA_PARK_PTR_REG        0x28u   /* [4:0] RD frame ptr (park)          */
#define VDMA_MM2S_VSIZE          0x50u   /* linhas; escrever AQUI inicia       */
#define VDMA_MM2S_HSIZE          0x54u   /* bytes por linha                    */
#define VDMA_MM2S_FRMDLY_STRIDE  0x58u   /* [15:0] stride em bytes             */
#define VDMA_MM2S_START_ADDR1    0x5Cu   /* endereco do frame store 1          */
#define VDMA_MM2S_START_ADDR2    0x60u   /* endereco do frame store 2          */

#define VDMACR_RS                (1u << 0)   /* run/stop                       */
#define VDMACR_CIRCULAR          (1u << 1)   /* 1=circular, 0=park             */
#define VDMACR_RESET             (1u << 2)   /* soft reset (auto-limpa)        */
#define VDMASR_HALTED            (1u << 0)
#define VDMASR_FRMCNT_IRQ        (1u << 12)  /* fim de frame (write-1-clear)   */

static inline uint32_t vdma_rd(uint32_t off)            { return Xil_In32(VDMA_BASE + off); }
static inline void     vdma_wr(uint32_t off, uint32_t v){ Xil_Out32(VDMA_BASE + off, v);    }

/* RGB565 (uint16_t) → XRGB8888 (0x00RRGGBB). Expande cada canal replicando os
   bits mais significativos para preencher 8 bits. */
static inline uint32_t rgb565_to_xrgb(uint16_t c) {
    uint32_t r = (c >> 11) & 0x1Fu;
    uint32_t g = (c >>  5) & 0x3Fu;
    uint32_t b =  c        & 0x1Fu;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    return (r << 16) | (g << 8) | b;
}

/* XRGB8888 → RGB565 (inverso, para get_pixel). */
static inline uint16_t xrgb_to_rgb565(uint32_t c) {
    uint32_t r = (c >> 16) & 0xFFu;
    uint32_t g = (c >>  8) & 0xFFu;
    uint32_t b =  c        & 0xFFu;
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

void fb_init(void) {
    active_fb = 0;

    /* Soft reset do canal MM2S e espera concluir. */
    vdma_wr(VDMA_MM2S_VDMACR, VDMACR_RESET);
    while (vdma_rd(VDMA_MM2S_VDMACR) & VDMACR_RESET) { }

    /* Enderecos dos 2 frame stores. */
    vdma_wr(VDMA_MM2S_START_ADDR1, FRAMEBUFFER_BASE);
    vdma_wr(VDMA_MM2S_START_ADDR2, FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE);

    /* Modo PARK (CIRCULAR=0): exibe sempre o frame apontado por PARK_PTR_REG.
       RS=1 inicia o canal. */
    vdma_wr(VDMA_MM2S_VDMACR, VDMACR_RS);

    /* Park no frame 0 (= active_fb). */
    vdma_wr(VDMA_PARK_PTR_REG, 0u);

    /* Geometria do frame. VSIZE deve ser escrito por ULTIMO (dispara o fetch). */
    vdma_wr(VDMA_MM2S_FRMDLY_STRIDE, (unsigned)SCREEN_W * FB_BPP);  /* stride */
    vdma_wr(VDMA_MM2S_HSIZE,         (unsigned)SCREEN_W * FB_BPP);  /* bytes/linha */
    vdma_wr(VDMA_MM2S_VSIZE,         (unsigned)SCREEN_H);           /* linhas → start */
}

void fb_cleanup(void) {
    /* Para o canal MM2S do VDMA. */
    vdma_wr(VDMA_MM2S_VDMACR, vdma_rd(VDMA_MM2S_VDMACR) & ~VDMACR_RS);
}

void fb_swap(void) {
    int back = 1 - active_fb;

    /* Garante que o back buffer recem-desenhado esta na DDR antes do VDMA ler. */
    Xil_DCacheFlushRange((INTPTR)fb[back], FRAMEBUFFER_SIZE);

    /* Aponta o park ptr para o back buffer; a PL passa a exibi-lo no proximo
       limite de frame. */
    vdma_wr(VDMA_PARK_PTR_REG, (uint32_t)back);
    active_fb = back;

    /* "vsync": espera o VDMA completar um frame (bit FrmCnt, write-1-clear).
       Limpa o flag e espera ele subir de novo = um limite de frame passou. */
    vdma_wr(VDMA_MM2S_VDMASR, VDMASR_FRMCNT_IRQ);
    while (!(vdma_rd(VDMA_MM2S_VDMASR) & VDMASR_FRMCNT_IRQ)) { }
}

void put_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return;
    fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x] = rgb565_to_xrgb(color);
}

uint16_t get_pixel(int x, int y) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return COLOR_BLACK;
    return xrgb_to_rgb565(fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x]);
}
