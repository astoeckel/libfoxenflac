/*! \file "rtos.h"
   \brief RTOS abstraction layer
   Stream lines basic procedures such as task creation and clock setup.
*/

#ifndef RTOS_RTOS_H_
#define RTOS_RTOS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* VARIABLE DEFINITIONS ------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*!\typedef MBCmdItems
	Mailbox Command object. Include in other structs to create a Queue command object

 \var SemaphoreHandle_t MBCmdItems::*sem_cmd
 Semaphore for thread sequencing (e.g: Command complete)
 \var bool MBCmdItems::success
 Flags command success/failure
*/
typedef struct
{
    SemaphoreHandle_t* sem_cmd;
    bool success;
} MBCmdItems;

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*! \brief Create a new task

    \param *task Task handle
    \param priority Task priority
    \param stacksize Size of Stack
    \param *new_thread Task function
    \param pcName Name of thread
    \param *arg Array of parameters
*/
void rtos_createThread(TaskHandle_t *task, int priority,
                       size_t stacksize, void (*new_thread) (void *), const char * const pcName,
					   void *arg);

/*! \brief Create a new Mailbox/Queue

    \param *mail Mailbox/Queue handle
    \param msg_size Size of Message
    \param mb_capacity Number of concurrent messages
*/
void rtos_createMailbox(QueueHandle_t *mail, size_t msg_size, uint8_t mb_capacity);

/*! \brief Create a new Semaphore

    \param *sem Semaphore handle
    \param sem_count Semaphore count
    \param init_count Semaphore count to start with
*/
void rtos_createSemaphore(SemaphoreHandle_t *sem, uint8_t sem_count, uint8_t init_count);

/*! \brief Create a new Clock

    \param *clock Timer Handle
    \param pcTimerName Name of Timer
    \param priority Timer priority
    \param period_ticks Length of period in rtos ticks
    \param oneshot Is the Timer a oneshot or renewing?
    \param *clock_fn Timer function
    \param *arg Pointer to parameter array
*/
void rtos_createClock(TimerHandle_t *clock, const char * const pcTimerName,
		int priority, uint32_t period_ticks,
        bool oneshot, void (*clock_fn) (TimerHandle_t), void *arg);

/*! \brief Check Water mark on Task

    \param *task Task handle
*/
uint32_t rtos_getTaskWaterMark(TaskHandle_t *task);

/*! \brief Signal that command execution was successful

    \param *items Mailbox command
*/
void rtos_cmd_success(MBCmdItems *items);

/*! \brief Signal that command execution was unsuccessful

    \param *items Mailbox command
*/
void rtos_cmd_fail(MBCmdItems *items);

/*! \brief Sleep RTOS thread

    \param time_ms Amount of time to sleep
*/
void rtos_Sleep_mS(uint32_t time_ms);

/*! \brief Return number of ticks since start (DO NOT CALL FROM ISR)

    \return Number of ticks
*/
uint32_t rtos_GetTicks(void);
#ifdef __cplusplus
}
#endif

#endif /* RTOS_RTOS_H_ */
