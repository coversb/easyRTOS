/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          os_middleware.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   FreeRTOS API middleware
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-11      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "basetype.h"
#include "os_config.h"
#include "os_middleware.h"
#include "os_task_define.h"
#include "os_trace_log.h"

/******************************************************************************
* Macros
******************************************************************************/
#define OS_TMR_ACT_RETRY 5

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
static uint32 os_task_init_mask = 0;

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : os_get_tick_count
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : return system boot up ms count
******************************************************************************/
uint32 os_get_tick_count(void)
{
    return xTaskGetTickCount();
}

/******************************************************************************
* Function    : os_msg_queue_send
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_msg_queue_send(OS_MSG_QUEUE_TYPE que, const void * const pdata, uint32 timeWait)
{
    if (true != xQueueSend(que, pdata, timeWait))
    {
        if (pdata != NULL)
        {
            OS_MSG_TYPE *msg = (OS_MSG_TYPE*)pdata;
            if (msg->pMsgData != NULL)
            {
                os_free(msg->pMsgData);
            }
        }
        OS_DBG_ERR(DBG_MOD_OS, "OS MSG send err");
    }
}

/******************************************************************************
* Function    : os_msg_data_vaild
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool os_msg_data_vaild(uint8 *p)
{
    if (p == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Bad message data");
        return false;
    }
    
    return true;
}

/******************************************************************************
* Function    : os_tmr_start
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_tmr_start(OS_TMR_TYPE tmrHdlr)
{
    uint32 tmrStartDelay = 50;
    uint8 tmrStartRetryCnt = 0;

    if (tmrHdlr == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Bad TMR");
        return;
    }

    if (false != os_tmr_is_active(tmrHdlr))
    {
        OS_DBG_ERR(DBG_MOD_OS, "Stop TMR first");
        os_tmr_stop(tmrHdlr);
    }

    while (true != xTimerStart(tmrHdlr, tmrStartDelay))
    {
        tmrStartDelay *= 2;
        tmrStartRetryCnt++;
        OS_DBG_ERR(DBG_MOD_OS, "OS TMR start err[%d], delay[%d]", tmrStartRetryCnt, tmrStartDelay);
        if (tmrStartRetryCnt > OS_TMR_ACT_RETRY)
        {
            OS_DBG_ERR(DBG_MOD_OS, "OS TMR err");
            break;
        }
    }
}

/******************************************************************************
* Function    : os_tmr_stop
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_tmr_stop(OS_TMR_TYPE tmrHdlr)
{
    uint32 tmrStartDelay = 50;
    uint8 tmrStartRetryCnt = 0;

    if (tmrHdlr == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Bad TMR");
        return;
    }

    if (false == os_tmr_is_active(tmrHdlr))
    {
        OS_DBG_ERR(DBG_MOD_OS, "TMR Already STOP");
        return;
    }

    while (true != xTimerStop(tmrHdlr, tmrStartDelay))
    {
        tmrStartDelay *= 2;
        tmrStartRetryCnt++;
        OS_DBG_ERR(DBG_MOD_OS, "OS TMR stop err[%d], delay[%d]", tmrStartRetryCnt, tmrStartDelay);
        if (tmrStartRetryCnt > OS_TMR_ACT_RETRY)
        {
            OS_DBG_ERR(DBG_MOD_OS, "OS TMR err");
            break;
        }
    }
}

/******************************************************************************
* Function    : os_mutex_lock_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool os_mutex_lock_init(OS_MUTEX_TYPE *mutex)
{
    //need uninit not null mutex first
    if (*mutex != NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Need delete mutex first");
        os_mutex_delete(*mutex);
        *mutex = NULL;
    }
    
    *mutex = os_mutex_create();
    if (*mutex == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Mutex init err");
        return false;
    }
    else
    {
        return true;
    }
}

/******************************************************************************
* Function    : os_mutex_lock_deinit
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_mutex_lock_deinit(OS_MUTEX_TYPE *mutex)
{
    if (*mutex != NULL)
    {
        os_mutex_delete(*mutex);
        *mutex = NULL;
    }
}

/******************************************************************************
* Function    : os_mutex_lock
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool os_mutex_lock(OS_MUTEX_TYPE *mutex)
{
    if (*mutex == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Invalid mutex");
        return false;
    }
    while (os_mutex_take(*mutex, OS_MUTEX_BLOCK_WAIT) != true)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Take mutex err");
    }
    return true;
}

/******************************************************************************
* Function    : os_mutex_try_lock
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool os_mutex_try_lock(OS_MUTEX_TYPE *mutex, uint32 timeout)
{
    if (*mutex == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Invalid mutex");
        return false;
    }
    if (os_mutex_take(*mutex, timeout) != true)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Take mutex err");
        return false;
    }
    return true;
}

/******************************************************************************
* Function    : os_mutex_unlock
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool os_mutex_unlock(OS_MUTEX_TYPE *mutex)
{
    if (*mutex == NULL)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Invalid mutex");
        return false;
    }
    return (bool)os_mutex_give(*mutex);
}

/******************************************************************************
* Function    : os_set_task_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : set task init flag
******************************************************************************/
void os_set_task_init(uint8 idx)
{
    BIT_SET(os_task_init_mask, idx);
}

/******************************************************************************
* Function    : os_check_task_init_sync
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : check task init flag
******************************************************************************/
static bool os_check_task_init(void)
{
    bool ret = true;
    for (uint8 idx = OS_TASK_ITEM_BEGIN; idx < OS_TASK_ITEM_END; ++idx)
    {
        if (!BIT_CHECK(os_task_init_mask, idx))
        {
            ret = false;
            break;
        }
    }

    return ret;
}

/******************************************************************************
* Function    : os_wait_task_init_sync
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_wait_task_init_sync(void)
{
    while (!os_check_task_init())
    {
        os_scheduler_delay(OS_TASK_SYNC_CHECK_INTERVAL);
    }
}

