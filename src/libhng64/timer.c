#include <stdint.h>

#include "timer.h"
#include "internal/core_hw.h"
#include "internal/timer_hw.h"

static uint32_t ms_total = 0;
static uint32_t ms_last_count = 0;
static uint32_t ms_leftover = 0;

uint32_t hng_timer_ms(void)
{
    uint32_t now = 0;

    CP0_READ(CP0_COUNT, now);

    ms_leftover += now - ms_last_count;
    ms_last_count = now;

    while (ms_leftover >= COUNT_PER_MS) {
        ms_leftover -= COUNT_PER_MS;
        ms_total++;
    }

    return ms_total;
}
