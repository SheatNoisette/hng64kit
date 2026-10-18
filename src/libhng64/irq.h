#ifndef HNG_IRQ_H
#define HNG_IRQ_H

// Levels are what 0x1F701104 reads, master pic 0x00-0x07 then the two slaves
// Should be good
enum hng_irq_level {
    HNG_IRQ_VBLANK = 0x00,
    HNG_IRQ_RASTER = 0x01,
    HNG_IRQ_3D = 0x03,
    HNG_IRQ_NETWORK = 0x0B,
    HNG_IRQ_IOMCU = 0x11,
};

typedef void (*hng_irq_handler)(void);

// Every level masked until it gets a handler, call it after hng_core_init
void hng_irq_init(void);

// Unmasks the level and NULL masks it again
void hng_irq_set_handler(enum hng_irq_level level, hng_irq_handler handler);

// Very explicit
void hng_irq_enable(void);
void hng_irq_disable(void);

#endif
