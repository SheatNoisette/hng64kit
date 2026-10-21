#include <stdint.h>

#include "core.h"
#include "internal/core_hw.h"

static void tlb_clear(void)
{
    CP0_WRITE(CP0_WIRED, 0);
    CP0_WRITE(CP0_PAGEMASK, 0);
    CP0_WRITE(CP0_ENTRYLO0, 0);
    CP0_WRITE(CP0_ENTRYLO1, 0);

    // Fast boot leaves all 32 undefined and two matching entries shut the cpu down
    for (uint32_t i = 0; i < TLB_ENTRIES; i++) {
        uint32_t vpn = TLB_UNUSED_VPN + i * TLB_VPN_STEP;

        CP0_WRITE(CP0_INDEX, i);
        CP0_WRITE(CP0_ENTRYHI, vpn);
        CP0_HAZARD();

        __asm__ volatile ("tlbwi");
        CP0_HAZARD();
    }

    // Asid 0 so later mappings don't have to care
    CP0_WRITE(CP0_ENTRYHI, 0);
    CP0_HAZARD();
}

static void tlb_map(uint32_t index, uint32_t virt, uint32_t phys)
{
    uint32_t even = (phys >> 6) | TLB_ENTRYLO_FLAGS;
    uint32_t odd = ((phys + TLB_ODD_PAGE_OFFSET) >> 6) | TLB_ENTRYLO_FLAGS;

    CP0_WRITE(CP0_INDEX, index);
    CP0_WRITE(CP0_PAGEMASK, TLB_PAGEMASK_16MB);
    CP0_WRITE(CP0_ENTRYLO0, even);
    CP0_WRITE(CP0_ENTRYLO1, odd);
    CP0_WRITE(CP0_ENTRYHI, virt);
    CP0_HAZARD();

    __asm__ volatile ("tlbwi");
    CP0_HAZARD();
}

void hng_core_init(void)
{
    uint32_t config = 0;

    // Interrupts off to avoid being hung please
    CP0_WRITE(CP0_STATUS, STATUS_INIT);
    CP0_HAZARD();

    // Drops a software interrupt left pending
    CP0_WRITE(CP0_CAUSE, 0);

    CP0_WRITE(CP0_WATCHLO, 0);
    CP0_WRITE(CP0_WATCHHI, 0);

    // Writing compare also clears a pending timer interrupt
    CP0_WRITE(CP0_COUNT, 0);
    CP0_WRITE(CP0_COMPARE, UINT32_MAX);

    __asm__ volatile ("ctc1 %0, $31" : : "r"(FCSR_INIT));

    // Fast boot without the dma leaves kseg0 dirty
    CP0_READ(CP0_CONFIG, config);
    config = (config & ~CONFIG_K0_MASK) | CONFIG_K0_CACHED;
    CP0_WRITE(CP0_CONFIG, config);
    CP0_HAZARD();

    tlb_clear();

    // Everything past 0x20000000 is out of kseg1 reach and fast boot never maps it anyway
    tlb_map(TLB_VIDEO_INDEX, TLB_VIDEO_VIRT, TLB_VIDEO_PHYS);
    tlb_map(TLB_3D_INDEX, TLB_3D_VIRT, TLB_3D_PHYS);
    tlb_map(TLB_SOUND_INDEX, TLB_SOUND_VIRT, TLB_SOUND_PHYS);
    tlb_map(TLB_SOUND_CTRL_INDEX, TLB_SOUND_CTRL_VIRT, TLB_SOUND_CTRL_PHYS);
    tlb_map(TLB_NETWORK_INDEX, TLB_NETWORK_VIRT, TLB_NETWORK_PHYS);
}
