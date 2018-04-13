/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          os_middleware.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   FreeRTOS API middleware
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-11      1.00                    Chen Hao
*
******************************************************************************/
#ifndef __OS_MIDDLEWARE_H__
#define __OS_MIDDLEWARE_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "basetype.h"

/*FreeRTOS header file*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "timers.h"
#include "semphr.h"
#include "portable.h"
#include "FreeRTOSConfig.h"

/******************************************************************************
* Macros
******************************************************************************/
//util macro
#define MIN_VALUE(a, b) ((a) > (b) ? (b) : (a))
#define MAX_VALUE(a, b) ((a) < (b) ? (b) : (a))
#define BIT_SET(a, b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1<<(b)))
#define BIT_CHECK(a, b) ((a) & (1<<(b)))
#define CEIL_VALUE(a, b) (((a) + (b) - 1) / (b))

#define DELAY_1_MS 1
#define DELAY_50_MS 50
#define DELAY_100_MS 100
#define DELAY_500_MS 500
#define DELAY_1_S 1000

/*FreeRTOS*/
#define OS_MSG_BLOCK_WAIT portMAX_DELAY
#define OS_MSG_RECV_FAILED pdFAIL
#define OS_MSG_QUEUE_TYPE xQueueHandle
#define OS_TMR_TYPE TimerHandle_t
#define OS_TASK_TYPE xTaskHandle
#define OS_MUTEX_BLOCK_WAIT portMAX_DELAY
#define OS_MUTEX_TYPE SemaphoreHandle_t

#define os_malloc(size) pvPortMalloc((size)) 
#define os_free(p) vPortFree((p))
#define os_scheduler_delay(ms) vTaskDelay((ms)/portTICK_RATE_MS)
#define os_sys_free_heap() xPortGetFreeHeapSize()

//task
#define os_task_scheduler() vTaskStartScheduler()
#define os_task_create(tMain, tParam, tName, tStack, tPriority, tHdlr) xTaskCreate((tMain), (tName), (tStack/4), (tParam), (tPriority), (tHdlr))
#define os_task_free_stack(taskHdlr) uxTaskGetStackHighWaterMark((taskHdlr))

//message
#define os_msg_queue_create(queSize, dataSize) xQueueCreate((queSize), (dataSize)) 
#define os_msg_queue_recv(que, pData, timeout) xQueueReceive((que), (pData), (timeout))
//#define os_msg_queue_send(que, pData, timeout) xQueueSend((que), (pData), (timeout))

//timer
#define os_tmr_create(tInterval, tCallback, tAutoReload) xTimerCreate("OSTMR", ((tInterval)/portTICK_RATE_MS), (tAutoReload), NULL, (tCallback))
#define os_tmr_delete(tmrHdlr) xTimerDelete((tmrHdlr), 0)
//#define os_tmr_start(tmrHdlr) xTimerStart((tmrHdlr), 0)
//#define os_tmr_stop(tmrHdlr) xTimerStop((tmrHdlr), 0)
#define os_tmr_restart(tmrHdlr) xTimerReset((tmrHdlr), 0)
#define os_tmr_change_period(tmrHdlr, tInterval) xTimerChangePeriod((tmrHdlr), ((tInterval)/portTICK_RATE_MS), 0)
#define os_tmr_is_active(tmrHdlr) xTimerIsTimerActive((tmrHdlr))
#define os_tmr_get_expire_time(tmrHdlr) xTimerGetExpiryTime((tmrHdlr))

//mutex
#define os_mutex_create() xSemaphoreCreateMutex()
#define os_mutex_delete(mutexHdlr) vSemaphoreDelete((mutexHdlr))
#define os_mutex_take(mutexHdlr, timeout) xSemaphoreTake((mutexHdlr), ((timeout)/portTICK_RATE_MS))
#define os_mutex_give(mutexHdlr) xSemaphoreGive((mutexHdlr))

/******************************************************************************
* Enums
******************************************************************************/

/******************************************************************************
* Types
******************************************************************************/
typedef struct
{
    uint32 msgID;
    uint8 *pMsgData;
    uint32 msgData;
}OS_MSG_TYPE;

/******************************************************************************
* Global Functions
******************************************************************************/
/*OS API functions begin*/
//message
extern void os_msg_queue_send(OS_MSG_QUEUE_TYPE que, const void * const pdata, uint32 timeWait);
//timer
extern void os_tmr_start(OS_TMR_TYPE tmrHdlr);
extern void os_tmr_stop(OS_TMR_TYPE tmrHdlr);
//mutex
extern bool os_mutex_lock_init(OS_MUTEX_TYPE *mutex);
extern void os_mutex_lock_deinit(OS_MUTEX_TYPE *mutex);
extern bool os_mutex_lock(OS_MUTEX_TYPE *mutex);
extern bool os_mutex_try_lock(OS_MUTEX_TYPE *mutex, uint32 timeout);
extern bool os_mutex_unlock(OS_MUTEX_TYPE *mutex);
/*OS API functions end*/

#endif /* __OS_MIDDLEWARE_H__ */

