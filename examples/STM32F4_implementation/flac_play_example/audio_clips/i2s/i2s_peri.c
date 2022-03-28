#include "i2s_peri.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* LOCAL VARIABLES ------------------------------------------------------------*/


/* GLOBAL VARIABLES ------------------------------------------------------------*/
//Semaphore: Used to indicate a transfer is occuring
I2sObject *i2s_instance[I2S_MAX_PERIPH];
void (*hal_i2s_tx_cmplt_callback)(I2S_HandleTypeDef *i2s);
void (*hal_i2s_tx_halfcmplt_callback)(I2S_HandleTypeDef *i2s);

/* STATIC FUNCTIONS -----------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* PUBLIC --------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
//Initialisation
void i2sInit(I2sObject *i2s, I2S_HandleTypeDef *handle, uint8_t periph_index)
{
	i2s->handle = handle;
	i2s->periph_index = periph_index;
  
  hal_i2s_tx_cmplt_callback = NULL;
  hal_i2s_tx_halfcmplt_callback = NULL;

	if (i2s->periph_index >= I2S_MAX_PERIPH)
		return;

	i2s_instance[periph_index] = i2s;

	i2s->audclp_playing = false;
	i2s->a2dp_playing = false;
}

/*----------------------------------------------------------------------*/

void i2s_MspInit(I2sObject *i2s)
{
	HAL_I2S_MspInit(i2s->handle);
}

/*----------------------------------------------------------------------*/

void i2s_MspDeInit(I2sObject *i2s)
{
	HAL_I2S_MspDeInit(i2s->handle);
}

/*----------------------------------------------------------------------*/

I2sObject *i2s_matchObject(I2S_HandleTypeDef *hi2s)
{
	I2sObject *i2s = NULL;

	for (uint8_t periph_index = 0; periph_index < I2S_MAX_PERIPH; periph_index++)
	{
		if (i2s_instance[periph_index]->handle == hi2s)
		{
			i2s = i2s_instance[periph_index];
			break;
		}
	}

	return i2s;
}

/*----------------------------------------------------------------------*/

bool i2sCheckBusy(I2sObject *i2s)
{
	return i2s->audclp_playing || i2s->a2dp_playing;
}

/*----------------------------------------------------------------------*/

void i2s_InitAudClip(I2sObject *i2s)
{
	i2s->audclp_playing = true;

	//Setup I2S Peripheral
	HAL_I2S_DeInit(i2s->handle);

	i2s->handle->Init.AudioFreq = I2S_AUDIOFREQ_48K;
	HAL_I2S_Init(i2s->handle);
}

/*----------------------------------------------------------------------*/

void i2s_EndAudClip(I2sObject *i2s)
{
	i2s->audclp_playing = false;

}

/*----------------------------------------------------------------------*/

void i2s_InitStream(I2sObject *i2s, uint32_t sample_rate, void (*played_handler)(uint8_t buff_ind))
{
	//Wait until our audio clip playback is completely
	while(i2s->audclp_playing);

	i2s->a2dp_playing = true;

	i2s->played_handler = played_handler;

	i2s->handle->Init.AudioFreq = sample_rate;

	HAL_I2S_Init(i2s->handle);
}

/*----------------------------------------------------------------------*/

void i2s_PlayStream(I2sObject *i2s, uint16_t *pBuffer, uint32_t size)
{
	HAL_I2S_Transmit_DMA(i2s->handle, pBuffer, size);
}

/*----------------------------------------------------------------------*/

void i2s_StopStream(I2sObject *i2s)
{
	i2s->a2dp_playing = false;
	HAL_I2S_DMAStop(i2s->handle);
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* CALLBACKS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

void HAL_I2S_Assign_TxCpltCallback(void (*cb)(I2S_HandleTypeDef *hi2s)) {
  hal_i2s_tx_cmplt_callback = cb;
}

void HAL_I2S_Assign_TxHalfCpltCallback(void (*cb)(I2S_HandleTypeDef *hi2s)) {
  hal_i2s_tx_halfcmplt_callback = cb;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	I2sObject *i2s = i2s_matchObject(hi2s);
	if (i2s == NULL)
			return;

	if (i2s->audclp_playing)
	{
		if (hal_i2s_tx_cmplt_callback != NULL)
			hal_i2s_tx_cmplt_callback(hi2s);
	}
	else if (i2s->a2dp_playing)
	{
		(*i2s->played_handler)(1);
	}
}

/*----------------------------------------------------------------------*/

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	I2sObject *i2s = i2s_matchObject(hi2s);

	if (i2s == NULL)
		return;

	if (i2s->audclp_playing)
	{
		if (hal_i2s_tx_halfcmplt_callback != NULL)
		    hal_i2s_tx_halfcmplt_callback(hi2s);
	}
	else if (i2s->a2dp_playing)
	{
		(*i2s->played_handler)(0);
	}
}



