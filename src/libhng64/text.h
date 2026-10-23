#ifndef HNG_TEXT_H
#define HNG_TEXT_H

#include <stdint.h>

// White or red after hng_bios_text_init
enum hng_text_palette {
    HNG_TEXT_PALETTE_WHITE = 0,
    HNG_TEXT_PALETTE_RED = 1,
};

// Writes tilemap 0 directly, IRQ safe, use this than the BIOS call
void hng_text_print(const char *text, uint32_t row, uint32_t col, enum hng_text_palette palette);

#endif
