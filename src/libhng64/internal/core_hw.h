#ifndef HNG_CORE_HW_H
#define HNG_CORE_HW_H

#define CP0_INDEX 0
#define CP0_ENTRYLO0 2
#define CP0_ENTRYLO1 3
#define CP0_PAGEMASK 5
#define CP0_WIRED 6
#define CP0_COUNT 9
#define CP0_ENTRYHI 10
#define CP0_COMPARE 11
#define CP0_STATUS 12
#define CP0_CAUSE 13
#define CP0_CONFIG 16
#define CP0_WATCHLO 18
#define CP0_WATCHHI 19

// CU0 CU1 and FR like the bios hands over + IE + IM cleared
#define STATUS_INIT 0x34000000

// FS makes denormal results flush instead of trapping, denormal inputs still trap though
#define FCSR_INIT 0x01000000

#define CONFIG_K0_MASK 0x7
#define CONFIG_K0_CACHED 0x3

#define TLB_ENTRIES 32

// Kseg0 never goes through the tlb so these entries can't match hmm
#define TLB_UNUSED_VPN 0x80000000
#define TLB_VPN_STEP 0x2000

// Uncached, dirty, valid and global like the bios entries
#define TLB_ENTRYLO_FLAGS 0x17

#define TLB_PAGEMASK_16MB 0x01FFE000
#define TLB_ODD_PAGE_OFFSET 0x01000000

// Same entries as BIOS
#define TLB_VIDEO_INDEX 0
#define TLB_VIDEO_VIRT 0xC0000000
#define TLB_VIDEO_PHYS 0x20000000

#define TLB_3D_INDEX 1
#define TLB_3D_VIRT 0xD0000000
#define TLB_3D_PHYS 0x30000000

#define TLB_SOUND_INDEX 4
#define TLB_SOUND_VIRT 0xE0000000
#define TLB_SOUND_PHYS 0x60000000

#define TLB_NETWORK_INDEX 7
#define TLB_NETWORK_VIRT 0x60000000
#define TLB_NETWORK_PHYS 0xC0000000

#define CP0_STR(reg) #reg
#define CP0_READ(reg, out) __asm__ volatile ("mfc0 %0, $" CP0_STR(reg) : "=r"(out))
#define CP0_WRITE(reg, value) __asm__ volatile ("mtc0 %0, $" CP0_STR(reg) : : "r"(value))

// Cp0 has no interlocks so pad, 4 covers worst case per um hopefully (NEC manual table 19-2)
// BIOS seems to do the same anyway
#define CP0_HAZARD() __asm__ volatile ("nop\n\tnop\n\tnop\n\tnop")

#endif
