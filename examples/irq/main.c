#include <stdint.h>

#include <libhng64/bios.h>
#include <libhng64/core.h>
#include <libhng64/iomcu.h>
#include <libhng64/irq.h>

#define PRINT_FRAMES 30

#define TITLE_ROW 2
#define FIRST_ROW 4
#define NAME_COL 2
#define VALUE_COL 14

#define COUNT_DIGITS 10

enum level_slot {
    SLOT_VBLANK,
    SLOT_3D,
    SLOT_NETWORK,
    SLOT_IOMCU,
    SLOT_COUNT,
};

static const char *const slot_names[SLOT_COUNT] = {"Vblank", "3D", "Network", "I/O MCU"};
static volatile uint32_t irq_counts[SLOT_COUNT];

static void on_vblank(void)
{
    irq_counts[SLOT_VBLANK]++;
}

static void on_3d(void)
{
    irq_counts[SLOT_3D]++;
}

static void on_network(void)
{
    irq_counts[SLOT_NETWORK]++;
}

static void on_iomcu(void)
{
    irq_counts[SLOT_IOMCU]++;
}

static void format_decimal(uint32_t value, char *out)
{
    for (int32_t i = COUNT_DIGITS - 1; i >= 0; i--) {
        out[i] = (char)('0' + value % 10);
        value /= 10;
    }

    out[COUNT_DIGITS] = '\0';
}

static void print_counts(const uint32_t *counts)
{
    char digits[COUNT_DIGITS + 1];
    uint32_t total = 0;

    for (uint32_t i = 0; i < SLOT_COUNT; i++) {
        format_decimal(counts[i], digits);
        hng_bios_print(digits, FIRST_ROW + i, VALUE_COL, HNG_BIOS_PALETTE_WHITE);
        total += counts[i];
    }

    format_decimal(total, digits);
    hng_bios_print(digits, FIRST_ROW + SLOT_COUNT + 1, VALUE_COL, HNG_BIOS_PALETTE_RED);
}

int main(void)
{
    struct hng_iomcu_inputs inputs = {0};
    uint32_t snapshot[SLOT_COUNT] = {0};
    uint32_t seen_vblank = 0;
    uint32_t next_print = 0;

    hng_core_init();
    hng_bios_text_init();

    hng_bios_print("IRQ COUNTERS", TITLE_ROW, NAME_COL, HNG_BIOS_PALETTE_WHITE);
    for (uint32_t i = 0; i < SLOT_COUNT; i++) {
        hng_bios_print(slot_names[i], FIRST_ROW + i, NAME_COL, HNG_BIOS_PALETTE_WHITE);
    }

    hng_bios_print("Total", FIRST_ROW + SLOT_COUNT + 1, NAME_COL, HNG_BIOS_PALETTE_RED);

    // Same open levels as roadedge so the masks end up 0x36 0xF7 0xFD
    // No idea what the other are
    hng_irq_init();
    hng_irq_set_handler(HNG_IRQ_VBLANK, on_vblank);
    hng_irq_set_handler(HNG_IRQ_3D, on_3d);
    hng_irq_set_handler(HNG_IRQ_NETWORK, on_network);
    hng_irq_set_handler(HNG_IRQ_IOMCU, on_iomcu);
    hng_irq_enable();

    for (;;) {
        if (irq_counts[SLOT_VBLANK] == seen_vblank) {
            continue;
        }

        seen_vblank = irq_counts[SLOT_VBLANK];

        // Kick IOMCU so coin and service presses gives 0x11
        hng_iomcu_poll(&inputs);

        if (seen_vblank < next_print) {
            continue;
        }

        next_print = seen_vblank + PRINT_FRAMES;

        hng_irq_disable();
        for (uint32_t i = 0; i < SLOT_COUNT; i++) {
            snapshot[i] = irq_counts[i];
        }

        print_counts(snapshot);
        hng_irq_enable();
    }
}
