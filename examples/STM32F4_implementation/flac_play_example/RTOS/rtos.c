/*! \file "rtos.c"
   \brief RTOS abstraction layer
   Stream lines basic procedures such as task creation and clock setup.
*/

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* INCLUDES ------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "rtos.h"

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* FUNCTIONS -----------------------------------------------------------*/
/*----------------------------------------------------------------------*/

void rtos_createThread(TaskHandle_t *task, int priority,
                       size_t stacksize, void (*new_thread) (void *), const char * const pcName,
					   void *arg)
{
    xTaskCreate(*new_thread, pcName, stacksize,
        		arg, priority, task);
}

/*----------------------------------------------------------------------*/

void rtos_createMailbox(QueueHandle_t *mail, size_t msg_size, uint8_t mb_capacity)
{
    *mail = xQueueCreate(mb_capacity, msg_size);
}

/*----------------------------------------------------------------------*/

void rtos_createSemaphore(SemaphoreHandle_t *sem, uint8_t sem_count, uint8_t init_count)
{
    if (sem_count >= 1)
    	*sem = xSemaphoreCreateCounting(sem_count, init_count);
}

/*----------------------------------------------------------------------*/

void rtos_createClock(TimerHandle_t *clock, const char * const pcTimerName,
		int priority, uint32_t period_ticks,
        bool oneshot, void (*clock_fn) (TimerHandle_t), void *arg)
{
   *clock = xTimerCreate(pcTimerName, period_ticks, !oneshot, arg, *clock_fn);

   if (!oneshot)
	   xTimerStart(*clock, 0);
}

/*----------------------------------------------------------------------*/

uint32_t rtos_getTaskWaterMark(TaskHandle_t *task)
{
	return (uint32_t)uxTaskGetStackHighWaterMark(task);
}

/*----------------------------------------------------------------------*/

void rtos_cmd_fail(MBCmdItems *items)
{
    items->success = false;
    xSemaphoreGive(*items->sem_cmd);
}

/*----------------------------------------------------------------------*/

void rtos_cmd_success(MBCmdItems *items)
{
    items->success = true;
    xSemaphoreGive(*items->sem_cmd);
}

/*----------------------------------------------------------------------*/

void rtos_Sleep_mS(uint32_t time_ms)
{
	vTaskDelay(time_ms);
}

/*----------------------------------------------------------------------*/

void rtos_Sleep_S(uint32_t time_s)
{
	vTaskDelay(time_s*1000);
}

/*----------------------------------------------------------------------*/

uint32_t rtos_GetTicks(void)
{
	return xTaskGetTickCount();
}
