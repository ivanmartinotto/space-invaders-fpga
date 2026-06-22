# Build legacy raw-gcc (sem Vitis/BSP). Rode da raiz do projeto:
#   make -f legacy-gcc/Makefile
# A camada de HW (framebuffer/timer/input) vem desta pasta — versoes com MMIO
# direto, sem dependencia da BSP do Vitis. O Vitis e o fluxo suportado; veja
# docs/VITIS.md. As demais fontes sao compartilhadas com sw/.
CC      = arm-none-eabi-gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard \
          -Isw -ffreestanding
LDFLAGS = -T legacy-gcc/linker.ld -nostartfiles --specs=nosys.specs
LIBS    = -lm -lc -lgcc

# Fontes compartilhadas de sw/, exceto a camada de HW (sobrescrita aqui).
SHARED  = $(filter-out sw/framebuffer.c sw/timer.c sw/input.c, \
            $(wildcard sw/*.c)) $(wildcard sw/assets/*.c)
HWLAYER = legacy-gcc/framebuffer.c legacy-gcc/timer.c legacy-gcc/input.c
SRC     = $(SHARED) $(HWLAYER)
ASRC    = legacy-gcc/crt0.S
TARGET  = space_invaders.elf
# Bitstream do projeto Vivado (programado antes do .elf)
BITSTREAM ?= hw/space_invaders.bit

all: $(TARGET)

$(TARGET): $(SRC) $(ASRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

# Gera binário puro (opcional — para FSBL/QSPI ou "stop; dow")
$(TARGET:.elf=.bin): $(TARGET)
	arm-none-eabi-objcopy -O binary $< $@

# Deploy bare-metal no Zynq via JTAG usando XSCT (Vivado/Vitis).
# Carrega o bitstream na PL, baixa o .elf na DDR e dá run.
deploy: $(TARGET)
	xsct -eval "connect; \
	            fpga -file $(BITSTREAM); \
	            targets -set -filter {name =~ \"ARM*#0\"}; \
	            rst -processor; \
	            dow $(TARGET); \
	            con"

clean:
	rm -f $(TARGET) $(TARGET:.elf=.bin)

.PHONY: all deploy clean
