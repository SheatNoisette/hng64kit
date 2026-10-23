# Hyper Neo Geo 64 BIOS

> @TODO: Document log service menu and misc stuff.

The BIOS is the program that's run at the start of the board, it is contained
as "BROM1.bin" for MAME.

## Jump table

Bios stage starts with a jump table of 33 slots at 0x9FC08004
After loading a game, the BIOS copies the stage to RAM to `0x80004000-0x80007FFF`
Then, programs call slot `n` at `0x80004000 + (*(u32 *)(0x80004000 + n) & 0x7FFF)`.


| Slot           | Address                    | What is this                                                                                  | Used by the BIOS                     | Used by games                                                             |
|----------------|----------------------------|-----------------------------------------------------------------------------------------------|--------------------------------------|---------------------------------------------------------------------------|
| `0x04`         | `0x9FC08148`               | Text video init                                                                               | Yes                                  | `roadedge`, `xrally`, `bbust2`, `buriki`                                  |
| `0x08`         | `0x9FC08200`               | System init, then clear tilemap 0                                                             | No                                   | ?                                                                         |
| `0x0C`         | `0x9FC08228`               | Print, register calling conv (`at`, `v0`, `s7`)                                               | Yes                                  | No                                                                        |
| `0x10`         | `0x9FC082A4`               | I/O MCU init                                                                                  | Yes                                  | No                                                                        |
| `0x14`         | `0x9FC083CC`               | System Log Reader                                                                             | Yes, hidden mode                     | No                                                                        |
| `0x18-0x24`    | `0x9FC08900`...            | Test ? Error reporter                                                                         | Internal                             | No                                                                        |
| `0x28-0x44`    | `0x9FC093A4`...            | POST tests                                                                                    | POST lists                           | Through `0x5C`                                                            |
| `0x48`         | `0x9FC09988`               | Run a test list                                                                               | Yes                                  | No                                                                        |
| `0x4C`         | `0x9FC09300`               | System init stuff K0 = 3, TLB, 3D buffer control                                              | Yes                                  | All five                                                                  |
| `0x50`         | `0x9FC09474`               | Print the banner                                                                              | Yes                                  | No                                                                        |
| `0x54`         | `0x9FC09938`               | Crosshatch                                                                                    | Yes                                  | No                                                                        |
| `0x58`         | `0x9FC08270`               | Print with C arguments `a0` string, `a1` font byte offset, `a2` tilemap offset, `a3` palette  | No                                   | `roadedge`, `xrally`, `bbust2`, `buriki`, most in `roadedge` and `bbust2` |
| `0x5C`         | `0x9FC09A3C`               | Run one POST test `a1`, quiet flag `a2` (seems to be 1?), return error count                  | No                                   | All, always with `a2 = 1`                                                 |
| `0x60`         | `0x9FC082F0`               | I/O MCU handshake                                                                             | Yes                                  | No                                                                        |
| `0x64`         | `0x9FC09A84`               | DMA copy                                                                                      | Yes                                  | No                                                                        |
| `0x68`, `0x6C` | `0x9FC09B04`, `0x9FC09B24` | Cache on, cache flush                                                                         | Internal                             | No                                                                        |
| `0x70`         | `0x9FC098C4`               | Network dual-port test                                                                        | POST list                            | No                                                                        |
| `0x74`         | `0x9FC09B84`               | RTC init: CF = 3 (RESET, STOP), CE = 0, CD = 0, CF = 4 (24-hour, run)                         | Yes                                  | No                                                                        |
| `0x78`         | `0x9FC09BB0`               | Read the RTC into SRAM `0x1002-0x100E`                                                        | Internal                             | `roadedge`, `xrally`, `bbust2`                                            |
| `0x7C`         | `0x9FC09BF8`               | Set the RTC from SRAM `0x1002-0x100E`. Sets HOLD, gives up with -1 if BUSY                    | No                                   | `roadedge`, `xrally`, `bbust2`                                            |
| `0x80`         | `0x9FC09C50`               | Log an error                                                                                  | Yes                                  | All ?                                                                     |
| `0x84`         | `0x9FC09CC4`               | 16-bit byte sum                                                                               | No (the Boot stage has its own copy) | ?                                                                         |
| `0x88`         | `0`                        | Empty slot                                                                                    |                                      |                                                                           |
| `0x8C`         | `0x9FC097D4`               | Battery SRAM test                                                                             | No                                   | ?                                                                         |
| `0x90`         | `0x9FC099D4`               | Run a test list through the RAM copy                                                          | No                                   | ?                                                                         |
| `0x94`         | `0x9FC08100`               | Write one TLB entry                                                                           | Internal                             | No                                                                        |


