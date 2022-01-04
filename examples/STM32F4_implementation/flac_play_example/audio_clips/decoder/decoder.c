#include "decoder.h"
#include "clip_handler.h"
#include "dac_handler.h"
#include "foxen/flac.h"
#include "buffers/buffers.h"

#include "wrappers/wrp_malloc.h"

//TODO: remove
#include "rtos.h"

// the audio stream handles audio clip playback from loading
// audio clips using clip_hanler.h abstraction layer,
// to decoding of the audio clip using flac streaming feature, from
// foxen/flac.h. it makes use of a large ring buffer, using ring16.h, that
// allows the decoder some extra time for its computations.
// This piece of software interfaces an I2S peripheral through a custom wrapper
// - wrappers/i2s_wrp.h.

// flac decoder stuff

struct decoder {
  void *flac_mem;
  fx_flac_t *flac;
  uint8_t clip_idx;
  uint8_t *raw;
  int32_t *decoded;
  uint32_t cursor;
  bool eoc;
};

struct decoder decoder = {
	.flac_mem = NULL,
    .flac = NULL,
    .clip_idx = CLIP_IDX_INVALID,
    .raw = NULL,
    .decoded = decoded_internal,
    .cursor = 0,
    .eoc = false,
};

// i2s playback stuff
struct dac dac = {
    .is_streaming = false,
    .is_stereo = false,
    .sample_rate = 48000,
    .buf = NULL,
    .size = 1000,
    .tx_halfcomplete = false,
    .tx_complete = false,
};

// event callback pointer
void (*event_cb)(void *context);

// task state
enum decoder_task_state *task_state_ptr;
static enum decoder_task_state decoder_get_task_state(void) {
  return *task_state_ptr;
}

// streaming task
static int32_t streaming(struct dac *o, struct decoder *d) {

  enum decoder_event evt;

  // start stream
  if (!o->is_streaming) {
    dac_handler_start_stream(o);
  }

  // end stream
  if (dac_handler_ring_is_empty() && d->eoc &&
      (o->tx_complete || o->tx_halfcomplete)) {
    // stop dac
    dac_handler_stop_stream(o);
    // callback end of stream
    evt = DEC_EVT_STREAMING_END;
    // evts are handler by caller and state machine
    // will use mbox and thread to go on
    event_cb((void *)&evt);
    return 0;
  }

  // dac dma complete
  if (o->tx_complete || o->tx_halfcomplete) {
    // dac_handler_dma_buffer_feed(o); // @NOTE: doesn't work in main thread
	  o->tx_complete = false;
	  o->tx_halfcomplete = false;
  }

  return 0;
}

// buffering task
static int32_t buffering(struct decoder *d) {
  enum decoder_event evt;
  // clip reach its end, we are now waiting for playback to finish
  if (d->eoc)
    return 0;

  // read a chunk of the clip
  uint32_t to_read = RAW_BUF_SIZE - d->cursor;
  if (to_read > 0) {
    uint32_t nbytes = clip_handler_chunk_read((uint8_t *)(d->raw + d->cursor), to_read);
    if (!nbytes) {
      // end of clip
      free(d->flac_mem);
      d->eoc = true;
      evt = DEC_EVT_BUFFERING_END;
      event_cb((void *)&evt);
      return 0;
    }
    d->cursor += nbytes;
  }

  // flac decoder in action
  uint32_t raw_size = d->cursor;
  uint32_t decoded_size = DECODED_BUF_SIZE;
  switch (
      fx_flac_process(d->flac, d->raw, &raw_size, d->decoded, &decoded_size)) {
  case FLAC_END_OF_METADATA:
    // TODO: should handle this stuff
    // most likely the stored clips don't
    // contain metadata
    break;

  case FLAC_ERR:
    return -1;

  default:
    break;
  }

  // push each sample to the ring
  for (uint32_t sample = 0; sample < decoded_size; sample++) {
    if (dac_handler_ring_buffer_push_sample(
            (int16_t)(d->decoded[sample] >> 16))) {
      evt = DEC_EVT_ERROR;
      event_cb((void *)&evt);
    }
  }

  // relocate raw buffer
  memcpy(d->raw, &d->raw[raw_size], (d->cursor - raw_size));
  // TODO: optimize for loops (specifically foe ring buffer) if performance is
  // poor
//  for (uint32_t rlct = 0; rlct < (d->cursor - raw_size); rlct++) {
//    d->raw[rlct] = d->raw[rlct + raw_size];
//  }
  // update cursor for next clip chunk read
  d->cursor -= raw_size;

  if (dac_handler_ring_is_above_threshold() &&
		  decoder_get_task_state() == DEC_TS_BUFFERING) {

      // callback buffering reached threshold
      evt = DEC_EVT_BUFFERING_REACH_THRSHLD;
      // evts are handler by caller and state machine
      // will use mbox and thread to go on
      event_cb((void *)&evt);
  }

  return 0;
}

// idle task
static int32_t idle(void) { return 0; }

// one thread to rule them all
int32_t decoder_thread(enum decoder_task_state task_state) {

  task_state_ptr = &task_state;
  switch (task_state) {

  // buffering nests with streaming
  case DEC_TS_STREAMING:
    streaming(&dac, &decoder);
    /* FALL THROUGH */
  case DEC_TS_BUFFERING:
    buffering(&decoder);
    break;

  // doin nothin
  case DEC_TS_IDLE:
    idle();
  default:
    break;
  }

  return 0;
}

// API
int32_t decoder_play_clip(uint8_t clip_idx) {
  // this is mandatory
  if (event_cb == NULL)
    return -1;

  // halt previous stream
  if (decoder_get_task_state() == DEC_TS_STREAMING) {
    // stop previous stream
    dac_handler_stop_stream(&dac);
    free(decoder.flac);
  }

  // handle clip
  if (clip_handler_init_clip(clip_idx))
    return -2;

  // set decoder handle stuff
  decoder.raw = mem_buffer;
  memset(decoder.raw, 0, RAW_BUF_SIZE);
  memset(decoder.decoded, 0, DECODED_BUF_SIZE);
  decoder.cursor = 0;
  decoder.eoc = false;

  // alloc flac instance
  uint32_t flacsize = fx_flac_size(FLAC_SUBSET_MAX_BLOCK_SIZE_48KHZ, 1U);
  if (flacsize == 0)
	  return -3;

  decoder.flac_mem = malloc(flacsize);
  decoder.flac = fx_flac_init(decoder.flac_mem, FLAC_SUBSET_MAX_BLOCK_SIZE_48KHZ, 1U);

  // handle dac
  dac.sample_rate = clip_handler_get_sample_rate();
  dac.is_stereo = clip_handler_is_stereo();
  dac_handler_init(&dac);

  return 0;
}

int32_t decoder_events_assign_callback(void (*cb)(void *context)) {
  if (cb != NULL)
    event_cb = cb;
  return 0;
}
