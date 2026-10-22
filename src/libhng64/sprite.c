#include <stdint.h>

#include "sprite.h"
#include "internal/sprite_hw.h"

void hng_sprite_init(void)
{
    volatile uint32_t *regs = (volatile uint32_t *)SPRITE_REGS;
    struct hng_sprite hidden = {0};

    regs[SPRITE_REG_MODE] = SPRITE_MODE_4BPP;
    regs[SPRITE_REG_OFFSET] = 0;
    regs[SPRITE_REG_UNKNOWN2] = 0;
    regs[SPRITE_REG_UNKNOWN3] = 0;
    regs[SPRITE_REG_UNKNOWN4] = SPRITE_UNKNOWN4_VALUE;

    // POST leaves 0xAAAAAAAA everywhere sigh
    hng_sprite_hide(&hidden);
    for (uint32_t i = 0; i < HNG_SPRITE_COUNT; i++) {
        hng_sprite_copy(&hidden, i, 1);
    }
}

void hng_sprite_load_palette(const struct hng_sprite_image *image)
{
    volatile uint32_t *palette = (volatile uint32_t *)PALETTE_RAM;
    uint32_t base = image->palette * PALETTE_BLOCK_SIZE;

    for (uint32_t i = 0; i < image->color_count; i++) {
        palette[base + i] = image->colors[i];
    }
}

void hng_sprite_set(struct hng_sprite *sprite, const struct hng_sprite_image *image,
                    int32_t x, int32_t y, uint32_t depth)
{
    uint32_t pos_x = (uint32_t)x & SPRITE_POS_MASK;
    uint32_t pos_y = (uint32_t)y & SPRITE_POS_MASK;

    sprite->position = (pos_y << SPRITE_Y_SHIFT) | pos_x;
    sprite->zoom = SPRITE_ZOOM_1TO1;
    sprite->depth_chain = (depth & SPRITE_DEPTH_MASK) << SPRITE_DEPTH_SHIFT;
    sprite->palette = image->palette << SPRITE_PALETTE_SHIFT;
    sprite->tile = image->tile & SPRITE_TILE_MASK;
}

void hng_sprite_hide(struct hng_sprite *sprite)
{
    sprite->depth_chain = (uint32_t)SPRITE_DEPTH_HIDDEN << SPRITE_DEPTH_SHIFT;
}

void hng_sprite_copy(const struct hng_sprite *sprites, uint32_t first, uint32_t count)
{
    volatile uint32_t *ram = (volatile uint32_t *)SPRITE_RAM + first * SPRITE_ENTRY_WORDS;
    const uint32_t *src = (const uint32_t *)sprites;

    for (uint32_t i = 0; i < count * SPRITE_ENTRY_WORDS; i++) {
        ram[i] = src[i];
    }
}
