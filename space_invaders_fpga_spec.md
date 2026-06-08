# Especificação de Software e Hardware — Space Invaders em FPGA (Zynq / DE10)

**Versão:** 2.0  
**Data:** 2026-06-08  
**Destinatário:** Claude Code  
**Plataformas alvo:** Xilinx Zynq-7000 (ARM Cortex-A9) · Intel/Terasic DE10-Nano (Cyclone V SoC, ARM Cortex-A9)  
**Paradigma:** Sistema heterogêneo — lógica do jogo no ARM (HPS), saída de vídeo VGA na FPGA (PL)

---

## 1. Visão Geral do Sistema

O jogo é dividido em dois domínios cooperantes:

| Domínio | Responsável | Tecnologia |
|---------|-------------|------------|
| **HPS (ARM)** | Lógica do jogo, input, áudio (opcional) | C99, Linux embarcado ou bare-metal |
| **PL (FPGA)** | Geração de sinal VGA, leitura do framebuffer | VHDL ou Verilog (ou IP Wizard) |

O ARM escreve pixels no **framebuffer** em RAM compartilhada. A FPGA lê esse framebuffer via barramento **AXI** (Zynq) ou **FPGA-to-HPS bridge** (DE10) e gera os sinais VGA (HSYNC, VSYNC, R, G, B) em tempo real.

```
┌─────────────────────────────────────────┐
│                  SoC                    │
│                                         │
│  ┌──────────┐    AXI/HPS-FPGA Bridge    │
│  │  ARM HPS │ ──────────────────────►   │
│  │  (jogo)  │    Framebuffer (DDR3)     │
│  └──────────┘          │               │
│         ▲              ▼               │
│         │    ┌──────────────────┐      │
│  GPIO / │    │   PL (FPGA)      │      │
│  UART   │    │  VGA Controller  │──► VGA
│  USB HID│    │  Pixel Readout   │      │
│         │    └──────────────────┘      │
└─────────────────────────────────────────┘
```

---

## 2. Especificação de Hardware (FPGA — PL)

### 2.1 Resolução e Timing VGA

Resolução alvo: **640 × 480 @ 60 Hz** (padrão VGA, pixel clock 25,175 MHz).

| Parâmetro | Valor |
|-----------|-------|
| Pixel clock | 25,175 MHz (PLL a partir do clock da placa) |
| Pixels visíveis por linha | 640 |
| Front porch H | 16 ciclos |
| Sync pulse H | 96 ciclos |
| Back porch H | 48 ciclos |
| Linhas visíveis | 480 |
| Front porch V | 10 linhas |
| Sync pulse V | 2 linhas |
| Back porch V | 33 linhas |
| Polaridade HSYNC / VSYNC | Negativa (ativo baixo) |

### 2.2 Formato do Framebuffer

- **Localização:** DDR3 SDRAM — endereço base configurável via constante (`FRAMEBUFFER_BASE`).
- **Formato de pixel:** `RGB565` (16 bits por pixel) — equilibra uso de memória e qualidade visual.
- **Tamanho:** 640 × 480 × 2 bytes = **614.400 bytes (~600 KB)**.
- **Layout:** linear, row-major — pixel (x, y) no offset `(y × 640 + x) × 2`.
- **Double buffering:** dois framebuffers alternados; o ARM sinaliza troca via registrador de controle mapeado em memória (endereço `CTRL_REG`).

```
Endereços sugeridos (ajustar ao memory map da plataforma):
  FRAMEBUFFER_0:  0x20000000  (600 KB)
  FRAMEBUFFER_1:  0x20100000  (600 KB)
  CTRL_REG:       0xFF200000  (word de 32 bits)
    bit 0: buffer ativo (0 = FB0, 1 = FB1)
    bit 1: vsync flag (escrito pela FPGA, lido pelo ARM)
```

### 2.3 Módulos VHDL/Verilog

#### 2.3.1 `vga_sync.vhd` — Gerador de Sincronismo

Responsável por gerar os contadores de pixel e linha, e os pulsos HSYNC/VSYNC.

```vhdl
-- Entradas: clk_25mhz, reset
-- Saídas:   hsync, vsync, h_count (10 bits), v_count (10 bits), video_on
entity vga_sync is
  port (
    clk      : in  std_logic;
    reset    : in  std_logic;
    hsync    : out std_logic;
    vsync    : out std_logic;
    h_count  : out std_logic_vector(9 downto 0);
    v_count  : out std_logic_vector(9 downto 0);
    video_on : out std_logic   -- '1' apenas na área visível
  );
end entity;
```

