/******************************************************************************
*
*     Open source
*
*******************************************************************************
*  file name:          app_main.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   app demo
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-13      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "hal_board.h"
#include "hal_wdg.h"
#include "hal_bkp.h"
#include "hal_rtc.h"

#include "os_middleware.h"
#include "os_task_define.h"
#include "os_trace_log.h"

/******************************************************************************
* Macros
******************************************************************************/
#define DEBUG_COM hwSerial1

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/

/******************************************************************************
* Local Functions
******************************************************************************/

/******************************************************************************
* Function    : hardware_init
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void hardware_init()
{
    hal_board_init();

    DEBUG_COM.begin(115200);
    uint16 fmVer = 0x1000;
    OS_INFO("Easy RTOS V%d.%02d.%02d", (fmVer >> 12), ((fmVer >> 4) & 0xFF), (fmVer & 0x000F));
    OS_INFO("@%s-%s", __DATE__, __TIME__);

    hal_wdg_init();
    hal_bkp_init();
    hal_rtc_init();
}

/******************************************************************************
* Function    : demo_task
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void demo_task(void *pvParameters)
{
    while (1)
    {
        hal_wdg_feed();

        OS_INFO("demo_task: %u", hal_rtc_get());
        
        os_scheduler_delay(DELAY_1_S);
    }
}

/******************************************************************************
* Function    : main
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
int main(void)
{
    hardware_init();

    OS_TASK_TYPE taskHdlr;
    os_task_create(demo_task, NULL, "DEMO", 2048, 0, &taskHdlr);
    os_task_create_all();

    os_task_scheduler();

    return 0;
}
