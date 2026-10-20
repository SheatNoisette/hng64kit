#ifndef HNG_BIOS_H
#define HNG_BIOS_H

#include <stdint.h>

// Services busy wait and assume nobody else touches the hardware so call them with irqs off
// Are at 0x80004000 which only a normal boot SHOULD leaves behind

// The five tests games run, the others wipe main RAM, SRAM or the mailboxes
enum hng_bios_test {
    HNG_BIOS_TEST_SCROLL = 0x28,
    HNG_BIOS_TEST_PALETTE = 0x2C,
    HNG_BIOS_TEST_SPRITE = 0x30,
    HNG_BIOS_TEST_3D_BANK_A = 0x34,
    HNG_BIOS_TEST_3D_BANK_B = 0x38,
};

enum hng_bios_log_device {
    HNG_BIOS_DEV_MAIN_CPU = 0x00,
    HNG_BIOS_DEV_SCROLL = 0x01,
    HNG_BIOS_DEV_PALETTE = 0x02,
    HNG_BIOS_DEV_SPRITE = 0x03,
    HNG_BIOS_DEV_Z_BUFFER_A = 0x04,
    HNG_BIOS_DEV_FRAME_BUFFER_A0 = 0x05,
    HNG_BIOS_DEV_FRAME_BUFFER_A1 = 0x06,
    HNG_BIOS_DEV_Z_BUFFER_B = 0x07,
    HNG_BIOS_DEV_FRAME_BUFFER_B0 = 0x08,
    HNG_BIOS_DEV_FRAME_BUFFER_B1 = 0x09,
    HNG_BIOS_DEV_SOUND = 0x0A,
    HNG_BIOS_DEV_MAIN_RAM = 0x0B,
    HNG_BIOS_DEV_RENDER_BANK_A = 0x0C,
    HNG_BIOS_DEV_RENDER_BANK_B = 0x0D,
    HNG_BIOS_DEV_NET_DPRAM = 0x0E,
    HNG_BIOS_DEV_IO_DPRAM = 0x0F,
    HNG_BIOS_DEV_SRAM = 0x10,
    HNG_BIOS_DEV_IO_MCU = 0x11,
    HNG_BIOS_DEV_NET_CPU_DPRAM = 0x12,
    HNG_BIOS_DEV_NET_CPU_SRAM = 0x13,
    HNG_BIOS_DEV_NET_CPU_RAM = 0x14,
    HNG_BIOS_DEV_NET_CPU = 0x15,
};

// Cpu exceptions: HNG_BIOS_CODE_EXCEPTION + Cause.ExcCode
enum hng_bios_log_code {
    HNG_BIOS_CODE_5555 = 0x01,
    HNG_BIOS_CODE_AAAA = 0x02,
    HNG_BIOS_CODE_ADDRESS = 0x03,
    HNG_BIOS_CODE_MORE_ERRORS = 0x04,
    HNG_BIOS_CODE_TIMEOUT = 0x05,
    HNG_BIOS_CODE_NET_BOOT_REWRITE = 0x06,
    HNG_BIOS_CODE_NET_PROGRAM_REWRITE = 0x07,
    HNG_BIOS_CODE_EXCEPTION = 0x10,
};

// White and red after hng_bios_text_init
enum hng_bios_palette {
    HNG_BIOS_PALETTE_WHITE = 0,
    HNG_BIOS_PALETTE_RED = 1,
};

enum hng_bios_rtc_result {
    HNG_BIOS_RTC_OK = 0,
    HNG_BIOS_RTC_BUSY = -1,
};

// Binary vals, year is 0-99 only
struct hng_bios_time {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;
};

// Wipes the palette and sets tilemap 0 up as 512x448 text
void hng_bios_text_init(void);

// Needs a font at tile 8 in the SC ROMs
void hng_bios_print(const char *text, uint32_t row, uint32_t col, enum hng_bios_palette palette);

// Error count please note that tested RAM is trashed afterwards
uint32_t hng_bios_run_test(enum hng_bios_test test);

void hng_bios_log_error(uint32_t address, enum hng_bios_log_device device, enum hng_bios_log_code code);

void hng_bios_rtc_read(struct hng_bios_time *time);

// Busy means the clock was mid update so try again
enum hng_bios_rtc_result hng_bios_rtc_write(const struct hng_bios_time *time);

#endif
