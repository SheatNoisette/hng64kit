#ifndef HNG_IRQ_HW_H
#define HNG_IRQ_HW_H

#define IRQ_LEVEL_REG 0xBF701104
#define IRQ_ACK_REG 0xBF70111C
#define IRQ_LEVEL_MASK 0xFF

// Three 8 line pics so 24 levels
#define IRQ_LEVELS 24
#define PIC_LINES 8
#define PIC_COUNT 3

#define PIC_MASTER 0
#define PIC_SLAVE1 1
#define PIC_SLAVE2 2

#define PIC_MASTER_CMD 0xBF701064
#define PIC_MASTER_DATA 0xBF70106C
#define PIC_SLAVE1_CMD 0xBF701074
#define PIC_SLAVE1_DATA 0xBF70107C
#define PIC_SLAVE2_CMD 0xBF701084
#define PIC_SLAVE2_DATA 0xBF70108C

// Same icw values as roadedge, only the master runs auto eoi
#define PIC_ICW1 0x11
#define PIC_MASTER_ICW2 0x00
#define PIC_MASTER_ICW3 0xC0
#define PIC_MASTER_ICW4 0x13
#define PIC_SLAVE1_ICW2 0x08
#define PIC_SLAVE1_ICW3 0x06
#define PIC_SLAVE2_ICW2 0x10
#define PIC_SLAVE2_ICW3 0x07
#define PIC_SLAVE_ICW4 0x11

#define PIC_MASK_ALL 0xFF
#define PIC_EOI 0x20

// Master ir6 and ir7 carry the slaves
#define PIC_SLAVE1_CASCADE 0x40
#define PIC_SLAVE2_CASCADE 0x80

#define STATUS_IE 0x1
#define STATUS_IM2 0x400
#define CAUSE_IP2 0x400

// Called by exception entry
void hng_irq_dispatch(void);

#endif
