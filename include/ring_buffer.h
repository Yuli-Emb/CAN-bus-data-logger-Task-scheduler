#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdint.h>

typedef struct{
    uint8_t buf[256];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint8_t overrun;
} Ring_Buffer; 

uint8_t rb_write(Ring_Buffer *rb, uint8_t byte);
uint8_t rb_read(Ring_Buffer *rb, uint8_t *byte);
uint32_t rb_available(Ring_Buffer *rb);

#endif