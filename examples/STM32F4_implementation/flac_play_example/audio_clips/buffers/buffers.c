#include "buffers.h"

uint8_t mem_buffer[RAW_BUF_SIZE];
int16_t ring_internal[RING_BUF_SIZE];
int32_t decoded_internal[DECODED_BUF_SIZE];
int16_t i2s_dma_buf[DMA_BUF_SIZE];
