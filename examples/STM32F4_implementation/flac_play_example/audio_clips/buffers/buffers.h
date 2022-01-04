#ifndef BUFFERS_H_
#define BUFFERS_H_


#include "audio_clip_defs.h"

/*!\uint8_t mem_buffer[]
	Used for:
	- FLAC raw input buffer: RAW_BUF_SIZE
*/
extern uint8_t mem_buffer[RAW_BUF_SIZE];

/*!\uint8_t ring_internal[]
	Ring buffer.
	Used for:
	- FLAC ring buffer: RING_BUF_SIZE
*/
extern int16_t ring_internal[RING_BUF_SIZE];

/*!\uint8_t decoded_internal[]
	Decoded buffer.
	Used for:
	- FLAC decoded buffer: DECODED_BUF_SIZE
*/
extern int32_t decoded_internal[DECODED_BUF_SIZE];

/*!\uint8_t i2s_dma_buf[]
	I2S DMA buffer.
	Used for:
	- FLAC I2S output: DMA_BUF_SIZE
*/
extern int16_t i2s_dma_buf[DMA_BUF_SIZE];

#endif /* BUFFERS_H_ */
