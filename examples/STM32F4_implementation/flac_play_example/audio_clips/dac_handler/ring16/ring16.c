#include "ring16.h"

static inline uint16_t r16_inc_wrap(uint16_t index, uint16_t buf_size) {
  return ++index & (buf_size - 1);
}

int32_t ring16_push(struct ring16 *rb, int16_t v) {
  if (rb->buf == NULL)
    return -1;
  if (rb->is_full)
    return -2;

  rb->buf[rb->head] = v;
  rb->head = r16_inc_wrap(rb->head, rb->size);
  rb->is_empty = false;
  rb->is_full = rb->head == rb->tail;
  rb->entries++;

  return 0;
}

int32_t ring16_pop(struct ring16 *rb, volatile int16_t *v) {
  if (rb->buf == NULL)
    return -1;
  if (rb->is_empty)
    return -2;

  *v = rb->buf[rb->tail];
  rb->tail = r16_inc_wrap(rb->tail, rb->size);
  rb->is_full = false;
  rb->is_empty = rb->tail == rb->head;
  rb->entries--;

  return 0;
}

int32_t ring16_memrd(struct ring16 *rb, volatile int16_t *o, uint16_t s) {
  for (uint16_t n = 0; n < s; n++) {
    int e = ring16_pop(rb, &o[n]);
    if (e) {
      return e;
    }
  }
  return 0;
}

int32_t ring16_flush(struct ring16 *rb) {
  rb->tail = 0;
  rb->head = 0;
  rb->is_empty = true;
  rb->is_full = false;
  rb->entries = 0;
  return 0;
}
