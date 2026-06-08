# Space Invaders — FPGA (Cyclone V / Zynq-7000)

Jogo Space Invaders em sistema heterogêneo: lógica do jogo no ARM HPS (C99 bare-metal), saída de vídeo VGA gerada na FPGA (VHDL).

Spec completa: [`space_invaders_fpga_spec.md`](space_invaders_fpga_spec.md)

---

## Plataformas suportadas

| Placa | FPGA | Toolchain HW | Toolchain SW |
|-------|------|--------------|--------------|
| Terasic DE10-Nano | Intel Cyclone V SoC | Quartus Prime Lite | `arm-none-eabi-gcc` |
| Xilinx ZedBoard | Zynq-7000 | Vivado Free | `arm-none-eabi-gcc` |

---

## Estrutura

```
space_invaders/
├── hw/                         ← Projeto FPGA (ainda não implementado)
│   ├── top.vhd
│   ├── vga_sync.vhd
│   ├── framebuffer_reader.vhd
│   ├── rgb_output.vhd
│   ├── ctrl_reg.vhd
│   └── constraints/
└── sw/                         ← Código ARM (C99 bare-metal)
    ├── Makefile
    ├── crt0.S                  ← Startup bare-metal
    ├── linker.ld               ← Linker script (código em 0x01000000)
    ├── main.c
    ├── game.h                  ← Todos os tipos e constantes
    ├── game.c / game_api.h     ← Máquina de estados + renderização
    ├── framebuffer.c/h         ← Acesso direto ao framebuffer (DDR3)
    ├── input.c/h               ← Botões via PIO no LW bridge
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

## Mapa de memória

| Região | Endereço | Tamanho |
|--------|----------|---------|
| Framebuffer 0 | `0x20000000` | 600 KB |
| Framebuffer 1 | `0x20100000` | 600 KB |
| Registrador de controle | `0xFF200000` | 4 bytes |
| PIO de entrada (botões) | `0xFF200060` | 4 bytes |
| Global Timer (Cortex-A9) | `0xFFFEC200` | — |

---

## Compilar (SW)

### Pré-requisitos

```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

### Build

```bash
make all
```

Gera `space_invaders.elf`. Para binário puro (U-Boot / JTAG):

```bash
make space_invaders.bin
```

### Deploy via SSH

```bash
make deploy BOARD_IP=192.168.1.100
```

---

## Carregar na placa

### Via U-Boot

```
tftp 0x01000000 space_invaders.bin
go 0x01000000
```

### Via JTAG (OpenOCD / DS-5)

Carregar `space_invaders.elf` diretamente no endereço `0x01000000`.

---

## Controles

| Botão | Ação |
|-------|------|
| KEY0 | Mover esquerda |
| KEY1 | Mover direita |
| KEY2 | Atirar |
| KEY3 | Pausar / Retomar |

Reiniciar: KEY2 na tela de Game Over.

---

## Ajustes de plataforma

**Frequência do CPU diferente de 800 MHz** — editar `sw/timer.h`:
```c
#define CPU_FREQ_HZ  800000000UL   /* ajustar conforme PLL */
```

**Endereço base dos botões diferente** — editar `sw/input.c`:
```c
#define GPIO_PIO_BASE  0xFF200060u  /* endereço do PIO no Platform Designer */
```

**Endereços do framebuffer** — editar `sw/framebuffer.c`:
```c
#define FRAMEBUFFER_BASE  0x20000000u
#define CTRL_REG_BASE     0xFF200000u
```

---

## VGA — timing

Resolução: **640 × 480 @ 60 Hz**, pixel clock 25,175 MHz.
Formato de pixel: **RGB565** (16 bpp), layout row-major.
Double buffering sincronizado por vsync flag no `CTRL_REG` bit 1.
