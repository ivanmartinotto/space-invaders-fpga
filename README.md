# Space Invaders — FPGA (Zynq-7000 / Zybo Z7-20)

Jogo Space Invaders em sistema heterogêneo: lógica do jogo no ARM PS (C99 bare-metal, Cortex-A9), saída de vídeo VGA gerada na FPGA PL (VHDL).

Spec completa: [`space_invaders_fpga_spec.md`](space_invaders_fpga_spec.md)

---

## Plataforma alvo

| Placa | FPGA | DDR | CPU | Toolchain HW | Toolchain SW | Saída VGA |
|-------|------|-----|-----|--------------|--------------|-----------|
| Digilent Zybo Z7-20 | Xilinx XC7Z020 (Zynq-7000) | 1 GB | Cortex-A9 @ 667 MHz | Vivado | `arm-none-eabi-gcc` | Pmod VGA (RGB444) |

> Versão anterior tinha duplo alvo (DE10-Nano / Zynq). Esta versão é **Zynq-only, Vivado-only**.

---

## Estrutura

```
space_invaders/
├── hw/                         ← Projeto FPGA Vivado (ainda não implementado)
│   ├── top.vhd
│   ├── vga_sync.vhd
│   ├── rgb_output.vhd          ← RGB565 → RGB444 (Pmod VGA)
│   ├── ctrl_reg.vhd            ← AXI-Lite slave (GP0)
│   ├── constraints/
│   │   └── zybo_z7.xdc         ← Pin assignments (Pmod + botões)
│   └── block_design/           ← ZYNQ7 PS + AXI VDMA + Clocking Wizard
└── sw/                         ← Código ARM (C99 bare-metal)
    ├── Makefile
    ├── crt0.S                  ← Startup bare-metal (Zynq PS)
    ├── linker.ld               ← Linker script (código em 0x01000000)
    ├── main.c
    ├── game.h                  ← Todos os tipos e constantes
    ├── game.c / game_api.h     ← Máquina de estados + renderização
    ├── framebuffer.c/h         ← Acesso direto ao framebuffer (DDR)
    ├── input.c/h               ← Botões via AXI GPIO (PL)
    ├── timer.c/h               ← Global Timer 64-bit do Cortex-A9
    ├── renderer.c/h            ← Primitivas de desenho
    ├── player.c/h
    ├── invaders.c/h
    ├── ufo.c/h
    ├── bunker.c/h
    ├── bullet.c/h
    ├── collision.c/h
    ├── font.h
    └── assets/
        ├── sprites.c/h         ← Pixels RGB565
        └── font8x8.c           ← Fonte bitmap 8×8 (CP437)
```

---

## Mapa de memória (Zynq-7000)

| Região | Endereço | Tamanho | Domínio |
|--------|----------|---------|---------|
| Framebuffer 0 | `0x20000000` | 600 KB | DDR (lido pelo AXI VDMA) |
| Framebuffer 1 | `0x20100000` | 600 KB | DDR (lido pelo AXI VDMA) |
| Registrador de controle | `0x40000000` | 4 bytes | AXI-Lite slave (AXI GP0) |
| AXI GPIO (botões) | `0x41200000` | — | AXI-Lite (AXI GP0) |
| Global Timer (Cortex-A9) | `0xF8F00200` | — | PS PERIPHBASE |

> Endereços de `CTRL_REG`, `AXI GPIO` e do(s) framebuffer(s) no VDMA **devem coincidir** com o Address Editor / config do block design no Vivado.

---

## Compilar (SW)

### Pré-requisitos

```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi   # toolchain bare-metal
```

Vivado/Vitis instalado para o fluxo de HW e o deploy via XSCT.

### Build

```bash
make all
```

Gera `space_invaders.elf`. Para binário puro (FSBL / QSPI):

```bash
make space_invaders.bin
```

---

## Carregar na placa (bare-metal via JTAG)

Conectar a Zybo Z7-20 via USB (JTAG), modo de boot em JTAG.

```bash
make deploy
```

Roda `xsct`: programa o bitstream na PL, baixa o `.elf` na DDR e dá run. Ajustar o caminho do bitstream:

```bash
make deploy BITSTREAM=hw/space_invaders.bit
```

Equivalente manual no console XSCT:

```tcl
connect
fpga -file hw/space_invaders.bit
targets -set -filter {name =~ "ARM*#0"}
rst -processor
dow space_invaders.elf
con
```

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

**Frequência do CPU diferente de 667 MHz** — editar `sw/timer.h`:
```c
#define CPU_FREQ_HZ  667000000UL   /* PERIPHCLK = CPU/2 */
```

**Endereço base dos botões (AXI GPIO) diferente** — editar `sw/input.c`:
```c
#define GPIO_PIO_BASE  0x41200000u  /* Address Editor do Vivado */
```

**Endereços do framebuffer / controle** — editar `sw/framebuffer.c`:
```c
#define FRAMEBUFFER_BASE  0x20000000u  /* deve coincidir com o AXI VDMA */
#define CTRL_REG_BASE     0x40000000u  /* AXI-Lite slave na GP0 */
```

---

## VGA — timing

Resolução: **640 × 480 @ 60 Hz**, pixel clock 25,175 MHz (Clocking Wizard a partir do FCLK_CLK0).
Formato em DDR: **RGB565** (16 bpp), layout row-major.
Saída Pmod VGA: **RGB444** — truncamento RGB565 → 4 bits/canal feito na PL (`rgb_output.vhd`).
Double buffering sincronizado por vsync flag no `CTRL_REG` bit 1.
