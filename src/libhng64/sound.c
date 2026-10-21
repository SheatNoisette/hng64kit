#include <stdint.h>

#include "sound.h"
#include "internal/sound_hw.h"

void hng_sound_init(void)
{
    volatile uint16_t *src = (volatile uint16_t *)SOUND_IMAGE_PR;
    volatile uint16_t *dst = (volatile uint16_t *)SOUND_RAM;
    uint32_t words = SOUND_IMAGE_SIZE / sizeof(uint16_t);

    // Games does a 16bit copy, to be safe do it
    for (uint32_t i = 0; i < words; i++) {
        dst[i] = src[i];
    }

    *(volatile uint32_t *)SOUND_CPU_CTRL = SOUND_CPU_RUN;
}