#### 2.3.2 `framebuffer_reader.vhd` — Leitor do Framebuffer

Lê o pixel correspondente a (h_count, v_count) do framebuffer ativo via AXI Master (Zynq) ou Avalon Master (DE10).

- Calcula endereço: `base + (v_count × 640 + h_count) × 2`
- Latência de leitura deve ser compensada com pipeline (prefetch de 1–2 pixels).
- Para Zynq: instanciar IP **AXI VDMA** (Video DMA) — elimina a necessidade de lógica de leitura manual.
- Para DE10: usar **Avalon-MM Master** com burst read; considerar uso do IP **VGA Controller** do Platform Designer.

#### 2.3.3 `rgb_output.vhd` — Saída de Cor

Converte RGB565 (lido do framebuffer) para os pinos físicos do DAC VGA da placa.

```vhdl
-- RGB565 → pinos VGA (resistor ladder DAC)
-- Zynq ZedBoard:  VGA via Pmod VGA ou HDMI (TMDS)
-- DE10-Nano:      VGA via conector DB-15 com resistor ladder 4-bit por canal
--                 (apenas 4 bits por canal disponíveis → truncar RGB565)
red   <= pixel_rgb565(15 downto 11);   -- 5 bits
green <= pixel_rgb565(10 downto 5);    -- 6 bits → usar 5 MSBs se DAC for 5-bit
blue  <= pixel_rgb565(4  downto 0);    -- 5 bits
```

#### 2.3.4 `ctrl_reg.vhd` — Registrador de Controle

Registrador AXI-Lite (Zynq) ou Avalon-MM Slave (DE10) mapeado em memória para comunicação ARM↔FPGA:

- Bit 0 (escrita ARM): seleciona framebuffer ativo.
- Bit 1 (escrita FPGA): pulso de vsync — o ARM usa para sincronizar a troca de buffer.

#### 2.3.5 `top.vhd` — Módulo Top-Level

Instancia e conecta todos os módulos acima + PLL para geração de 25,175 MHz.

### 2.4 PLL / Clock

- **Zynq:** usar o IP **Clocking Wizard** — entrada 125 MHz (FCLK), saída 25,175 MHz para o domínio VGA.
- **DE10-Nano:** usar o IP **ALTPLL** — entrada 50 MHz do oscilador, saída 25,175 MHz.

### 2.5 Conexão Física dos Pinos

#### DE10-Nano — VGA via GPIO
| Sinal | Pino FPGA | Observação |
|-------|-----------|------------|
| VGA_R[3:0] | GPIO_0[0:3] | DAC resistor ladder |
| VGA_G[3:0] | GPIO_0[4:7] | DAC resistor ladder |
| VGA_B[3:0] | GPIO_0[8:11] | DAC resistor ladder |
| VGA_HS | GPIO_0[12] | HSYNC |
| VGA_VS | GPIO_0[13] | VSYNC |

> Referência: esquemático Terasic DE10-Nano com daughterboard VGA, ou adaptador VGA para GPIO disponível na comunidade Terasic.

#### Zynq ZedBoard — VGA via Pmod
| Sinal | Pmod JB/JC | Observação |
|-------|------------|------------|
| VGA_R[3:0] | JB[0:3] | usar Pmod VGA Digilent |
| VGA_G[3:0] | JB[4:7] | |
| VGA_B[3:0] | JC[0:3] | |
| VGA_HS | JC[4] | |
| VGA_VS | JC[5] | |

---

## 3. Especificação de Software (ARM — HPS)

### 3.1 Ambiente de Execução

| Item | Recomendado | Alternativa |
|------|-------------|-------------|
| SO | Linux embarcado (Yocto / buildroot) | Bare-metal (sem SO) |
| Toolchain | `arm-linux-gnueabihf-gcc` (Linux) | `arm-none-eabi-gcc` (bare-metal) |
| Padrão C | C99 | C11 |
| Bibliotecas | libc padrão, `libm` | Nenhuma (bare-metal) |
| Input | `/dev/input/eventX` (USB HID) ou GPIO | GPIO direto via `mmap` |

> **Recomendação:** usar Linux embarcado — simplifica acesso ao framebuffer, USB HID e toolchain.