## Services

Cleaned up from GhidraDec and Radare2, so t hey only are annonated.

### Common

```c
#define REG8(addr) (*(volatile u8 *)(addr))
#define REG16(addr) (*(volatile u16 *)(addr))
#define REG32(addr) (*(volatile u32 *)(addr))

#define BIOS_RAM 0x80004000

#define SPRITE_RAM 0xC0000000
#define SPRITE_RAM_END 0xC000C000
#define SPRITE_REGS 0xC0010000
#define TILEMAP_RAM 0xC0100000
#define SCROLL_RECS 0xC0140000
#define TILEMAP_RAM_END 0xC0180000
#define TILEMAP_REGS 0xC0190000
#define PALETTE 0xC0200000
#define PALETTE_COUNT 0x1000
#define PALETTE_END (PALETTE + PALETTE_COUNT * 4)
#define MIXER 0xC0208000

#define BUFFER_CTL_3D 0xD0000000
#define BANK_A_3D 0xD0100000
#define BANK_B_3D 0xD0200000
#define BANK_SIZE_3D 0x60000

#define RTC 0xBF702104
#define SRAM 0xBF800000
#define SRAM_LOG_INDEX 0x1000
#define SRAM_ERRORS 0x1001
#define SRAM_RTC 0x1002
#define SRAM_RTC_LAST 0x100E

#define ATTR_WHITE 0x00000008
#define ATTR_RED 0x01000008
#define ATTR_FAILED 0x01000000

// State are kept in registers
u32 at; // Row base
u32 v0; // Bit 16 quiet, low half the cursor offset
u32 s7; // Added to each char
u32 t0; // Errors in the current test
```

**Please note that these functions are recreations of the original BIOS**
**routines and that only critical sections are done. Test menus are skipped,**
**take this as a HUGE grain of salt.**

### SVC 04

```c
// Every entry black
for (int i = 0; i < PALETTE_COUNT; i++) {
    REG32(PALETTE + i * 4) = 0;
}

// color 1 of palettes 0 and 1
REG32(PALETTE + 0x01 * 4) = 0x00FFFFFF;
REG32(PALETTE + 0x11 * 4) = 0x00FF0000;

REG16(TILEMAP_REGS + 0x00) = 0x0007;
REG16(TILEMAP_REGS + 0x08) = 0x0840;
REG16(TILEMAP_REGS + 0x10) = 0;

for (int i = 0; i < 4; i++) {
    REG32(SCROLL_RECS + i * 4) = 0;
}

REG32(MIXER + 0x0C) = 0x01404209;
REG16(MIXER + 0x00) = 0x0007;
REG32(MIXER + 0x04) = 0;
REG16(MIXER + 0x08) = 512;
REG16(MIXER + 0x0A) = 448;
REG16(MIXER + 0x50) = 0;
REG32(MIXER + 0x18) = 0;
REG32(MIXER + 0x1C) = 0;
REG32(MIXER + 0x24) = 0;

REG16(SPRITE_REGS + 0x00) = 0x0007;
```

NOTE: /!\ t7 is left at 0xC0204000 as the system stage relies on it

### SVC 4C

```c
#define TLB_FLAGS 0x17
#define TLB_PAGE_SIZE 0x01000000
#define TLB_MASK_16MB 0x01FFE000

static void tlb_write(u32 index, u32 virt, u32 phys)
{
    u32 even = (phys >> 6) | TLB_FLAGS;
    u32 odd = ((phys + TLB_PAGE_SIZE) >> 6) | TLB_FLAGS;

    cp0_index = index;
    cp0_pagemask = TLB_MASK_16MB;
    cp0_entrylo0 = even;
    cp0_entrylo1 = odd;
    cp0_entryhi = virt;
    tlbwi();
}

cp0_config = (cp0_config & ~0x3F) | 0x23;

tlb_write(0, 0xC0000000, 0x20000000); // Video
tlb_write(1, 0xD0000000, 0x30000000); // 3D buffers
tlb_write(4, 0xE0000000, 0x60000000); // Sound RAM
tlb_write(7, 0x60000000, 0xC0000000); // Network dual port

REG8(BUFFER_CTL_3D + 0) = 1;
REG8(BUFFER_CTL_3D + 2) = 0;
REG8(BUFFER_CTL_3D + 3) = 2;
```

