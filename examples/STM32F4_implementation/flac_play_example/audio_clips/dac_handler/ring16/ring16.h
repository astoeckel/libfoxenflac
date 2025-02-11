#ifndef __RING16_H__
#define __RING16_H__

#include "audio_clip_defs.h"

struct ring16 {
  uint16_t head;
  uint16_t tail;
  int16_t *buf;
  uint16_t size;
  uint16_t entries;
  bool is_empty : 1;
  bool is_full : 1;
};

int32_t ring16_push(struct ring16 *rb, int16_t v);
int32_t ring16_pop(struct ring16 *rb, volatile int16_t *v);
int32_t ring16_memrd(struct ring16 *rb, volatile int16_t *o, uint16_t s);
int32_t ring16_flush(struct ring16 *rb);
#endif // __RING16_H__