### 3.2 Acesso ao Framebuffer pelo ARM

No Linux, o framebuffer é acessado via `mmap` sobre `/dev/mem`:

```c
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

#define FRAMEBUFFER_BASE  0x20000000
#define FRAMEBUFFER_SIZE  (640 * 480 * 2)
#define CTRL_REG_BASE     0xFF200000

static uint16_t *fb[2];    /* dois framebuffers */
static uint32_t *ctrl_reg;
static int       active_fb = 0;

void fb_init(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);

    fb[0] = mmap(NULL, FRAMEBUFFER_SIZE, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, FRAMEBUFFER_BASE);
    fb[1] = mmap(NULL, FRAMEBUFFER_SIZE, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE);
    ctrl_reg = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, CTRL_REG_BASE);
    close(fd);
}

/* Escreve pixel no back buffer */
static inline void put_pixel(int x, int y, uint16_t color) {
    fb[1 - active_fb][y * 640 + x] = color;
}

/* Troca buffers (aguarda vsync) */
void fb_swap(void) {
    while (!(*ctrl_reg & 0x2));      /* aguarda vsync flag da FPGA */
    active_fb = 1 - active_fb;
    *ctrl_reg = (*ctrl_reg & ~0x1) | active_fb;   /* sinaliza novo buffer */
    *ctrl_reg &= ~0x2;               /* limpa vsync flag */
}
```

### 3.3 Primitivas de Desenho

Implementar em `renderer.c`:

```c
/* Converte RGB888 → RGB565 */
static inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void draw_rect(int x, int y, int w, int h, uint16_t color);
void draw_char(int x, int y, char c, uint16_t fg, uint16_t bg);
void draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg);
void draw_sprite(int x, int y, const Sprite *spr);
void clear_screen(uint16_t color);
```

### 3.4 Sistema de Sprites

Sprites definidos como arrays de pixels RGB565 (ou bitmaps 1-bit com cor parametrizada):

```c
typedef struct {
    int      width, height;
    uint16_t pixels[];   /* row-major, RGB565 */
} Sprite;

/* Exemplo: nave do jogador 13×8 pixels */
extern const Sprite SPRITE_PLAYER;
extern const Sprite SPRITE_INVADER_A[2];  /* 2 frames de animação */
extern const Sprite SPRITE_INVADER_B[2];
extern const Sprite SPRITE_INVADER_C[2];
extern const Sprite SPRITE_UFO;
extern const Sprite SPRITE_BULLET_PLAYER;
extern const Sprite SPRITE_BULLET_ENEMY;
extern const Sprite SPRITE_EXPLOSION;
```

Sprites podem ser definidos em `assets/sprites.c` como arrays estáticos em C ou carregados de arquivo (se Linux com sistema de arquivos disponível).

### 3.5 Fonte Bitmap

- Usar fonte **8×8 pixels** (ex.: CP437 bitmap font, domínio público).
- Armazenada como array `uint8_t font8x8[128][8]` em `assets/font8x8.c`.
- `draw_char()` usa a fonte para renderizar texto no framebuffer.

### 3.6 Input

#### Linux (recomendado) — USB HID Keyboard

```c
#include <linux/input.h>

int input_init(void) {
    return open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
}

void input_poll(int fd, InputState *state) {
    struct input_event ev;
    while (read(fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_KEY) {
            if (ev.code == KEY_LEFT  || ev.code == KEY_A) state->left  = (ev.value != 0);
            if (ev.code == KEY_RIGHT || ev.code == KEY_D) state->right = (ev.value != 0);
            if (ev.code == KEY_SPACE)                     state->fire  = (ev.value == 1);
            if (ev.code == KEY_P)                         state->pause = (ev.value == 1);
            if (ev.code == KEY_Q)                         state->quit  = (ev.value == 1);
            if (ev.code == KEY_R)                         state->reset = (ev.value == 1);
        }
    }
}
```

#### Bare-metal / GPIO — Botões da Placa

- Mapear botões físicos (KEY0–KEY3 no DE10-Nano) via `mmap` nos registradores GPIO do HPS.
- KEY0 = esquerda, KEY1 = direita, KEY2 = fire, KEY3 = pause.

