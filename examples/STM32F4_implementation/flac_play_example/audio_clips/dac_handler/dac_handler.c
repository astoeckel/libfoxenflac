#include "dac_handler.h"
#include "wrappers/wrp_i2s.h"
#include "buffers/buffers.h"

struct dac *dac_p;

// ring buffer stuff

struct ring16 ring = {
    .head = 0,
    .tail = 0,
    .buf = ring_internal,
    .size = RING_BUF_SIZE,
    .entries = 0,
    .is_empty = true,
    .is_full = false,
};


int32_t dac_handler_init(struct dac *o) {
  dac_p = o; // workaround for dma_buffer_feed cb
  o->buf = wrp_i2s_get_dma_buf(&o->size);
  ring16_flush(&ring);
  return 0;
}

int32_t dac_handler_start_stream(struct dac *o) {
  // status
  o->is_streaming = true;
  // i2s stuff
  wrp_i2s_tx();
  return 0;
}

int32_t dac_handler_stop_stream(struct dac *o) {
  // status
  o->is_streaming = false;
  wrp_i2s_stop();
  ring16_flush(&ring);
  return 0;
}

int32_t dac_handler_dma_buffer_feed(struct dac *o) {
  uint32_t offset;
  if (o->tx_complete) {
    o->tx_complete = false;
    offset = o->size >> 1;
  } else {
    o->tx_halfcomplete = false;
    offset = 0;
  }

  // filling the dma buffer is quite different for stereo vs mono
  if (o->is_stereo) {
    // straight from the decoder
    ring16_memrd(&ring, &o->buf[offset], o->size >> 1);
  } else {
    // duplicate sample for each channel
    for (uint32_t s = 0; s < (o->size >> 1); s += 2) {
      if (ring16_pop(&ring, &o->buf[offset + s])) {
    	  return -1;
      }
      o->buf[offset + s + 1] = o->buf[offset + s];
    }
  }

  return 0;
}

void dac_handler_dma_txcmplt_buffer_feed_cb(void) {
  uint32_t offset = dac_p->size >> 1;
  dac_p->tx_complete = true;

  // filling the dma buffer is quite different for stereo vs mono
  if (dac_p->is_stereo) {
    // straight from the decoder
    ring16_memrd(&ring, &dac_p->buf[offset], dac_p->size >> 1);
  } else {
    // duplicate sample for each channel
    for (uint32_t s = 0; s < (dac_p->size >> 1); s += 2) {
      if (ring16_pop(&ring, &dac_p->buf[offset + s])) {
    	  return;
      }
      dac_p->buf[offset + s + 1] = dac_p->buf[offset + s];
    }
  }

  return;
}

void dac_handler_dma_txhalfcmplt_buffer_feed_cb(void) {
  uint32_t offset = 0;
  dac_p->tx_halfcomplete = true;

  // filling the dma buffer is quite different for stereo vs mono
  if (dac_p->is_stereo) {
    // straight from the decoder
    ring16_memrd(&ring, &dac_p->buf[offset], dac_p->size >> 1);
  } else {
    // duplicate sample for each channel
    for (uint32_t s = 0; s < (dac_p->size >> 1); s += 2) {
      if (ring16_pop(&ring, &dac_p->buf[offset + s])) {
    	  return;
      }
      dac_p->buf[offset + s + 1] = dac_p->buf[offset + s];
    }
  }

  return;
}

int32_t dac_handler_ring_buffer_push_sample(int16_t s) {
  return ring16_push(&ring, s);
}

bool dac_handler_ring_is_above_threshold(void) {
  return ring.entries > RING_THRSH;
}

bool dac_handler_ring_is_above_push_threshold(void) {
  return ring.entries > RING_PUSH_THRSH;
}

bool dac_handler_ring_is_empty(void) { return ring.is_empty; }
