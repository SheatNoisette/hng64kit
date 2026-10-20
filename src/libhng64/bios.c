#include <stdint.h>

#include "bios.h"
#include "internal/bios_hw.h"

static uint32_t bios_call(uint32_t slot, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t entry = *(volatile uint32_t *)(BIOS_RAM + slot);
    uint32_t target = (entry & BIOS_OFFSET_MASK) | BIOS_RAM;
    register uint32_t a0 __asm__("$4") = arg0;
    register uint32_t a1 __asm__("$5") = arg1;
    register uint32_t a2 __asm__("$6") = arg2;
    register uint32_t a3 __asm__("$7") = arg3;
    register uint32_t t9 __asm__("$25") = target;
    register uint32_t v0 __asm__("$2");

    // Services are hand written asm that trash s0-s7
    // Good job SNK!
    __asm__ volatile (
        ".set push\n\t"
        ".set noreorder\n\t"
        "jalr $25\n\t"
        "nop\n\t"
        ".set pop"
        : "=r"(v0), "+r"(a0), "+r"(a1), "+r"(a2), "+r"(a3), "+r"(t9)
        :
        : "$1", "$3", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$31",
          "memory");

    return v0;
}

static uint8_t two_digits(uint8_t tens, uint8_t units)
{
    return (uint8_t)(tens * 10 + units);
}

static void split_digits(uint8_t value, volatile uint8_t *tens, volatile uint8_t *units)
{
    *tens = value / 10;
    *units = value % 10;
}

void hng_bios_text_init(void)
{
    bios_call(BIOS_SLOT_TEXT_INIT, 0, 0, 0, 0);
}

void hng_bios_print(const char *text, uint32_t row, uint32_t col, enum hng_bios_palette palette)
{
    uint32_t offset = (row << TEXT_ROW_SHIFT) + (col << TEXT_COL_SHIFT);

    bios_call(BIOS_SLOT_PRINT, (uint32_t)text, TEXT_FONT_BASE, offset, palette);
}

uint32_t hng_bios_run_test(enum hng_bios_test test)
{
    return bios_call(BIOS_SLOT_RUN_TEST, 0, test, BIOS_TEST_QUIET, 0);
}

void hng_bios_log_error(uint32_t address, enum hng_bios_log_device device, enum hng_bios_log_code code)
{
    bios_call(BIOS_SLOT_LOG_ERROR, 0, address, device, code);
}

void hng_bios_rtc_read(struct hng_bios_time *time)
{
    volatile uint8_t *n = (volatile uint8_t *)RTC_NIBBLES;

    bios_call(BIOS_SLOT_RTC_READ, 0, 0, 0, 0);

    time->weekday = n[0] & 0x7;
    time->year = two_digits(n[1] & 0xF, n[2] & 0xF);
    time->month = two_digits(n[3] & 0x1, n[4] & 0xF);
    time->day = two_digits(n[5] & 0x3, n[6] & 0xF);
    time->hour = two_digits(n[7] & 0x3, n[8] & 0xF);
    time->minute = two_digits(n[9] & 0x7, n[10] & 0xF);
    time->second = two_digits(n[11] & 0x7, n[12] & 0xF);
}

enum hng_bios_rtc_result hng_bios_rtc_write(const struct hng_bios_time *time)
{
    volatile uint8_t *n = (volatile uint8_t *)RTC_NIBBLES;
    uint32_t result = 0;

    n[0] = time->weekday;
    split_digits(time->year, &n[1], &n[2]);
    split_digits(time->month, &n[3], &n[4]);
    split_digits(time->day, &n[5], &n[6]);
    split_digits(time->hour, &n[7], &n[8]);
    split_digits(time->minute, &n[9], &n[10]);
    split_digits(time->second, &n[11], &n[12]);

    result = bios_call(BIOS_SLOT_RTC_WRITE, 0, 0, 0, 0);
    if (result != 0) {
        return HNG_BIOS_RTC_BUSY;
    }

    return HNG_BIOS_RTC_OK;
}