```c
#define HPS_GPIO_BASE  0xFF200060
volatile uint32_t *gpio = (uint32_t *)HPS_GPIO_BASE;
#define BTN_LEFT   (1 << 0)
#define BTN_RIGHT  (1 << 1)
#define BTN_FIRE   (1 << 2)
```

---

## 4. Lógica do Jogo (ARM)

Esta seção é equivalente à especificação original — a lógica de jogo é **independente de plataforma**.

### 4.1 Resolução Lógica × Física

A área de jogo usa coordenadas em **pixels físicos** (640×480). As entidades têm posição e dimensão em pixels:

| Entidade | Largura | Altura | Posição Y inicial |
|----------|---------|--------|-------------------|
| Jogador | 13 | 8 | 440 |
| Alienígena tipo A | 12 | 8 | 80–120 |
| Alienígena tipo B | 11 | 8 | 136–176 |
| Alienígena tipo C | 8 | 8 | 192 |
| UFO | 16 | 7 | 40 |
| Projétil jogador | 3 | 8 | — |
| Projétil inimigo | 3 | 8 | — |
| Bunker | 36 | 24 | 380 |

Espaçamento da grade de alienígenas: 16 px horizontal, 16 px vertical.

### 4.2 Estruturas de Dados

```c
typedef struct { int x, y; } Vec2;

typedef struct {
    uint16_t pixels[16 * 16];   /* sprite max 16×16 */
    int      w, h;
} Sprite;

typedef struct {
    Vec2  pos;
    int   active;
    int   dy;          /* -1 = sobe (jogador), +1 = desce (inimigo) */
    int   dx;          /* para projéteis diagonais futuros */
} Bullet;

typedef struct {
    Vec2   pos;
    int    lives;
    Bullet bullet;
    int    invincible_frames;   /* frames de imunidade após ser atingido */
} Player;

typedef struct {
    Vec2 pos;
    int  alive;
    int  type;         /* 0=A, 1=B, 2=C */
    int  anim_frame;   /* 0 ou 1 — alterna a cada passo da frota */
} Invader;

typedef struct {
    Invader grid[5][11];
    Vec2    origin;    /* posição do canto superior esquerdo da grade */
    int     dir;       /* +1 = direita, -1 = esquerda */
    int     step_px;   /* pixels por passo (aumenta com o nível) */
    int     alive_count;
    float   step_interval;  /* segundos entre passos */
    float   step_timer;
    Bullet  bullet;
    float   shoot_timer;
    float   shoot_interval;
} Fleet;

typedef struct {
    Vec2  pos;
    int   active;
    int   dx;
    int   points;
    float spawn_timer;
} UFO;

/* Bunker: grade de células 3×3 pixels cada, 4 colunas × 4 linhas = 36×24 px */
#define BUNKER_COLS 12
#define BUNKER_ROWS 8
typedef struct {
    Vec2 origin;
    uint8_t cells[BUNKER_ROWS][BUNKER_COLS];  /* 1=intacto, 0=destruído */
} Bunker;

typedef enum { STATE_TITLE, STATE_PLAYING, STATE_PAUSED,
               STATE_NEXT_LEVEL, STATE_GAME_OVER } GameState_e;

typedef struct {
    Player     player;
    Fleet      fleet;
    UFO        ufo;
    Bunker     bunkers[4];
    int        score;
    int        high_score;
    int        level;
    GameState_e state;
    float      state_timer;   /* para telas temporárias */
    InputState input;
} Game;
```

### 4.3 Mecânicas

Idênticas à especificação v1.0, seções 2.2–2.9, adaptadas às coordenadas em pixels:

- **Movimento do jogador:** 2 px/frame (limitado entre x=8 e x=619).
- **Projétil do jogador:** 4 px/frame para cima.
- **Projétil inimigo:** 3 px/frame para baixo.
- **Frota:** passo horizontal de `step_px` pixels a cada `step_interval` segundos.
  - `step_px` inicial = 2; `step_interval` inicial = 0,5 s.
  - A cada alienígena eliminado: `step_interval` reduz 0,008 s (mínimo 0,05 s).
- **UFO:** 1 px/frame, spawn a cada 25–30 s aleatórios.
- **Detecção de colisão:** AABB (Axis-Aligned Bounding Box) em pixels.
- **Bunker:** cada `cell[r][c]` é destruída por qualquer projétil que sobreponha sua área `(origin.x + c*3, origin.y + r*3, 3, 3)`.

### 4.4 Loop Principal

