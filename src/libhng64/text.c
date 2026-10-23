#include <stdint.h>

#include "text.h"
#include "internal/text_hw.h"

void hng_text_print(const char *text, uint32_t row, uint32_t col, enum hng_text_palette palette)
{
    volatile uint32_t *cells = (volatile uint32_t *)TEXT_MAP + row * TEXT_MAP_COLUMNS;
    uint32_t color = (uint32_t)palette << TEXT_PALETTE_SHIFT;

    for (uint32_t i = 0; text[i] != '\0' && col + i < TEXT_MAP_COLUMNS; i++) {
        uint8_t c = (uint8_t)text[i];
        cells[col + i] = color | (c + TEXT_FONT_TILE);
    }
}
