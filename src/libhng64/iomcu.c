#include <stdint.h>

#include "iomcu.h"
#include "internal/iomcu_hw.h"

enum iomcu_state {
    IOMCU_UNCONFIGURED,
    IOMCU_CONFIGURED,
};

static enum iomcu_state state = IOMCU_UNCONFIGURED;

static uint8_t dp_read(uint32_t offset)
{
    return *(volatile uint8_t *)(IOMCU_DP + offset);
}

static void dp_write(uint32_t offset, uint8_t value)
{
    *(volatile uint8_t *)(IOMCU_DP + offset) = value;
}

static uint16_t dp_read_word(uint32_t offset)
{
    uint16_t high = dp_read(offset);
    uint16_t low = dp_read(offset + 1);

    return (uint16_t)((high << 8) | low);
}

static void mcu_send(uint8_t cmd)
{
    // Mcu never clears the command and reruns whatever sits there
    dp_write(DP_PARAM, 0);
    dp_write(DP_CMD, cmd);

    *(volatile uint16_t *)IOMCU_KICK = 1;
}

static void mcu_configure(void)
{
    // 8 + 8 fits the 0x30 bytes the mcu copies out, it checks nothing itself
    dp_write(DP_CMD_DATA + SIZE_DIGITAL, HNG_IOMCU_PORTS);
    dp_write(DP_CMD_DATA + SIZE_ANALOG, HNG_IOMCU_PORTS);
    dp_write(DP_CMD_DATA + SIZE_OUTPUT1, 0);
    dp_write(DP_CMD_DATA + SIZE_OUTPUT2, 0);

    mcu_send(IOMCU_CMD_SIZES);
}

static void mcu_boot(uint8_t status)
{
    if (status == IOMCU_BOOT_WAIT) {
        dp_write(DP_STATUS, 0);
        mcu_send(IOMCU_CMD_EXCHANGE);
        return;
    }

    if (status == IOMCU_WANTS_CONFIG) {
        mcu_configure();
        state = IOMCU_CONFIGURED;
    }
}

static uint32_t mcu_answered(uint8_t status, uint8_t detail)
{
    if (status == IOMCU_DONE || status == IOMCU_DONE_EVENT) {
        return 1;
    }

    // These errors still ran the exchange !
    if (status == IOMCU_ERROR && (detail == IOMCU_ERR_ALARM || detail == IOMCU_ERR_COMMAND)) {
        return 1;
    }

    return 0;
}

enum hng_iomcu_result hng_iomcu_poll(struct hng_iomcu_inputs *inputs)
{
    uint8_t status = dp_read(DP_STATUS);
    uint8_t detail = dp_read(DP_DETAIL);

    // Only 3 and 4 are safe to act on before the config, games seemsto do the
    // same
    if (state == IOMCU_UNCONFIGURED) {
        mcu_boot(status);
        return HNG_IOMCU_WAITING;
    }

    if (!mcu_answered(status, detail)) {
        return HNG_IOMCU_WAITING;
    }

    inputs->player1 = dp_read_word(DP_PLAYER1);
    inputs->player2 = dp_read_word(DP_PLAYER2);

    for (uint32_t i = 0; i < HNG_IOMCU_PORTS; i++) {
        inputs->digital[i] = dp_read(DP_DIGITAL + i);
        inputs->analog[i] = dp_read(DP_ANALOG + i);
    }

    inputs->event = HNG_IOMCU_EVENT_NONE;
    if (status == IOMCU_DONE_EVENT) {
        inputs->event = (enum hng_iomcu_event)detail;
    }

    mcu_send(IOMCU_CMD_EXCHANGE);
    return HNG_IOMCU_FRESH;
}
