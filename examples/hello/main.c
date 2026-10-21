#include <stdint.h>

#include <libhng64/bios.h>
#include <libhng64/core.h>
#include <libhng64/timer.h>

#define UPTIME_ROW 6
#define UPTIME_COL 10

#define UPTIME_DIGITS 10

static void format_seconds(uint32_t seconds, char *out)
{
    for (int32_t i = UPTIME_DIGITS - 1; i >= 0; i--) {
        out[i] = (char)('0' + seconds % 10);
        seconds /= 10;
    }

    out[UPTIME_DIGITS] = '\0';
}

int main(void)
{
    char digits[UPTIME_DIGITS + 1];
    uint32_t shown = UINT32_MAX;
    uint32_t seconds = 0;

    hng_core_init();
    hng_bios_text_init();

    hng_bios_print("HELLO FROM HNG64KIT !!!!!", 2, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("Lower case and symbols work too :) {[(<@#$%&>)]}", 3, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("This line is RED", 4, 2, HNG_BIOS_PALETTE_RED);
    hng_bios_print("Uptime:", UPTIME_ROW, 2, HNG_BIOS_PALETTE_WHITE);

    for (;;) {
        seconds = hng_timer_ms() / 1000;
        if (seconds == shown) {
            continue;
        }

        format_seconds(seconds, digits);
        hng_bios_print(digits, UPTIME_ROW, UPTIME_COL, HNG_BIOS_PALETTE_WHITE);
        shown = seconds;
    }
}
