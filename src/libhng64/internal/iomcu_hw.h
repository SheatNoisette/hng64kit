#ifndef HNG_IOMCU_HW_H
#define HNG_IOMCU_HW_H

// Dual port ram is shared with the mcu
#define IOMCU_DP 0xBF808000

// MAME only kicks on a store to the upper half so 16 bit store
// @TODO: Verify on hardware
#define IOMCU_KICK 0xBF7021C4

#define DP_CMD 0x000
#define DP_PARAM 0x001
#define DP_STATUS 0x002
#define DP_DETAIL 0x003
#define DP_PLAYER1 0x008
#define DP_PLAYER2 0x00A
#define DP_DIGITAL 0x010
#define DP_CMD_DATA 0x200

#define DP_ANALOG (DP_DIGITAL + HNG_IOMCU_PORTS)

// Block sizes order for command 0x06
#define SIZE_DIGITAL 0
#define SIZE_ANALOG 1
#define SIZE_OUTPUT1 2
#define SIZE_OUTPUT2 3

#define IOMCU_CMD_EXCHANGE 0x00
#define IOMCU_CMD_SIZES 0x06

#define IOMCU_DONE 0x00
#define IOMCU_DONE_EVENT 0x01
#define IOMCU_BOOT_WAIT 0x03
#define IOMCU_WANTS_CONFIG 0x04
#define IOMCU_ERROR 0x0F

#define IOMCU_ERR_ALARM 0x01
#define IOMCU_ERR_COMMAND 0x02

#endif
