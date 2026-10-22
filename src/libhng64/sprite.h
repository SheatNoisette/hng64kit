#ifndef HNG_SPRITE_H
#define HNG_SPRITE_H

#include <stdint.h>

// Entries in sprite RAM
#define HNG_SPRITE_COUNT 1536

// Smaller depth draws in front
// 0x7FF hides
#define HNG_SPRITE_DEPTH_MAX 0x7FE

// One 16x16 4bpp tile in the SP ROM
struct hng_sprite_image {
    uint32_t tile;
    uint32_t palette;
    uint32_t width;
    uint32_t height;
    const uint32_t *colors;
    uint32_t color_count;
};

// One sprite RAM entry
struct hng_sprite {
    uint32_t position;
    uint32_t zoom;
    uint32_t depth_chain;
    uint32_t palette;
    uint32_t tile;
    uint32_t unused[3];
};

// Call it after hng_bios_text_init which resets the registers
void hng_sprite_init(void);

// Colors go to the image palette
// NOTE: hng_bios_text_init wipes them
void hng_sprite_load_palette(const struct hng_sprite_image *image);

// X and y are signed 10 bits
// 0,0 is the top left of the screen
void hng_sprite_set(struct hng_sprite *sprite, const struct hng_sprite_image *image,
                    int32_t x, int32_t y, uint32_t depth);

void hng_sprite_hide(struct hng_sprite *sprite);

// Copies to sprite RAM entries first and up, do in vblank
void hng_sprite_copy(const struct hng_sprite *sprites, uint32_t first, uint32_t count);

#endif
