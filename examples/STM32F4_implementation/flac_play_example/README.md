# flac_play_example:

This example illustrates an implementation of libfoxenflac on an STM32F411CE,
using FREERTOS and STM32Cube's HAL drivers. The example runs at 96MHz and is able to play a 48kHz FLAC file with minimal buffering.

The following is a short description on how to get this running on your own STM32F4.

But first, a rundown of the example project. This example is built on an STM32CubeMX project.
There are two RTOS threads used here:

- audioClipLoop(), which can be found in "Core/Src/loop.c".
	+ This illustrates the initialisation phase, and playback and can be thought of as a barebones statemachine loop substitute.
- audioClipThread(), which is found in "audio_clips/audio_clip.c".
	+ This is the thread where the magic happens, ie playback of audio clips.

The relevant code is found in two folders:

- RTOS: Here we have some general purpose functions that act as wrappers for FREE-RTOS' functionality. If you use another RTOS implementation, you will need to modify the files found here.
- audio_clips: All code related to FLAC playback is found here.


## Usage

Top-level usage of the implementation is described in "loop.c":

```C
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

```

In the above code, a list of clips is described by "clips" and passed to the audioClip library with "audioClipListInit()".
In the main thread, 3 initialization functions need to be called:
- i2sInit()
- audioClipInit()
- audioClipListInit()

Clips are played using audioClipPlay(). This is a blocking function that waits on the FLAC file to finish playing.

## FLAC file storage (internal and external)

In our example, a single FLAC file is stored in FLASH memory at 0x08060000. The file, on playback, should sound like a distinguished English gentleman saying the phrase "Battery 100 percent."
The example file is included in the example folder in both .flac and .bin forms. Before running the example, you'll need to use STM32 ST-LINK Utility (or the Mac/Linux equivalent) to load the .bin file into FLASH at the indicated address (0x08060000).

Now of course, there is a method to playback from external memory, since you would not be able to fit many FLAC files on MCU memory.
For this you will have to edit the file "/audio_clips/wrappers/wrp_mem.c". The two functions in question are shown below:

```C
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLES -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

MemElement mem_elem;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

int32_t wrp_mem_read(uint8_t *buf, uint32_t offset, uint32_t size)
{
	//Read code here:
	memcpy(buf, (void *)(mem_elem.offset + offset), size);

	return 0;
}

/**********************************************************************************/

int32_t wrp_mem_set_element(uint8_t idx)
{
	//Retrieve size and offset here
	mem_elem.size = audio_clips[idx].size;
	mem_elem.offset = audio_clips[idx].address;

	return 0;
}

```

wrp_mem_read(): Here is where you perform the read operation to "buf". Note that "offset" refers to the offset from the start of the file.

wrp_mem_set_element(): This functions stores the details of the file (found in our "clips" array). The size and address of the clip can also be retrieved dynamically if you utilize a filesystem to access external memory. If so, you will not need to populate the .size and .address fields in the clips array.

## MALLOC

Definitions for malloc and free are found in the file "/audio_clips/wrappers/wrp_malloc.h". These may be modified if you plan to use another implementation of RTOS.

## I2S

As noted above, we use the STM32 HAL Drivers for our I2S implementation. If you wish to use a different driver, you'll need to modify the following files with comparable functionality:

- "/audio_clips/wrappers/wrp_i2s.c"
- "/audio_clips/i2s/i2s_peri.h"
- "/audio_clips/i2s/i2s_peri.c"

## Sample rate

The sample rate of this example is fixed at 48kHz. It is, however, possible to change the sample rate using the following line in "/audio_clips/i2s/i2s_peri.c":

```C
void i2s_InitAudClip(I2sObject *i2s)
{
	i2s->audclp_playing = true;

	//Setup I2S Peripheral
	HAL_I2S_DeInit(i2s->handle);

//CHANGE
	i2s->handle->Init.AudioFreq = I2S_AUDIOFREQ_48K;
/////////////////////////////////////////////////////
	HAL_I2S_Init(i2s->handle);
}

```

## Buffers

As you might expect, we need a healthy amount of RAM for the various buffers required to read, decode and playback a 48kHz FLAC sample without any jitter. We use 4 buffers described below, for a total of approx 21kB. It SHOULD be possible to reduce the requirements by either using a faster CPU or playing a lower sample rate. You are free to tweak the various sizes of the buffers, though these are fine tuned for 48kHz playback at a clockrate of 96MHz.

Suggestion: One strategy that works well is "sharing" buffers, assuming clip playback is only required sporadically.

```C
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

```