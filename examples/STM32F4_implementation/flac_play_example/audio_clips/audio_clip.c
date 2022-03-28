/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "audio_clip.h"

#include "rtos.h"
#include "rtos_config.h"

#include "decoder.h"
#include "dac_handler.h"
#include "wrappers/wrp_i2s.h"
#include "wrappers/wrp_mem.h"
#include "wrappers/wrp_malloc.h"

#include "i2s.h"
#include "clips.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLES -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

I2sObject *i2sDrv;

enum acmail_type {
  ACMAIL_DECODER_STATE,
  ACMAIL_RESERVED,
};

struct acmail {
  uint8_t type;
  uint8_t pl[3];
  MBCmdItems items;
};

// rtos and mbox state
TaskHandle_t ac_task;
QueueHandle_t ac_mail;
SemaphoreHandle_t sem;

bool busy = false;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

// aditional user callback
static void (*p_callback)(void *context);

/*----------------------------------------------------------------------*/

/*! \brief Main AUDIO CLIP THREAD, initialized in audioClipInit()
 	 	 This handles playback of audio clips pointed to by audioClipPlay().

    \param *context NULL
*/
static void audioClipThread(void *context) {
  static enum decoder_task_state decoder_ts = DEC_TS_IDLE;
  struct acmail mbox_data;
  static uint32_t q_timeout = AUDP_IDLE_INTERVAL;
  struct acmail mbox_backup;

  while (1) {
    if (xQueueReceive(ac_mail, &mbox_data, q_timeout)) {

    	switch (mbox_data.type) {
		  case ACMAIL_DECODER_STATE:
			decoder_ts = mbox_data.pl[0];
			if(decoder_ts == DEC_TS_BUFFERING)
			{
				q_timeout = 0;
				mbox_backup = mbox_data;
			}
			if(decoder_ts == DEC_TS_IDLE)
			{
				q_timeout = AUDP_IDLE_INTERVAL;
				rtos_cmd_success(&mbox_backup.items);
			}

			break;

		  default:
			break;

		}
    }
    decoder_thread(decoder_ts);
  }
  return;
}

/*----------------------------------------------------------------------*/

/*! \brief Decoder callback called during playback

    \param *context Contains decoder event
*/
static void internal_decoder_callback(void *context) {

  enum decoder_event *evt = (enum decoder_event *)context;
  struct acmail acmail;

  switch (*evt) {

  // buffer reached threshold
  // start streaming
  case DEC_EVT_BUFFERING_REACH_THRSHLD:
    acmail.type = ACMAIL_DECODER_STATE;
    acmail.pl[0] = DEC_TS_STREAMING;
    xQueueSend(ac_mail, &acmail, 0);
    break;

  // end of clip mem reads
  case DEC_EVT_BUFFERING_END:
    // nothing to do..
    break;

  // end of clip streaming
  case DEC_EVT_STREAMING_END:
    acmail.type = ACMAIL_DECODER_STATE;
    acmail.pl[0] = DEC_TS_IDLE;
    xQueueSend(ac_mail, &acmail, 0);
    busy = false;

    // call user callback
    if (p_callback != NULL)
      p_callback(NULL);
    break;

  case DEC_EVT_ERROR:
    break;
  }

  return;
}

/*----------------------------------------------------------------------*/

int audioClipEOStreamAssignCB(void (*cb)(void *context)) {
  if (cb == NULL)
    p_callback = cb;
  return 0;
}

/*----------------------------------------------------------------------*/

int audioClipInit(I2sObject *i2s, void (*eos_cb)(void *context)) {

	i2sDrv = i2s;

  // pass the peripheral instances to the wrapper layer
  wrp_i2s_init(i2sDrv, dac_handler_dma_txcmplt_buffer_feed_cb, dac_handler_dma_txhalfcmplt_buffer_feed_cb); // rework I2SObject

  // assign decoder events callback
  decoder_events_assign_callback(internal_decoder_callback);
  // assign user callback
  audioClipEOStreamAssignCB(eos_cb);

  // create mbox
  rtos_createMailbox(&ac_mail, sizeof(struct acmail), AC_MBOX_SIZE);
  // create Semaphore
  rtos_createSemaphore(&sem, 1, 0);
  // create thread
  rtos_createThread(&ac_task, AC_RTOS_PRIORITY, AC_STACKSIZE,
                    audioClipThread, "Audio Clips Thread", NULL);

  return 0;
}

/*----------------------------------------------------------------------*/

int audioClipPlay(uint8_t clip) {
	bool bt_pause = false;
	uint8_t bt_vol0 = 0, bt_vol1 = 0;

	i2s_InitAudClip(i2sDrv);

	// trigger buffering and playback
	decoder_play_clip(clip);

	// mbox to self starts buffering
	struct acmail acmail;
	acmail.type = ACMAIL_DECODER_STATE;
	acmail.pl[0] = DEC_TS_BUFFERING;
	acmail.items.sem_cmd = &sem;
	acmail.items.success = false;
	xQueueSend(ac_mail, &acmail, 0);

	bool result = xSemaphoreTake(sem, CLIP_TIMEOUT);

	i2s_EndAudClip(i2sDrv);

	return 0;
}

/*----------------------------------------------------------------------*/

void audioClipListInit(clip_node *clips, uint16_t numclips)
{
	audio_clips = clips;
	num_clips = numclips;
}
