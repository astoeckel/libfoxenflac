#ifndef __CLIP_HANDLER__
#define __CLIP_HANDLER__

#include "audio_clip_defs.h"

typedef struct {
  uint8_t name[32];
  uint8_t is_stereo;
  uint32_t sampling_rate;
  uint32_t address;
  uint32_t size;
} clip_node;

int32_t clip_handler_chunk_read(uint8_t *chk, uint32_t size);
int32_t clip_handler_init_clip(uint8_t clip_idx);
uint32_t clip_handler_get_sample_rate(void);
bool clip_handler_is_stereo(void);
#endif // __CLIP_HANDLER__
