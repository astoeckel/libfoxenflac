/*! \file "i2s_peri.h"
   \brief I2S Abstraction layer.
   Provides functions to interface with HAL drivers
*/

#ifndef I2S_H_
#define I2S_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/* Driver Header files */
#include "stm32f4xx_hal.h"

#include "rtos.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* CONSTANTS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\def I2S_MAX_PERIPH
	Maximum number of I2S peripherals
*/
#define I2S_MAX_PERIPH		1

/*!\var uint32_t I2S_TRANSFER_TIMEOUT
	Number of ticks to wait before a transfer timeouts
*/
static const uint32_t I2S_TRANSFER_TIMEOUT = 5000;

/*!\def I2S_BUFFSZ
	Size of output buffer in 16bit words
*/
#define I2S_BUFFSZ	1024

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLE DEFINITIONS ------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\typedef I2sObject
	I2S Object. Contains necessary peripheral information

 \var I2S_HandleTypeDef I2sObject::*handle
 Pointer to HAL I2S handle
  \var uint8_t I2sObject::periph_index
 I2S peripheral index
  \var uint16_t I2sObject::buffer
 Buffer for I2S output
  \var bool I2sObject::playing
 Flag indicates if currently outputting on I2S
*/
typedef struct
{
    //I2S objects
	I2S_HandleTypeDef *handle;
	uint8_t periph_index;

	//Audio clip playback
	bool audclp_playing;
	uint16_t *buffer;
	uint32_t buffer_size;

	//A2DP playback
	bool a2dp_playing;
	void (*played_handler)(uint8_t buffer_index);

} I2sObject;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Initialize I2sObject

    \param *i2s Pointer to I2sObject object
    \param *handle Pointer to HAL I2S handle
    \param periph_index Index of I2S peripheral
*/
void i2sInit(I2sObject *i2s, I2S_HandleTypeDef *handl, uint8_t periph_index);

/*! \brief Start low level hardware for I2S

    \param *i2s Pointer to I2sObject object
*/
void i2s_MspInit(I2sObject *i2s);

/*! \brief Reset low level hardware for I2S

    \param *i2s Pointer to I2sObject object
*/
void i2s_MspDeInit(I2sObject *i2s);

/*! \brief Initialise i2s peripheral for audio clip functionality

    \param *i2s Pointer to I2sObject object
*/
void i2s_InitAudClip(I2sObject *i2s);

/*! \brief Executed when audio clip playback ended

    \param *i2s Pointer to I2sObject object
*/
void i2s_EndAudClip(I2sObject *i2s);

/*! \brief Output stream initialise

    \param *i2s Pointer to I2sObject object
    \param uint32_t Sample rate
    \param void* Pointer to played_handler
*/
void i2s_InitStream(I2sObject *i2s, uint32_t sample_rate, void (*played_handler)(uint8_t buff_ind));

/*! \brief Play stream

    \param *i2s Pointer to I2sObject object
    \param uint16_t* Pointer to output buffer
    \param uint32_t Size
*/
void i2s_PlayStream(I2sObject *i2s, uint16_t *pBuffer, uint32_t size);

/*! \brief Stop stream

    \param *i2s Pointer to I2sObject object
*/
void i2s_StopStream(I2sObject *i2s);

/*! \brief Check if peripheral is busy

    \param *i2s Pointer to I2sObject
    \return True: Busy. False: Idle.
*/
bool i2sCheckBusy(I2sObject *i2s);

void HAL_I2S_Assign_TxCpltCallback(void (*cb)(I2S_HandleTypeDef *i2s));
void HAL_I2S_Assign_TxHalfCpltCallback(void (*cb)(I2S_HandleTypeDef *i2s));

/* THREADS --------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif


#endif /* I2C_H_ */
