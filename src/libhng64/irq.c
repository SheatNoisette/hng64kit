#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "internal/core_hw.h"
#include "internal/irq_hw.h"

#define IRQ_ACK_BITS 32

static const uint32_t pic_cmd[PIC_COUNT] = {PIC_MASTER_CMD, PIC_SLAVE1_CMD, PIC_SLAVE2_CMD};
static const uint32_t pic_data[PIC_COUNT] = {PIC_MASTER_DATA, PIC_SLAVE1_DATA, PIC_SLAVE2_DATA};
static const uint8_t pic_cascade[PIC_COUNT] = {0, PIC_SLAVE1_CASCADE, PIC_SLAVE2_CASCADE};

// Roadedge acks exactly these one by one
// @TODO: Why?
static const uint32_t irq_init_acks[] = {0x1, 0x2, 0x4, 0x8, 0x20, 0x200, 0x400, 0x20000, 0x800};
static hng_irq_handler irq_handlers[IRQ_LEVELS];
static uint8_t pic_masks[PIC_COUNT];

static void reg_write(uint32_t addr, uint32_t value)
{
    // Games write every byte register as a full word
    *(volatile uint32_t *)addr = value;
}

static void pic_set_mask(uint32_t pic, uint8_t mask)
{
    pic_masks[pic] = mask;
    reg_write(pic_data[pic], mask);
}

void hng_irq_init(void)
{
    uint32_t ack_count = sizeof(irq_init_acks) / sizeof(irq_init_acks[0]);

    for (uint32_t pic = 0; pic < PIC_COUNT; pic++) {
        pic_set_mask(pic, PIC_MASK_ALL);
    }

    reg_write(PIC_MASTER_CMD, PIC_ICW1);
    reg_write(PIC_MASTER_DATA, PIC_MASTER_ICW2);
    reg_write(PIC_MASTER_DATA, PIC_MASTER_ICW3);
    reg_write(PIC_MASTER_DATA, PIC_MASTER_ICW4);

    reg_write(PIC_SLAVE1_CMD, PIC_ICW1);
    reg_write(PIC_SLAVE1_DATA, PIC_SLAVE1_ICW2);
    reg_write(PIC_SLAVE1_DATA, PIC_SLAVE1_ICW3);
    reg_write(PIC_SLAVE1_DATA, PIC_SLAVE_ICW4);

    // Bios left slave 2 on base 0 where the mcu would look like vblank
    reg_write(PIC_SLAVE2_CMD, PIC_ICW1);
    reg_write(PIC_SLAVE2_DATA, PIC_SLAVE2_ICW2);
    reg_write(PIC_SLAVE2_DATA, PIC_SLAVE2_ICW3);
    reg_write(PIC_SLAVE2_DATA, PIC_SLAVE_ICW4);

    // Icw1 opens every line so shut them again
    for (uint32_t pic = 0; pic < PIC_COUNT; pic++) {
        pic_set_mask(pic, PIC_MASK_ALL);
    }

    for (uint32_t i = 0; i < ack_count; i++) {
        reg_write(IRQ_ACK_REG, irq_init_acks[i]);
    }
}

void hng_irq_set_handler(enum hng_irq_level level, hng_irq_handler handler)
{
    uint32_t index = (uint32_t)level;
    uint32_t pic = index / PIC_LINES;
    uint8_t line = 1 << (index % PIC_LINES);

    if (index >= IRQ_LEVELS) {
        return;
    }

    if (handler == NULL) {
        pic_set_mask(pic, pic_masks[pic] | line);
        irq_handlers[index] = NULL;
        return;
    }

    irq_handlers[index] = handler;
    pic_set_mask(pic, pic_masks[pic] & ~line);

    // Slave lines go nowhere until their cascade input on the master opens
    if (pic != PIC_MASTER) {
        pic_set_mask(PIC_MASTER, pic_masks[PIC_MASTER] & ~pic_cascade[pic]);
    }
}

void hng_irq_enable(void)
{
    uint32_t status = 0;

    CP0_READ(CP0_STATUS, status);
    status |= STATUS_IE | STATUS_IM2;
    CP0_WRITE(CP0_STATUS, status);
    CP0_HAZARD();
}

void hng_irq_disable(void)
{
    uint32_t status = 0;

    CP0_READ(CP0_STATUS, status);
    status &= ~STATUS_IE;
    CP0_WRITE(CP0_STATUS, status);

    // Ie still lets one through for a few instructions after the write
    CP0_HAZARD();
}

void hng_irq_dispatch(void)
{
    uint32_t cause = 0;
    uint32_t level = 0;
    hng_irq_handler handler = NULL;

    CP0_READ(CP0_CAUSE, cause);

    // Level reg keeps a stale value when nothing pending
    if ((cause & CAUSE_IP2) == 0) {
        return;
    }

    level = *(volatile uint32_t *)IRQ_LEVEL_REG & IRQ_LEVEL_MASK;

    // MAME has no mask so an unacked coin would keep int0 up forever
    if (level < IRQ_ACK_BITS) {
        reg_write(IRQ_ACK_REG, 1u << level);
    }

    if (level >= IRQ_LEVELS) {
        return;
    }

    // Slaves don't auto eoi and would block their lower lines
    if (level >= PIC_LINES) {
        reg_write(pic_cmd[level / PIC_LINES], PIC_EOI);
    }

    handler = irq_handlers[level];
    if (handler == NULL) {
        return;
    }

    handler();
}