### SVC 58

```c
void svc58(const char *text, u32 font, u32 offset, u32 palette)
{
    volatile u32 *cell = (volatile u32 *)(TILEMAP_RAM + offset);
    // 8x8 4bpp tiles are 32 bytes
    u32 tile = font >> 5;
    u32 attr = (palette << 24) + tile;
    u8 c = *text;

    do {
        *cell = c + attr;
        cell++;
        text++;
        c = *text;
    } while (c != 0);
}
```

### SVC 5C

```c
u32 svc5c(u32 unused, u32 test, u32 quiet)
{
    u32 entry = REG32(BIOS_RAM + test);
    void (*run)(void) = (void (*)(void))((entry & 0x7FFF) | BIOS_RAM);

    at = 0;
    v0 = quiet << 16;
    s7 = ATTR_WHITE;
    t0 = 0;
    REG8(SRAM + SRAM_ERRORS) = 0;

    run();

    return REG8(SRAM + SRAM_ERRORS);
}
```

### Tests done by 5C

```c
#define CODE_5555 1
#define CODE_AAAA 2
#define CODE_ADDR 3
#define CODE_MORE_ERRORS 4

#define ERRORS_LOGGED_MAX 8
#define ERRORS_CAPPED 0x10000

static void report_error(u32 addr, u16 code)
{
    s7 = ATTR_RED;

    if (t0 == ERRORS_CAPPED) {
        return;
    }

    if (t0 == ERRORS_LOGGED_MAX) {
        svc80(0, more_errors_addr(addr), more_errors_device(addr), CODE_MORE_ERRORS);
        t0 = ERRORS_CAPPED;
        return;
    }

    svc80(0, addr, device_of(addr), code);
    t0++;
}

static void test_fill(u32 start, u32 end, u32 step, u32 a, u32 b)
{
    for (u32 p = start; p != end; p += step) {
        REG32(p) = a;
        if (REG32(p) != a) {
            report_error(p, CODE_5555);
        }
    }

    for (u32 p = start; p != end; p += step) {
        REG32(p) = b;
        if (REG32(p) != b) {
            report_error(p, CODE_AAAA);
        }
    }
}

// svc 1C
static void test_addr(u32 start, u32 end)
{
    for (u32 p = start; p != end; p += 4) {
        REG32(p) = p;
    }

    for (u32 p = start; p != end; p += 4) {
        if (REG32(p) != p) {
            report_error(p, CODE_ADDR);
        }
    }
}

test_fill(TILEMAP_RAM, TILEMAP_RAM_END, 4, 0x55555555, 0xAAAAAAAA);
test_addr(TILEMAP_RAM, TILEMAP_RAM_END);

for (u32 p = TILEMAP_RAM; p != TILEMAP_RAM_END; p += 4) {
    REG32(p) = 0;
}

if (s7 & ATTR_FAILED) {
    v0 = 0;
    print(at + 0x10, "Scroll Ram NG");
    s7 = ATTR_WHITE;
}

test_fill(PALETTE, PALETTE_END, 4, 0x00555555, 0x00AAAAAA);

// svc 30
test_fill(SPRITE_RAM + 0x00, SPRITE_RAM_END + 0x00, 0x20, 0x01550155, 0x02AA02AA);
test_fill(SPRITE_RAM + 0x04, SPRITE_RAM_END + 0x04, 0x20, 0x55555555, 0xAAAAAAAA);
test_fill(SPRITE_RAM + 0x08, SPRITE_RAM_END + 0x08, 0x20, 0x05555555, 0x02AA2AAA);
test_fill(SPRITE_RAM + 0x0C, SPRITE_RAM_END + 0x0C, 0x20, 0x00550000, 0x00AA0000);
test_fill(SPRITE_RAM + 0x10, SPRITE_RAM_END + 0x10, 0x20, 0x00000055, 0x000000AA);

// svc 34
test_fill(BANK_A_3D, BANK_A_3D + BANK_SIZE_3D, 4, 0x55555555, 0xAAAAAAAA);
test_addr(BANK_A_3D, BANK_A_3D + BANK_SIZE_3D);

// svc 38
test_fill(BANK_B_3D, BANK_B_3D + BANK_SIZE_3D, 4, 0x55555555, 0xAAAAAAAA);
test_addr(BANK_B_3D, BANK_B_3D + BANK_SIZE_3D);
```

