#ifndef __I2S_WRP__
#define __I2S_WRP__

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "audio_clip_defs.h"
#include "i2s/i2s_peri.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Initialise the I2S wrapper
    \param *obj Pointer to I2S Object
    \param *cmpltcb Pointer to TX Complete callback
    \param *halfcmpltcb Pointer to TX Half-Complete callback
    \return 0 if success
*/
int32_t wrp_i2s_init(I2sObject *obj, void (*cmpltcb)(void), void (*halfcmpltcb)(void));

/*! \brief Get address and size of DMA buffer
    \param *size Pointer to uint32_t, returns size
    \return pointer to buffer
*/
int16_t *wrp_i2s_get_dma_buf(uint32_t *size);

/*! \brief Begin transmission of I2S
	\return 0 if success
*/
int32_t wrp_i2s_tx(void);

/*! \brief Stop transmission of I2S
	\return 0 if success
*/
int32_t wrp_i2s_stop(void);

#endif // __I2S_WRP__
