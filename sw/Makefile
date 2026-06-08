CC      = arm-none-eabi-gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard \
          -Isw -ffreestanding
LDFLAGS = -T sw/linker.ld -nostartfiles --specs=nosys.specs
LIBS    = -lm -lc -lgcc
SRC     = $(wildcard sw/*.c) $(wildcard sw/assets/*.c)
ASRC    = sw/crt0.S
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
