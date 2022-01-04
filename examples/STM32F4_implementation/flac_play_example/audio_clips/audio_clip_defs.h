#ifndef __AUDIO_CLIP_DEFS__
#define __AUDIO_CLIP_DEFS__

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


// TODO: tune these values for higher decoder throughput
// the performance of the ring is highly tied to the sampling rate
// these were tested for 48ksps
#define RAW_BUF_SIZE 512  // u8 Make sure this isn't more than MEM_BUFF_SIZE
#define DECODED_BUF_SIZE 1024 // u32
// TODO: this is a lot.. we should play with these numbers..
#define RING_BUF_SIZE 8192                   // 2^n, u16
#define RING_THRSH ((RING_BUF_SIZE * 96) / 100) // 96%
#define RING_PUSH_THRSH ((RING_BUF_SIZE * 4) / 10) // 40%
// TODO: tune these and avoid draining the ring buffer
#define DMA_BUF_SIZE 256

#define CLIP_IDX_INVALID 255

#endif // __AUDIO_CLIP_DEFS__