| Broken address            | Device                  | More Errors entry     |
|---------------------------|-------------------------|-----------------------|
| Sprite RAM                | `0x03` SpriteMemory     | `0xC0000000`, `0x03`  |
| Tilemap RAM               | `0x01` ScrollMemory     | `0xC0100000`, `0x01`  |
| Palette                   | `0x02` PaletteMemory    | `0xC0200000`, `0x02`  |
| Bank A, first 128 KB      | `0x05` FlameBuffer A0   | `0xD0000000`, `0x0C`  |
| Bank A, second 128 KB     | `0x06` FlameBuffer A1   | `0xD0000000`, `0x0C`  |
| Bank A, third 128 KB      | `0x04` Z-Buffer A       | `0xD0000000`, `0x0C`  |
| Bank B, first 128 KB      | `0x08` FlameBuffer B0   | `0xD0200000`, `0x0D`  |
| Bank B, second 128 KB     | `0x09` FlameBuffer B1   | `0xD0200000`, `0x0D`  |
| Bank B, third 128 KB      | `0x07` Z-Buffer B       | `0xD0200000`, `0x0D`  |

### SVC 78

```c
#define RTC_REG(n) (RTC + (n) * 8)
#define RTC_DIGITS 13

for (int i = 0; i < RTC_DIGITS; i++) {
    u8 first = 0;
    u8 second = 0;

    do {
        first = REG32(RTC_REG(i)) & 0xF;
        second = REG32(RTC_REG(i)) & 0xF;
    } while (first != second);

    REG8(SRAM + SRAM_RTC_LAST - i) = first;
}
```

NOTE:
SRAM 0x1002 -> weekday, then year, month, day, hour, minute and second, tens
before the units

### SVC 7C

```c
#define RTC_REG_D 0xD
#define RTC_HOLD 0x1
#define RTC_BUSY 0x2

s32 svc7c(void)
{
    u32 ctl_d = REG32(RTC_REG(RTC_REG_D));

    REG32(RTC_REG(RTC_REG_D)) = ctl_d | RTC_HOLD;
    if (REG32(RTC_REG(RTC_REG_D)) & RTC_BUSY) {
        REG32(RTC_REG(RTC_REG_D)) = ctl_d;
        return -1;
    }

    for (int i = 0; i < RTC_DIGITS; i++) {
        REG32(RTC_REG(i)) = REG8(SRAM + SRAM_RTC_LAST - i);
    }

    REG32(RTC_REG(RTC_REG_D)) = ctl_d;
    return 0;
}
```

### SVC 80

```c
#define LOG_ENTRY_SIZE 16
#define LOG_STAMP_DIGITS 5

void svc80(u32 unused, u32 addr, u8 device, u16 code)
{
    u8 index = 0;
    u32 entry = 0;

    svc78();

    index = REG8(SRAM + SRAM_LOG_INDEX);
    entry = SRAM + index * LOG_ENTRY_SIZE;

    REG8(entry + 5) = addr >> 24;
    REG8(entry + 6) = addr >> 16;
    REG8(entry + 7) = addr >> 8;
    REG8(entry + 8) = addr;
    REG8(entry + 9) = device;
    REG16(entry + 0xA) = code;

    // Weekday and seconds are dropped ?
    for (int i = 0; i < LOG_STAMP_DIGITS; i++) {
        u8 tens = REG8(SRAM + SRAM_RTC + 1 + i * 2);
        u8 units = REG8(SRAM + SRAM_RTC + 2 + i * 2);

        REG8(entry + i) = (tens << 4) | units;
    }

    REG8(SRAM + SRAM_LOG_INDEX) = index + 1;
    REG8(SRAM + SRAM_ERRORS) = REG8(SRAM + SRAM_ERRORS) + 1;
}
```
