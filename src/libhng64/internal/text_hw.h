#ifndef HNG_TEXT_HW_H
#define HNG_TEXT_HW_H

// Tilemap 0 that the layer hng_bios_text_init sets up
#define TEXT_MAP 0xC0100000
#define TEXT_MAP_COLUMNS 128

// Handle exactly the same as the BIOS character c is tile c + 8
#define TEXT_FONT_TILE 8
#define TEXT_PALETTE_SHIFT 24

#endif