```c
#define TARGET_FPS     60
#define FRAME_US       (1000000 / TARGET_FPS)

void game_loop(Game *g) {
    struct timeval t0, t1;
    while (g->state != STATE_QUIT) {
        gettimeofday(&t0, NULL);

        input_poll(g->input_fd, &g->input);
        game_update(g, 1.0f / TARGET_FPS);   /* delta fixo */
        game_render(g);
        fb_swap();

        gettimeofday(&t1, NULL);
        long elapsed = (t1.tv_sec - t0.tv_sec) * 1000000L
                     + (t1.tv_usec - t0.tv_usec);
        if (elapsed < FRAME_US)
            usleep(FRAME_US - elapsed);
    }
}
```

---

## 5. Arquitetura de Arquivos

```
space_invaders_fpga/
│
├── hw/                          ← Projeto FPGA
│   ├── top.vhd                  ← Top-level
│   ├── vga_sync.vhd             ← Gerador de sincronismo VGA
│   ├── framebuffer_reader.vhd   ← Leitura do framebuffer via AXI/Avalon
│   ├── rgb_output.vhd           ← Conversão RGB565 → pinos VGA
│   ├── ctrl_reg.vhd             ← Registrador de controle ARM↔FPGA
│   ├── pll_25mhz.vhd            ← PLL 25,175 MHz (gerado pelo IP Wizard)
│   ├── constraints/
│   │   ├── de10nano.qsf         ← Pin assignments DE10-Nano (Quartus)
│   │   └── zynq_zedboard.xdc    ← Constraints Zynq (Vivado)
│   └── platform_designer/       ← Arquivo .qsys (DE10) ou block design (Zynq)
│
├── sw/                          ← Projeto ARM (C)
│   ├── Makefile
│   ├── main.c                   ← Ponto de entrada, init, game_loop
│   ├── game.c / game.h          ← Estado global, init/reset
│   ├── player.c / player.h
│   ├── invaders.c / invaders.h
│   ├── ufo.c / ufo.h
│   ├── bunker.c / bunker.h
│   ├── bullet.c / bullet.h
│   ├── collision.c / collision.h
│   ├── renderer.c / renderer.h  ← Primitivas de desenho no framebuffer
│   ├── framebuffer.c / framebuffer.h ← mmap, swap, put_pixel
│   ├── input.c / input.h        ← USB HID ou GPIO
│   ├── font.c / font.h          ← Fonte bitmap 8×8
│   └── assets/
│       ├── sprites.c / sprites.h ← Sprites RGB565
│       ├── font8x8.c             ← Bitmap da fonte
│       └── highscore.txt         ← Score persistido (opcional)
│
└── README.md
```

---

## 6. Makefile (Cross-compilation)

```makefile
# Cross-compiler para ARM Cortex-A9 com Linux
CC      = arm-linux-gnueabihf-gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard
LIBS    = -lm
SRC     = $(wildcard sw/src/*.c)
TARGET  = space_invaders

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Deploy via SSH para a placa (ajustar IP)
deploy:
	scp $(TARGET) root@192.168.1.100:/home/root/

clean:
	rm -f $(TARGET)

.PHONY: all deploy clean
```

Para bare-metal, substituir por `arm-none-eabi-gcc` e adicionar linker script (`-T linker.ld`) apontando para endereço de RAM do HPS.

---

## 7. Fluxo de Desenvolvimento Recomendado

```
Fase 1 — FPGA (hw/)
  1.1  Implementar vga_sync.vhd e testar em simulação (ModelSim/GHDL)
  1.2  Testar output VGA com padrão de barras de cores (sem ARM)
  1.3  Implementar ctrl_reg.vhd e framebuffer_reader.vhd
  1.4  Síntese e P&R (Quartus / Vivado) — verificar timing @ 25,175 MHz
  1.5  Testar leitura do framebuffer com dados estáticos escritos pelo ARM

Fase 2 — Software (sw/)
  2.1  Implementar framebuffer.c (mmap, put_pixel, fb_swap) no PC com display falso
  2.2  Implementar renderer.c (primitivas, sprites, fonte) com testes unitários
  2.3  Implementar lógica do jogo (game.c e módulos) — testar desacoplado do HW
  2.4  Integrar input (USB HID ou GPIO)

Fase 3 — Integração
  3.1  Gravar bitstream na FPGA e iniciar Linux no ARM
  3.2  Copiar binário para a placa, executar
  3.3  Verificar sincronismo de buffer, tearing, latência de input
  3.4  Ajuste fino de velocidades e dificuldade
```

