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
#include "hal_wdg.h"
#include "hal_rcc.h"
#include "hal_gpio.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/

/******************************************************************************
* Local Functions
******************************************************************************/
#if ( BOARD_IWDG_ENABLE == 1)
/******************************************************************************
* Function    : hal_iwdg_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : internal watchdog init
******************************************************************************/
static void hal_iwdg_init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(0xFFF); //0xfff*256/40k=26s
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/******************************************************************************
* Function    : hal_iwdg_feed
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : internal watchdog feed
******************************************************************************/
static void hal_iwdg_feed(void)
{
    IWDG_ReloadCounter();
}
#endif /*BOARD_IWDG_ENABLE*/

#if ( BOARD_EWDG_ENABLE == 1 )
/******************************************************************************
* Function    : hal_ewdg_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : external watchdog feed
******************************************************************************/
static void hal_ewdg_init(void)
{
    hal_rcc_enable(BOARD_EXT_WDG_IO_RCC);
    hal_gpio_set_mode(BOARD_EXT_WDG_PIN, GPIO_Mode_Out_PP);
    hal_gpio_set(BOARD_EXT_WDG_PIN, HAL_GPIO_LOW);
}

/******************************************************************************
* Function    : hal_ewdg_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : external watchdog feed
******************************************************************************/
static void hal_ewdg_feed(void)
{
    hal_gpio_set(BOARD_EXT_WDG_PIN, HAL_GPIO_LOW);
    for(int32 delay = 100; delay >= 0; --delay);
    hal_gpio_set(BOARD_EXT_WDG_PIN, HAL_GPIO_HIGH);
}
#endif /*BOARD_EWDG_ENABLE*/

/******************************************************************************
* Function    : hal_wdg_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void hal_wdg_init(void)
{
    #if ( BOARD_IWDG_ENABLE == 1)
    hal_iwdg_init();
    #endif /*BOARD_IWDG_ENABLE*/
    
    #if ( BOARD_EWDG_ENABLE == 1 )
    hal_ewdg_init();
    #endif /*BOARD_EWDG_ENABLE*/
}

/******************************************************************************
* Function    : hal_wdg_feed
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
void hal_wdg_feed(void)
{
    #if ( BOARD_IWDG_ENABLE == 1)
    hal_iwdg_feed();
    #endif /*BOARD_IWDG_ENABLE*/
    
    #if ( BOARD_EWDG_ENABLE == 1 )
    hal_ewdg_feed();
    #endif /*BOARD_EWDG_ENABLE*/
}

