#include "ring_buffer.h"

uint8_t rb_write(Ring_Buffer *rb, uint8_t byte) {
    if (((rb->head + 1) & 0xFF )== rb->tail){
        rb->overrun++;
        return 1;
    }

    rb->buf[rb->head] = byte;

    rb->head = (rb->head + 1) & 0xFF;

    return 0;
}

uint8_t rb_read(Ring_Buffer *rb, uint8_t *byte) {
    if (rb->head == rb->tail){
        return 1;
    }

    *byte = rb->buf[rb->tail];

    rb->tail = (rb->tail + 1) & 0xFF;
    
    return 0;
}

uint32_t rb_available(Ring_Buffer *rb) {
    return (rb->head - rb->tail) & 0xFF;
}