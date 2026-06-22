# Space Invaders — FPGA (Zynq-7000 / Zybo Z7-20)

Jogo Space Invaders em sistema heterogêneo: lógica do jogo no ARM PS (C99 bare-metal, Cortex-A9), saída de vídeo VGA gerada na FPGA PL (VHDL).

Spec completa: [`space_invaders_fpga_spec.md`](space_invaders_fpga_spec.md)

---

## Plataforma alvo

| Placa | FPGA | DDR | CPU | Toolchain HW | Toolchain SW | Saída VGA |
|-------|------|-----|-----|--------------|--------------|-----------|
| Digilent Zybo Z7-20 | Xilinx XC7Z020 (Zynq-7000) | 1 GB | Cortex-A9 @ 667 MHz | Vivado 2025.2 | Vitis 2025.2 (standalone) | Pmod VGA (RGB444) |

> Versão anterior tinha duplo alvo (DE10-Nano / Zynq). Esta versão é **Zynq-only, Vivado/Vitis-only**.
> A build bare-metal usa a BSP standalone do Vitis (startup + linker gerados pelo Vitis).
> A antiga build raw `arm-none-eabi-gcc` (crt0/linker próprios) está em `legacy-gcc/`.

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
├── legacy-gcc/                 ← Build raw arm-none-eabi-gcc (crt0/linker/Makefile próprios)
└── sw/                         ← Código ARM (C99 bare-metal, importável no Vitis)
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

## Rodar localmente (PC — sem FPGA)

Para desenvolvimento e testes sem hardware, o jogo roda nativamente via SDL2.

### Pré-requisitos

**Windows — MSYS2/MinGW64:**

1. Instalar [MSYS2](https://www.msys2.org)
2. Abrir terminal **"MSYS2 MinGW x64"** e instalar dependências:

```sh
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make
```

### Build

```sh
make -f Makefile.host
```

Gera `space_invaders.exe`.

### Rodar

```sh
./space_invaders.exe
```

### Controles (teclado)

| Tecla | Ação |
|-------|------|
| `←` / `A` | Mover esquerda |
| `→` / `D` | Mover direita |
| `SPACE` | Atirar |
| `P` | Pausar / Retomar |
| `R` | Reiniciar (na tela de Game Over) |
| `Q` / `ESC` | Sair |

### Arquivos host

Os três arquivos em `sw/host/` substituem os módulos de hardware:

| Arquivo host | Substitui | Descrição |
|---|---|---|
| `sw/host/framebuffer_sdl.c` | `sw/framebuffer.c` | Janela SDL2 + pixel buffer RGB565 |
| `sw/host/input_sdl.c` | `sw/input.c` | Teclado via SDL2 |
| `sw/host/timer_host.c` | `sw/timer.c` | `QueryPerformanceCounter` (Windows) |

Todo o código de lógica de jogo (`game.c`, `renderer.c`, `collision.c`, etc.) é compartilhado entre as duas builds sem modificação.

---

## Compilar e rodar na placa (SW — bare-metal via Vitis)

Fluxo principal: importar `sw/` numa Application Component standalone do
**Vitis 2025.2** (BSP gerada a partir do `.xsa` exportado do Vivado).

**Guia completo: [`docs/VITIS.md`](docs/VITIS.md).** Resumo:

1. Vivado: sintetizar o block design, gerar bitstream, `Export Hardware` (com bitstream) → `.xsa`.
2. Vitis: `New Platform Component` a partir do `.xsa`, OS = **standalone**, CPU = `ps7_cortexa9_0`.
3. Vitis: `New Application Component` (Empty C) e importar **só** `sw/*.c|*.h` e `sw/assets/*`.
   Não importar `sw/host/`, `legacy-gcc/`, `crt0.S`, `linker.ld` nem `Makefile*`.
4. Build → `Run As → Launch Hardware` (boot JTAG): programa a PL e baixa o `.elf` na DDR.

> O Vitis fornece startup (`boot.S`/crt0) e `lscript.ld` pela BSP. `fb_swap()` faz
> `Xil_DCacheFlushRange()` no buffer antes do swap (DDR é cacheável no standalone).

### Build legacy raw-gcc (opcional, sem Vitis)

Em `legacy-gcc/` há a build antiga com `arm-none-eabi-gcc` + crt0/linker próprios
e deploy via XSCT. Mantida por referência; o fluxo suportado agora é o Vitis.

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
