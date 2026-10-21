CC := wcc
AS := wasm
LD := wlink

# V53A is using the 80186 set with no stack checks and no DOS libs
CFLAGS := -q -1 -ms -s -zl -os -wx
ASFLAGS := -q

BUILD_DIR := build
OBJS := $(BUILD_DIR)/crt0.o $(BUILD_DIR)/main.o
BIN := $(BUILD_DIR)/soundfw.bin
MAP := $(BUILD_DIR)/soundfw.map

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/crt0.o: crt0.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -fo=$@ $<

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fo=$@ $<

$(BIN): soundfw.lnk $(OBJS)
	$(LD) @soundfw.lnk name $@ option map=$(MAP) $(foreach o,$(OBJS),file $(o))

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
