#include "clip_handler.h"
#include "wrappers/wrp_mem.h"
#include "clips.h"


// structures
struct clip_handler {
  uint8_t state;
  clip_node *clip;
  uint32_t offset;
  int32_t (*mem_read)(uint8_t *buf, uint32_t offset, uint32_t size);
  int32_t (*get_size)(void);
  int32_t (*get_addr)(void);
  int32_t (*set_element)(uint8_t id);
};

// clip_h handler
struct clip_handler clip_h = {
    .clip = NULL,
    .offset = 0,
    .mem_read = &wrp_mem_read,
    .get_size = &wrp_mem_get_size,
    .get_addr = &wrp_mem_get_address,
    .set_element = &wrp_mem_set_element,
};

clip_node *audio_clips;
uint16_t num_clips;

// function definitions
int32_t clip_handler_chunk_read(uint8_t *chk, uint32_t size) {

  int32_t nbytes = 0;
  if (clip_h.offset == clip_h.clip->size)
    return nbytes;

  if ((clip_h.offset + size) > clip_h.clip->size) {
    // end of buffering
    nbytes = clip_h.clip->size - clip_h.offset;
  } else
    nbytes = size;

  clip_h.mem_read(chk, clip_h.offset, nbytes);
  clip_h.offset += nbytes;

  return nbytes;
}

// this function is essential since it does not only check the id
// but also fills the size and address of the clips
// initially this code was thaugt to hold these parameters in the stack
int32_t clip_handler_init_clip(uint8_t clip_idx) {
  if (clip_idx >= num_clips)
    return -1;

  clip_h.offset = 0;

  clip_h.clip = &audio_clips[clip_idx];

  // this is somewhat more than checking the id
  // wrpper stuff..
  if (clip_h.set_element(clip_idx))
    return -2;

  clip_h.clip->size = clip_h.get_size();
  clip_h.clip->address = clip_h.get_addr();

  return 0;
}

uint32_t clip_handler_get_sample_rate(void) {
  if (clip_h.clip == NULL)
    return -1;
  return clip_h.clip->sampling_rate;
}

bool clip_handler_is_stereo(void) { return clip_h.clip->is_stereo; }
