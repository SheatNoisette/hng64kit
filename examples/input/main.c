#include <stdint.h>

#include <libhng64/bios.h>
#include <libhng64/core.h>
#include <libhng64/iomcu.h>
#include <libhng64/timer.h>

#define POLL_MS 16

#define STATE_ROW 2
#define PLAYER_ROW 4
#define DIGITAL_ROW 6
#define ANALOG_ROW 7
#define EVENT_ROW 9
#define VALUE_COL 12

static const char hex_digits[] = "0123456789ABCDEF";

static void format_hex(uint32_t value, uint32_t digits, char *out)
{
    for (int32_t i = digits - 1; i >= 0; i--) {
        out[i] = hex_digits[value & 0xF];
        value >>= 4;
    }

    out[digits] = '\0';
}

static void print_bytes(const uint8_t *bytes, uint32_t row)
{
    char text[HNG_IOMCU_PORTS * 3 + 1];

    for (uint32_t i = 0; i < HNG_IOMCU_PORTS; i++) {
        format_hex(bytes[i], 2, &text[i * 3]);
        text[i * 3 + 2] = ' ';
    }

    text[HNG_IOMCU_PORTS * 3] = '\0';
    hng_bios_print(text, row, VALUE_COL, HNG_BIOS_PALETTE_WHITE);
}

static void print_inputs(const struct hng_iomcu_inputs *inputs)
{
    char word[5];
    enum hng_bios_palette start1 = HNG_BIOS_PALETTE_WHITE;
    enum hng_bios_palette start2 = HNG_BIOS_PALETTE_WHITE;

    if (inputs->player1 & HNG_IOMCU_START) {
        start1 = HNG_BIOS_PALETTE_RED;
    }

    if (inputs->player2 & HNG_IOMCU_START) {
        start2 = HNG_BIOS_PALETTE_RED;
    }

    format_hex(inputs->player1, 4, word);
    hng_bios_print(word, PLAYER_ROW, VALUE_COL, start1);

    format_hex(inputs->player2, 4, word);
    hng_bios_print(word, PLAYER_ROW, VALUE_COL + 6, start2);

    print_bytes(inputs->digital, DIGITAL_ROW);
    print_bytes(inputs->analog, ANALOG_ROW);
}

static void print_event(enum hng_iomcu_event event)
{
    if (event == HNG_IOMCU_EVENT_TEST) {
        hng_bios_print("TEST   ", EVENT_ROW, VALUE_COL, HNG_BIOS_PALETTE_WHITE);
        return;
    }

    if (event == HNG_IOMCU_EVENT_SERVICE) {
        hng_bios_print("SERVICE", EVENT_ROW, VALUE_COL, HNG_BIOS_PALETTE_WHITE);
        return;
    }

    if (event == HNG_IOMCU_EVENT_COIN) {
        hng_bios_print("COIN   ", EVENT_ROW, VALUE_COL, HNG_BIOS_PALETTE_WHITE);
    }
}

int main(void)
{
    struct hng_iomcu_inputs inputs = {0};
    enum hng_iomcu_result result = HNG_IOMCU_WAITING;
    uint32_t next_poll = 0;
    uint32_t now = 0;
    uint32_t ready = 0;

    hng_core_init();
    hng_bios_text_init();

    hng_bios_print("I/O MCU:", STATE_ROW, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("BOOTING", STATE_ROW, VALUE_COL, HNG_BIOS_PALETTE_RED);
    hng_bios_print("P1 P2:", PLAYER_ROW, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("Digital:", DIGITAL_ROW, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("Analog:", ANALOG_ROW, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("Event:", EVENT_ROW, 2, HNG_BIOS_PALETTE_WHITE);

    for (;;) {
        now = hng_timer_ms();
        if (now < next_poll) {
            continue;
        }

        next_poll = now + POLL_MS;

        result = hng_iomcu_poll(&inputs);
        if (result != HNG_IOMCU_FRESH) {
            continue;
        }

        if (!ready) {
            hng_bios_print("READY  ", STATE_ROW, VALUE_COL, HNG_BIOS_PALETTE_WHITE);
            ready = 1;
        }

        print_inputs(&inputs);
        print_event(inputs.event);
    }
}
