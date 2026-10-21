#ifndef HNG_IOMCU_H
#define HNG_IOMCU_H

#include <stdint.h>

// Inputs IN0-IN7 and analog AN0-AN7
#define HNG_IOMCU_PORTS 8

// Player word bit 14 on every board, the other bits depend on the board wiring
// so can't test that
#define HNG_IOMCU_START 0x4000

enum hng_iomcu_event {
    HNG_IOMCU_EVENT_NONE = 0,
    HNG_IOMCU_EVENT_TEST = 1,
    HNG_IOMCU_EVENT_SERVICE = 2,
    HNG_IOMCU_EVENT_COIN = 3,
};

enum hng_iomcu_result {
    HNG_IOMCU_WAITING = 0,
    HNG_IOMCU_FRESH = 1,
};

// Everything is active high aka 1 = pressed
struct hng_iomcu_inputs {
    uint16_t player1;
    uint16_t player2;
    uint8_t digital[HNG_IOMCU_PORTS];
    uint8_t analog[HNG_IOMCU_PORTS];
    enum hng_iomcu_event event;
};

// Once per frame, each fresh answer is the one to the previous kick
// CAUTION: THE MCU WAITS FOR 7s BEFORE SENDING DATA
enum hng_iomcu_result hng_iomcu_poll(struct hng_iomcu_inputs *inputs);

#endif
