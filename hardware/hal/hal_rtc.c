/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_wdg.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   hardware watchdog
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-13      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include "hal_rtc.h"
#include "hal_bkp.h"
#include "os_trace_log.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : hal_rtc_set
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void hal_rtc_set(uint32 timestamp)
{
    #if ( BOARD_RTC_ENABLE == 1 )
    PWR_BackupAccessCmd(ENABLE);

    RTC_WaitForLastTask();
    RTC_SetCounter(timestamp);
    RTC_WaitForLastTask();

    PWR_BackupAccessCmd(DISABLE);
    #endif /*BOARD_RTC_ENABLE*/
}

/******************************************************************************
* Function    : hal_rtc_get
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
uint32 hal_rtc_get(void)
{
    uint32 time = 0;

    #if ( BOARD_RTC_ENABLE == 1 )
    RTC_WaitForLastTask();
    time = RTC_GetCounter();
    RTC_WaitForLastTask();
    #endif /*BOARD_RTC_ENABLE*/

    return time;
}

/******************************************************************************
* Function    : hal_rtc_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void hal_rtc_init(void)
{
    #if ( BOARD_RTC_ENABLE == 1 )
    if (hal_bkp_read(BOARD_BKP_RTC_ADDR) != BOARD_RTC_BKP_SETFLAG)
    {
        OS_DBG_ERR(DBG_MOD_HAL, "RTC is not configured");

        bool clkInitState = true;
        uint32 lseWait = 0;

        //Enable LSE
        RCC_LSEConfig(RCC_LSE_ON);
        //Wait till LSE is ready
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
            for (uint8 i = 0; i < 255; ++ i) {}//some delay
            
            ++lseWait;
            if (lseWait >= 0xFFFF)
            {
                OS_DBG_ERR(DBG_MOD_HAL, "LSE ERR");
                clkInitState = false;
                break;
            }
        }
        if (clkInitState)
        {
            hal_bkp_write(BOARD_BKP_RTC_ADDR, BOARD_RTC_BKP_SETFLAG);
        }

        //Select LSE as RTC Clock Source
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        //Enable RTC Clock */
        RCC_RTCCLKCmd(ENABLE);
        RTC_WaitForSynchro();
        
        //Set RTC prescaler: set RTC period to 1sec
        RTC_WaitForLastTask();
        RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

        OS_DBG_ERR(DBG_MOD_HAL, "RTC set to default");
        hal_rtc_set(BOARD_RTC_DEFAULT);
    }
    else
    {
        RTC_WaitForSynchro();
    }    
    #endif /*BOARD_RTC_ENABLE*/
}

