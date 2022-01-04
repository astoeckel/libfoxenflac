/*! \file "audio_types.h"
   \brief Audio_types header file

	All data types for Audio Subsystem declared here
*/

#ifndef SUB_AUDIO_AUDIO_TYPES_H_
#define SUB_AUDIO_AUDIO_TYPES_H_

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLE DEFINITIONS ------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\enum CSStatus
	Status of the CS47L90
*/
typedef enum
{
    CS_Initializing = 0,
    CS_Sleeping,
    CS_Operating
} CSStatus;

/*!\typedef AudioInputConfig
 Configuration for an input channel

 \var int16_t AudioInputConfig::volume
 Volume of channel in dB(x10) to first decibel place
 \var bool AudioInputConfig::mute
 Mute channel control
*/
typedef struct
{
    int16_t volume;
    bool mute;
} AudioInputConfig;

/*!\typedef AudioOutputConfig
 Configuration for an output channel

 \var int16_t AudioOutputConfig::volume
 Volume of channel in dB(x10) to first decibel place
 \var bool AudioOutputConfig::mute
 Mute channel control
*/
typedef struct
{
    int16_t volume;
    bool mute;
} AudioOutputConfig;

/*!\typedef AudioDSPConfig
 Settings for DSP

 \var bool AudioDSPConfig::enable
 Enable or disable DSP
 \var uint32_t AudioDSPConfig::clockreg
 Clock settings for AUD_DSP_CFG2_CLK register
*/
typedef struct
{
    bool enable;
    uint32_t clockreg;
} AudioDSPConfig;

/*!\typedef AudioState
 Audio Object. Contains all necessary data for Audio subsystem.

 \var SemaphoreHandle_t AudioState::sem_cmd
 Semaphore used to manage commands sent to other threads/subsystems
 \var CSStatus AudioState::status
 Status of CS47L90
 \var PeriObject AudioState::*peri
 Pointer to peripheral object
 \var SpiTransaction AudioState::spi_trans
 SPI transaction settings. Initialize on startup
 \var uint32_t AudioState::spi_read
 Location for single register SPI reads from CS47L90
 \var GPIO_TypeDef AudioState::*cs_port
 Pointer to SPI-CS pin port
 \var uint16_t AudioState::cs_pin
 Index of SPI-CS pin
 \var PowerUIState AudioState::*pwr
 Pointer to Power object
 \var AudioDSPConfig AudioState::*dsp_config
 Pointer to DSP config object
 \var AudioInputConfig AudioState::*input_config
 Pointer to Input configuration
 \var AudioOutputConfig AudioState::*output_config
 Pointer to Output configuration
 \var int16_t AudioState::*input_adj
 Pointer to Input Adjustments
 \var int16_t AudioState::*output_adj
 Pointer to Output Adjustments
 \var bool AudioState::adj_updated
 Flag raised whenever input/output config/adj modified
*/
typedef struct
{
    //Command management
//    SemaphoreHandle_t sem_cmd;

    CSStatus status;                //Status

//    PeriObject *peri;
    SpiTransaction spi_trans;
    uint32_t spi_read;

//  GPIO_TypeDef *cs_port;
//	uint16_t cs_pin;

//	PowerUIState *pwr;              //Power

    //Audio parameters
    AudioDSPConfig *dsp_config;
    AudioInputConfig *input_config;
    AudioOutputConfig *output_config;
    int16_t *input_adj;
    int16_t *output_adj;
    bool adj_updated;

} AudioState;

#endif /* SUB_AUDIO_AUDIO_TYPES_H_ */
