CC      = arm-none-eabi-gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard \
          -Isw -ffreestanding
LDFLAGS = -T sw/linker.ld -nostartfiles --specs=nosys.specs
LIBS    = -lm -lc -lgcc
SRC     = $(wildcard sw/*.c) $(wildcard sw/assets/*.c)
ASRC    = sw/crt0.S
TARGET  = space_invaders.elf
BOARD_IP ?= 192.168.1.100

all: $(TARGET)

$(TARGET): $(SRC) $(ASRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

# Gera binário puro para carregar via JTAG ou U-Boot "go"
$(TARGET:.elf=.bin): $(TARGET)
	arm-none-eabi-objcopy -O binary $< $@

deploy: $(TARGET)
	scp $(TARGET) root@$(BOARD_IP):/home/root/

clean:
	rm -f $(TARGET) $(TARGET:.elf=.bin)

.PHONY: all deploy clean
