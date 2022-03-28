#ifndef __DECODER__
#define __DECODER__

#include "audio_clip_defs.h"

enum decoder_event {
  DEC_EVT_BUFFERING_REACH_THRSHLD,
  DEC_EVT_BUFFERING_END,
  DEC_EVT_STREAMING_END,
  DEC_EVT_ERROR,
};

// task state
enum decoder_task_state {
  DEC_TS_IDLE,
  DEC_TS_BUFFERING,
  DEC_TS_STREAMING,
};

int32_t decoder_thread(enum decoder_task_state task_state);
int32_t decoder_play_clip(uint8_t clip_idx);
int32_t decoder_events_assign_callback(void (*cb)(void *context));
#endif // __DECODER__
