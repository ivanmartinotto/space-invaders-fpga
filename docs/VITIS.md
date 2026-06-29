# Importar no Vitis (Vivado 2025.2) — bare-metal Zybo Z7-20

Guia para rodar o software (`sw/`) bare-metal no PS (Cortex-A9) usando o
Vitis Unified IDE 2025.2. O Vitis fornece o startup (`boot.S`/crt0), o linker
script (`lscript.ld`) e a BSP standalone — por isso os arquivos `crt0.S` e
`linker.ld` da build raw-gcc foram movidos para `legacy-gcc/` e **não** devem
ser importados aqui (causariam `_start` duplicado e conflito de linker).

## Pré-requisitos

- Vivado 2025.2 com o block design do `hw/` sintetizado.
- Bitstream gerado e **hardware exportado** como `.xsa`
  (`File → Export → Export Hardware → Include bitstream`).

O `.xsa` precisa conter, no Address Editor / block design:

| Bloco | Endereço esperado pelo SW | Onde ajustar no SW |
|-------|---------------------------|--------------------|
| Framebuffer 0 (DDR, lido pelo VDMA, **32 bpp**) | `0x2000_0000` | `sw/framebuffer.c` |
| Framebuffer 1 (DDR, lido pelo VDMA, **32 bpp**) | `0x2012_C000` (`+SCREEN_W*SCREEN_H*4`) | `sw/framebuffer.c` |
| AXI VDMA (S_AXI_LITE, GP0) | `0x4300_0000` (`XPAR_AXI_VDMA_0_BASEADDR`) | `sw/framebuffer.c` |
| AXI GPIO botões (GP0) | `0x4120_0000` | `sw/input.c` |

> **Não há CTRL_REG.** O design Vivado (`vivado/build_hdmi.tcl`) usa AXI VDMA +
> rgb2dvi (HDMI). O framebuffer é **XRGB8888** (`0x00RRGGBB`, 32 bpp) — o jogo
> continua em RGB565 e a conversão acontece em `put_pixel`/`get_pixel`. O double
> buffer e o vsync são feitos pelo VDMA em modo *park* (ver notas abaixo).

> Se o seu Address Editor diferir, edite os `#define ...BASE` nesses arquivos
> (ou troque pelos `XPAR_*` de `xparameters.h`, gerado pela BSP).

## Passo a passo

1. **Platform Component (BSP)**
   - `File → New → Platform Component`.
   - Hardware: selecione o `.xsa` exportado do Vivado.
   - OS: **standalone**. CPU: **ps7_cortexa9_0**.
   - Build do platform (gera a BSP com `xil_io.h`, `xil_cache.h`,
     `xtime_l.h`, `sleep.h` etc.).

2. **Application Component**
   - `File → New → Application Component`, vinculado ao platform acima.
   - Template: **Empty Application (C)**.

3. **Importar as fontes**
   - No `src/` da application, importe (Import Sources) **somente**:
     - `sw/*.c` e `sw/*.h`
     - `sw/assets/*.c` e `sw/assets/*.h`
   - **NÃO** importe: `sw/host/`, `legacy-gcc/`, `crt0.S`, `linker.ld`,
     `Makefile*`. (O Vitis gera startup + `lscript.ld`.)
   - Em Build Settings adicione o include dir do `src/` (ou de `sw/`) para os
     headers serem achados (`-Isrc`).

4. **Linker — colocação na DDR**
   - O `lscript.ld` gerado coloca código/heap/stack no início da DDR.
   - Os framebuffers ficam em `0x2000_0000+` (512 MB), fora do código — a Zybo
     Z7-20 tem 1 GB de DDR, então não há colisão. Garanta que o heap/stack do
     `lscript.ld` não cresça até 512 MB (defaults estão muito abaixo disso).

5. **Build** (martelo). Gera o `.elf`.

## Rodar / debug

- Modo de boot da Zybo em **JTAG**, conectada por USB.
- `Run/Debug As → Launch Hardware`: o Vitis programa o bitstream na PL,
  baixa o `.elf` na DDR e roda.
- Console: `xil_printf` aparece no terminal serial (UART1, 115200) se você
  adicionar logs.

## Notas de hardware que o SW assume

- **Coerência de cache:** `fb_swap()` faz `Xil_DCacheFlushRange()` no buffer
  recém-desenhado antes do swap, porque a DDR é cacheável no BSP standalone.
  Não remova isso — sem o flush o VDMA lê pixels velhos.
- **Double buffer + vsync (AXI VDMA, modo park):** `fb_init()` configura o canal
  MM2S do VDMA com 2 frame stores e o coloca em modo *park* (não-circular).
  `fb_swap()` aponta `PARK_PTR_REG[4:0]` para o back buffer e espera o flag de
  fim-de-frame (`MM2S_VDMASR` bit 12, *write-1-clear*) como vsync. O VDMA precisa
  ter `c_num_fstores = 2` no block design (já está no `build_hdmi.tcl`).
- **Pixel 32 bpp:** o framebuffer é `0x00RRGGBB`. Se as cores aparecerem trocadas
  no monitor, ajuste o `TDATA_REMAP` do `axis_subset_converter_0` no
  `build_hdmi.tcl` (não mexa na conversão do software).
- **Timer:** `timer.c` usa `XTime_GetTime`/`COUNTS_PER_SECOND` da BSP, então a
  frequência do PS é pega automaticamente (não depende de `CPU_FREQ_HZ`).
