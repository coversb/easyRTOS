/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_board.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   board init support
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-12      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include "hal_board.h"

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
* Function    : hal_board_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : board init
******************************************************************************/
void hal_board_init(void)
{
    SystemInit();

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, BOARD_APP_OFFSET);
    NVIC_PriorityGroupConfig(BOARD_NVIC_PRIO_GROUP);
}

/******************************************************************************
* Function    : hal_board_nvic_set_irq
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : set irq channel
******************************************************************************/
void hal_board_nvic_set_irq(uint8 IRQChannel, uint8 PreemptionPriority, uint8 SubPriority, FunctionalState Cmd)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PreemptionPriority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SubPriority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = Cmd;
    NVIC_Init(&NVIC_InitStructure);		
}

