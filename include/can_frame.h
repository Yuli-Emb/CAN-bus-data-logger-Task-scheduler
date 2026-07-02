#ifndef CAN_FRAME_H
#define CAN_FRAME_H
#include <stdint.h>

typedef enum {
    WAIT_SOF,
    READ_ID_H,
    READ_ID_L,
    READ_DLC,
    READ_DATA,
    READ_CRC
} FrameState;

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
    uint32_t timestamp;
} CAN_frame_t;

#endif