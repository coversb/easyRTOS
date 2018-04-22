/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          os_task_define.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   define all task
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-18      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include "os_task_define.h"
#include "os_trace_log.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
/*
*task function, parameters, task name, stack size, priority, handler
*/
static OS_TASK_INFO_TYPE os_task_info[OS_TASK_ITEM_END] = 
{
    /*task main functino,   parameter, task name,     stack size,      priority,            task handler*/
    NULL, NULL, NULL, NULL, NULL, NULL
};

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : os_task_create_all
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : create all task defined in os_task_info
******************************************************************************/
void os_task_create_all(void)
{
    for (uint8 idx = OS_TASK_ITEM_BEGIN; idx < OS_TASK_ITEM_END; ++idx)
    {
        if (NULL == os_task_info[idx].function)
        {
            continue;
        }
        os_task_create(os_task_info[idx].function,
                                os_task_info[idx].param,
                                os_task_info[idx].name,
                                os_task_info[idx].stackSize,
                                os_task_info[idx].priority,
                                &os_task_info[idx].hdlr);
    }
}

/******************************************************************************
* Function    : os_task_print_free_stack
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_task_print_free_stack(void)
{
    for (uint8 idx = OS_TASK_ITEM_BEGIN; idx < OS_TASK_ITEM_END; ++idx)
    {
        uint32 freeStack;
        freeStack = os_task_free_stack(os_task_info[idx].hdlr);
        OS_INFO("%s FREE[%d]", os_task_info[idx].name, freeStack*4);
    }
}

/******************************************************************************
* Function    : os_task_print_free_heap
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void os_task_print_free_heap(void)
{
    OS_INFO("SYS HEAP FREE[%d]", os_sys_free_heap());
}

