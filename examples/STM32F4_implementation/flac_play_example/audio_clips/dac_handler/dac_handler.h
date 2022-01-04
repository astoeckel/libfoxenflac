#ifndef __DAC_HANDLER__
#define __DAC_HANDLER__

#include "audio_clip_defs.h"
#include "ring16.h"

struct dac {
  bool is_streaming;
  bool is_stereo;
  uint32_t sample_rate;
  int16_t *buf;
  uint32_t size;
  bool volatile tx_halfcomplete;
  bool volatile tx_complete;
  void (*eostream)(void *context);
};

int32_t dac_handler_init(struct dac *o);
int32_t dac_handler_start_stream(struct dac *o);
int32_t dac_handler_stop_stream(struct dac *o);
int32_t dac_handler_dma_buffer_feed(struct dac *o);
void dac_handler_dma_txcmplt_buffer_feed_cb(void);
void dac_handler_dma_txhalfcmplt_buffer_feed_cb(void);
// ring stuff
int32_t dac_handler_ring_buffer_push_sample(int16_t s);
bool dac_handler_ring_is_above_push_threshold(void);
bool dac_handler_ring_is_above_threshold(void);
bool dac_handler_ring_is_empty(void);
#endif // __DAC_HANDLER__
