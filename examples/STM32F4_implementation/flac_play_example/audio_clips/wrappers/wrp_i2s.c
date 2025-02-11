/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>

#include "wrp_i2s.h"
#include "buffers/buffers.h"

#include "i2s/i2s_peri.h"
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLES -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

I2sObject *i2s;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

int32_t dac_handler_dma_buffer_feed_test(int16_t *buf, uint32_t size, bool txcmplt);
void (*user_tx_complete_cb)(void);
void (*user_tx_halfcomplete_cb)(void);

/*----------------------------------------------------------------------*/

static void tx_complete_cb(I2S_HandleTypeDef *hi2s) {
  if(user_tx_complete_cb != NULL)
    user_tx_complete_cb();
}

/*----------------------------------------------------------------------*/

static void tx_halfcomplete_cb(I2S_HandleTypeDef *hi2s) {
  if(user_tx_halfcomplete_cb != NULL)
    user_tx_halfcomplete_cb();
}

/*----------------------------------------------------------------------*/

int32_t wrp_i2s_init(I2sObject *obj, void (*cmpltcb)(void), void (*halfcmpltcb)(void)) {

  i2s = obj;
  i2s->buffer = (uint16_t *)i2s_dma_buf;
  i2s->buffer_size = DMA_BUF_SIZE;

  user_tx_complete_cb = cmpltcb;
  user_tx_halfcomplete_cb = halfcmpltcb;

  HAL_I2S_Assign_TxCpltCallback(tx_complete_cb);
  HAL_I2S_Assign_TxHalfCpltCallback(tx_halfcomplete_cb);

  return 0;
}

/*----------------------------------------------------------------------*/

int16_t *wrp_i2s_get_dma_buf(uint32_t *size) {
  if (i2s->buffer != NULL) {
	*size = i2s->buffer_size;
    return (int16_t *)i2s->buffer;
  } else {
	 *size = 0;
	 return NULL;
  }
}

/*----------------------------------------------------------------------*/

int32_t wrp_i2s_tx(void) {

  memset(i2s->buffer, 0, i2s->buffer_size);

  HAL_I2S_Transmit_DMA(i2s->handle, i2s->buffer, i2s->buffer_size);
  return 0;
}

/*----------------------------------------------------------------------*/

int32_t wrp_i2s_stop(void) {
  HAL_I2S_DMAStop(i2s->handle);
  return 0;
}

