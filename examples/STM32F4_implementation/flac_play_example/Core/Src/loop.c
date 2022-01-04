/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "loop.h"

#include "i2s.h"
#include "i2s/i2s_peri.h"

#include "rtos.h"

#include "clip_handler/clip_handler.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLES -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\var I2sObject i2sObj
	Used to store and manage I2S status
*/
I2sObject i2sObj;

#define AUDIO_CLIPS_NUMBER 1

/*!\var clip_node clips
	Contains details of all our clips.
	These can be initialized or populated at runtime.
	Depends on how and where you store your flac files.
*/
clip_node clips[AUDIO_CLIPS_NUMBER] = {
	{
		.name = "battery100",
		.is_stereo = false,
		.sampling_rate = 48000,
		.address = 0x08060000,
		.size = 84454,
	}
};

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Our primary loop in this example.
		This initializes our I2sObject, audioClip thread,
		and audioClip list.
		It then loops our example audio clip infinitely.
    \param *context NULL not used
*/
void audioClipLoop(void *context)
{
	//Initialize I2S Object
	i2sInit(&i2sObj, &hi2s3, 0);

	//Initialize audio clip thread
	audioClipInit(&i2sObj, NULL);
	audioClipListInit(clips, AUDIO_CLIPS_NUMBER);

	while(1)
	{
		//Play audio clip
		audioClipPlay(0);

		rtos_Sleep_mS(100);
	}
}
