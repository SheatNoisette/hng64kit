#ifndef HNG_BIOS_HW_H
#define HNG_BIOS_HW_H

// System copies the Bios stage here before jumping to the game
#define BIOS_RAM 0x80004000
#define BIOS_OFFSET_MASK 0x7FFF

#define BIOS_SLOT_TEXT_INIT 0x04
#define BIOS_SLOT_PRINT 0x58
#define BIOS_SLOT_RUN_TEST 0x5C
#define BIOS_SLOT_RTC_READ 0x78
#define BIOS_SLOT_RTC_WRITE 0x7C
#define BIOS_SLOT_LOG_ERROR 0x80

// Tests print to address 0x10 unless quiet
#define BIOS_TEST_QUIET 1

// Tilemap 0 has 128 cells of 4 bytes per row
#define TEXT_ROW_SHIFT 9
#define TEXT_COL_SHIFT 2

// Byte offset 0x100 is tile 8, the char + 8 mapping BIOS text uses
#define TEXT_FONT_BASE 0x100

// Weekday then year tens down to second units in batt SRAM
#define RTC_NIBBLES 0xBF801002

#endif
