#ifndef __AUDIO_CLIPS__
#define __AUDIO_CLIPS__

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "i2s/i2s_peri.h"
#include "clip_handler/clip_handler.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* CONSTANTS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#define AC_MBOX_SIZE 16

/*!\def CLIP_TIMEOUT
 How long to wait before timing out an audio clip play
*/
#define CLIP_TIMEOUT	15000

/*!\def AUDP_IDLE_INTERVAL
 Audio clip thread interval (when idle)
*/
#define AUDP_IDLE_INTERVAL	250

/*!\def AUDP_DEFAULT_SR
 Default sample rate for audio clip module
*/
#define AUDP_DEFAULT_SR		48000

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Assign a CB to End of Stream event

    \param *cb Callback to assign
    \return 0 if success
*/
int audioClipEOStreamAssignCB(void (*cb)(void *context));

/*! \brief Initialize Clip playing subsystem/thread

    \param *i2s Pointer to I2sObject object
    \param *cb End of stream Callback
    \return 0 if success
*/
int audioClipInit(I2sObject *i2s, void (*cb)(void *context));

/*! \brief Play a clip from stored clip list. This function blocks with a
	Semaphore until the whole clip is played.

    \param clip Index of clip to play in clip list
    \return 0 if success
*/
int audioClipPlay(uint8_t clip);

/*! \brief Initialize "Clip List". Pass an array of "clip_node" and
 	 	 the number of clips

    \param *clips Array of clip_node
    \param numclips Number of clips described by array
*/
void audioClipListInit(clip_node *clips, uint16_t numclips);

#endif // __AUDIO_CLIPS__
