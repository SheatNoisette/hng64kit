CROSS ?= mips64-elf-
CC := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

FLAGS := -march=vr4300 -mtune=vr4300 -EB -G0 -mno-abicalls -fno-pic

BIOS_BASE := 0xBFC00000

# 512 KB flash and erased flash reads as 0xFF
BIOS_SIZE := 0x80000
ERASED := 0xFF

BUILD_DIR := build
OBJ := $(BUILD_DIR)/bios.o
ELF := $(BUILD_DIR)/bios.elf
RAW := $(BUILD_DIR)/bios.raw
BIN := $(BUILD_DIR)/brom1.bin

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $@

$(OBJ): bios.S svc.S | $(BUILD_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(FLAGS) -nostdlib -Wl,-Ttext=$(BIOS_BASE) -Wl,-e,reset $< -o $@

$(RAW): $(ELF)
	$(OBJCOPY) -O binary -j .text $< $@

$(BIN): $(RAW)
	$(OBJCOPY) -I binary -O binary --gap-fill=$(ERASED) --pad-to=$(BIOS_SIZE) $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