---

## 8. Pontuação e HUD

Idêntico à especificação v1.0. Renderizado via `draw_string()` nas últimas 16 linhas da tela (y = 456–472):

```
Score: 012345   Hi: 099999   Lives: ♥♥♥   Level: 04
```

---

## 9. Telas do Jogo

Idênticas à v1.0 (Título, Jogo, Pausa, Game Over, Próximo Nível), agora renderizadas diretamente no framebuffer via `draw_sprite()` e `draw_string()`.

---

## 10. Considerações por Plataforma

### DE10-Nano (Intel Cyclone V SoC)

- Ferramenta FPGA: **Intel Quartus Prime** (versão Lite, gratuita).
- Soft IP de vídeo: **VGA Controller** no Qsys/Platform Designer (Intel).
- Bridge HPS→FPGA: **FPGA-to-HPS** ou **Lightweight HPS-to-FPGA** AXI Bridge.
- Endereço base das bridges: `0xC0000000` (FPGA-to-HPS, 960 MB) ou `0xFF200000` (LW, 2 MB).
- Referência: *DE10-Nano User Manual*, Terasic; *Intel SoC FPGA Embedded Design Suite User Guide*.

### Zynq-7000 (Xilinx/AMD)

- Ferramenta FPGA: **Vivado Design Suite** (versão gratuita para Z-7010/Z-7020).
- Soft IP de vídeo: **AXI VDMA** + **AXI4-Stream to Video Out** (Xilinx IP Catalog).
- Conexão PS↔PL: via **AXI HP ports** (alta banda, para VDMA) ou **AXI GP ports** (para registradores de controle).
- Endereço da PL no PS: `0x40000000`–`0x7FFFFFFF` (AXI GP0).
- Referência: *Zynq-7000 TRM* (UG585); *AXI VDMA Product Guide* (PG020).

---

## 11. Critérios de Aceitação

### Hardware (FPGA)
- [ ] Sinal VGA estável 640×480@60Hz medido em monitor (sem glitches de sync).
- [ ] Barras de cores renderizadas corretamente antes da integração com o ARM.
- [ ] Leitura do framebuffer sem artefatos visíveis (pixel correto em posição correta).
- [ ] Troca de buffer (double buffering) sem tearing visível.
- [ ] Projeto sintetiza sem erros de timing no relatório do Quartus/Vivado.

### Software (ARM)
- [ ] Compila sem warnings com o Makefile fornecido.
- [ ] `put_pixel` e `draw_sprite` produzem resultado correto no framebuffer.
- [ ] Lógica do jogo opera identicamente à especificação v1.0.
- [ ] Input responde em ≤ 2 frames de latência.
- [ ] Loop mantém 60 FPS estáveis (verificar com contador de frames no HUD).
- [ ] Sem vazamento de memória (`mmap` liberado corretamente no `munmap`).

### Integração
- [ ] Jogo completo jogável no monitor VGA conectado à placa.
- [ ] Sem corrupção visual durante troca de buffer.
- [ ] Score, vidas e nível atualizados em tempo real no HUD.
- [ ] Game Over e reinício funcionam sem reinicializar a placa.

---

## 12. Entregáveis

1. **`hw/`** — Projeto FPGA completo (sintetizável no Quartus Lite ou Vivado Free).
2. **`sw/`** — Código-fonte C completo com Makefile de cross-compilation.
3. **`README.md`** — Instruções de síntese FPGA, cross-compilation do ARM, deploy e execução.
4. **Bitstream pré-compilado** (`.sof` para DE10-Nano, `.bit` para Zynq) — opcional.
5. Código comentado nos pontos de interface HW/SW (mmap, AXI addressing, vsync sync).

---

## 13. Referências

- Terasic DE10-Nano User Manual — https://www.terasic.com.tw
- Intel Cyclone V Hard Processor System Technical Reference Manual
- AMD Zynq-7000 SoC Technical Reference Manual (UG585)
- VGA Signal Timing Standard — VESA DMT
- Digilent Pmod VGA Reference Manual (para ZedBoard)
- CP437 Font Bitmap (domínio público) — https://github.com/dhepper/font8x8

---

*Fim da especificação — versão 2.0.*
