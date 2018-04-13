/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          board_config.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   bsp header files and macro to enable/disable featrures
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-12      1.00                    Chen Hao
*
******************************************************************************/
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "stm32f10x_conf.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

/******************************************************************************
* Macros
******************************************************************************/
#define BOARD_FLASH_BASE FLASH_BASE
#define BOARD_FLASH_SECTOR_SIZE 2048   //bytes
#define BOARD_APP_OFFSET 0x4000
#define BOARD_NVIC_PRIO_GROUP NVIC_PriorityGroup_1

//peripheral
/*USART1*/
#define BOARD_USART1_ENABLE 1   //USART1 for debug
#define BOARD_IQR_PRIO_USART1 1
#define BOARD_IQR_SUB_PRIO_USART1 2
#define BOARD_USART1_TX GPIOA, GPIO_Pin_9
#define BOARD_USART1_RX GPIOA, GPIO_Pin_10
#define BOARD_USART1_RX_BUFFSIZE 256

/*USART2*/
#define BOARD_USART2_ENABLE 0
#define BOARD_IQR_PRIO_USART2 1
#define BOARD_IQR_SUB_PRIO_USART2 2
#define BOARD_USART2_TX GPIOA, GPIO_Pin_2
#define BOARD_USART2_RX GPIOA, GPIO_Pin_3
#define BOARD_USART2_RX_BUFFSIZE 32

/*USART3*/
#define BOARD_USART3_ENABLE 0
#define BOARD_IQR_PRIO_USART3 1
#define BOARD_IQR_SUB_PRIO_USART3 1
#define BOARD_USART3_TX GPIOB, GPIO_Pin_10
#define BOARD_USART3_RX GPIOB, GPIO_Pin_11
#define BOARD_USART3_RX_BUFFSIZE 2048

/*USART5*/
#define BOARD_USART5_ENABLE 0
#define BOARD_IQR_PRIO_UART5 1
#define BOARD_IQR_SUB_PRIO_UART5 1
#define BOARD_UART5_TX GPIOC, GPIO_Pin_12
#define BOARD_UART5_RX GPIOD, GPIO_Pin_2
#define BOARD_UART5_RX_BUFFSIZE 128

#endif /* __BOARD_CONFIG_H__ */

