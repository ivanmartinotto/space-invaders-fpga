# Space Invaders — FPGA (Zynq-7000 / Zybo Z7-20)

Jogo Space Invaders em sistema heterogêneo: lógica do jogo no ARM PS (C99
bare-metal, Cortex-A9), saída de vídeo **HDMI** gerada na FPGA PL por um pipeline
de IPs (AXI VDMA → rgb2dvi), sem HDL custom.

Spec original: [`space_invaders_fpga_spec.md`](space_invaders_fpga_spec.md)
(descreve um design VGA antigo com `CTRL_REG` — **superado** pelo fluxo VDMA/HDMI
deste repo; tratar como histórico).

---

## Plataforma alvo

| Placa | FPGA | DDR | CPU | Toolchain HW | Toolchain SW | Saída de vídeo |
|-------|------|-----|-----|--------------|--------------|----------------|
| Digilent Zybo Z7-20 | Xilinx XC7Z020 (Zynq-7000) | 1 GB | Cortex-A9 @ 667 MHz | Vivado 2025.2 | Vitis 2025.2 (standalone) | HDMI TX (rgb2dvi) |

> A build bare-metal usa a BSP standalone do Vitis (startup + linker gerados pelo
> Vitis). A antiga build raw `arm-none-eabi-gcc` (crt0/linker próprios) está em
> `legacy-gcc/`, mantida só por referência.

---

## Estrutura

```
space_invaders/
├── vivado/                     ← Projeto FPGA (Vivado 2025.2)
│   ├── build_hdmi.tcl          ← Monta o Block Design (PS7 + AXI VDMA + rgb2dvi)
│   ├── hdmi.xdc                ← Constraints dos pinos HDMI TX
│   ├── LEIAME.txt              ← Passo a passo do Vivado
│   ├── vivado-boards/          ← (clonar) board files da Digilent
│   └── Zybo-Z7-20-HDMI/        ← (clonar) IP repo da Digilent (rgb2dvi)
├── docs/
│   └── VITIS.md                ← Passo a passo do software no Vitis
├── legacy-gcc/                 ← Build raw arm-none-eabi-gcc (opcional)
└── sw/
    └── space_invaders_all.c    ← TODO o jogo num único arquivo C
```

O jogo inteiro vive em **`sw/space_invaders_all.c`** (framebuffer/VDMA, renderer,
lógica, input, timer, sprites, fonte, `main`). Procure os banners
`/* ===== … */` para navegar entre as seções.

---

## Mapa de memória (Zynq-7000)

| Região | Endereço | Tamanho | Domínio |
|--------|----------|---------|---------|
| Framebuffer 0 (XRGB8888) | `0x20000000` | 1,2 MB (640×480×4) | DDR (lido pelo AXI VDMA) |
| Framebuffer 1 (XRGB8888) | `0x2012C000` | 1,2 MB | DDR (lido pelo AXI VDMA) |
| AXI VDMA (S_AXI_LITE) | `0x43000000` | — | AXI-Lite (AXI GP0) |
| AXI GPIO (botões) | `0x41200000` | — | AXI-Lite (AXI GP0) |

> Não há `CTRL_REG`: o double buffer e o vsync são feitos pelo AXI VDMA em modo
> *park*. Os endereços acima são atribuídos pela ferramenta — **confirme** no
> Address Editor do Vivado (ou use os `XPAR_*` de `xparameters.h`) e edite os
> `#define …BASE` no topo da seção framebuffer/VDMA de `space_invaders_all.c`.

---

## Compilar o hardware (Vivado 2025.2)

Sem HDL custom — o PL é um Block Design de IPs montado por script.
**Guia completo: [`vivado/LEIAME.txt`](vivado/LEIAME.txt).** Resumo:

1. Em `vivado/`, clonar as duas dependências da Digilent:
   ```sh
   git clone https://github.com/Digilent/vivado-boards
   git clone https://github.com/Digilent/Zybo-Z7-20-HDMI
   ```
2. No Tcl Console do Vivado: `cd {…/vivado}` e `source build_hdmi.tcl`.
3. `Generate Bitstream` → `Export Hardware` (com bitstream) → `.xsa`.

---

## Compilar e rodar na placa (SW — bare-metal via Vitis)

**Guia completo: [`docs/VITIS.md`](docs/VITIS.md).** Resumo:

1. Vitis: `New Platform Component` a partir do `.xsa`, OS = **standalone**,
   CPU = `ps7_cortexa9_0`.
2. Vitis: `New Application Component` (Empty C) e importar **só**
   `sw/space_invaders_all.c`.
3. Build → `Run As → Launch Hardware` (boot JTAG): programa a PL e baixa o
   `.elf` na DDR. Conecte o monitor no HDMI.

> O Vitis fornece startup (`boot.S`/crt0) e `lscript.ld` pela BSP. `fb_swap()`
> faz `Xil_DCacheFlushRange()` no buffer antes do swap (DDR é cacheável no
> standalone). Não importar `legacy-gcc/`, `crt0.S` nem `linker.ld`.

---

## Controles (Zybo Z7 — botões ativos-alto)

| Botão | Ação |
|-------|------|
| BTN0 | Mover esquerda |
| BTN1 | Mover direita |
| BTN2 | Atirar |
| BTN3 | Pausar / Retomar |

Reiniciar: BTN2 na tela de Game Over.

---

## Ajustes de plataforma

Tudo em `sw/space_invaders_all.c`. Se o Address Editor do Vivado diferir, edite os
`#define …BASE`:

```c
#define FRAMEBUFFER_BASE  0x20000000u  /* frames lidos pelo AXI VDMA */
#define VDMA_BASE         0x43000000u  /* XPAR_AXI_VDMA_0_BASEADDR   */
#define GPIO_PIO_BASE     0x41200000u  /* botões (AXI GPIO)          */
```

O timer usa `XTime_GetTime`/`COUNTS_PER_SECOND` da BSP — a frequência do PS é
pega automaticamente, sem `#define` de clock.

---

## Vídeo

- Resolução: **640 × 480 @ 60 Hz**, pixel clock 25,175 MHz (Clocking Wizard a
  partir do FCLK_CLK0 de 100 MHz; serial 5× = 125,875 MHz para o TMDS).
- Framebuffer em DDR: **XRGB8888** (`0x00RRGGBB`, 32 bpp), layout row-major. O
  jogo desenha em RGB565 e a conversão acontece em `put_pixel`/`get_pixel`.
- Double buffering + vsync: AXI VDMA em modo *park* (2 frame stores).
- Se as cores aparecerem trocadas no monitor, ajuste o `TDATA_REMAP` do
  `axis_subset_converter_0` em `build_hdmi.tcl` (não no software).
