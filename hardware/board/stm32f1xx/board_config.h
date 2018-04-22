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
#include "stm32f10x_iwdg.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_flash.h"

/******************************************************************************
* Macros
******************************************************************************/
#define BOARD_FLASH_BASE FLASH_BASE
#define BOARD_FLASH_SECTOR_SIZE 2048   //bytes
#define BOARD_APP_OFFSET 0x4000
#define BOARD_NVIC_PRIO_GROUP NVIC_PriorityGroup_1

//peripheral
/*internal wathdog*/
#define BOARD_IWDG_ENABLE 1 //internal watchdog
/*external watchdog*/
#define BOARD_EWDG_ENABLE 1 //external watchdog
#define BOARD_EXT_WDG_IO_RCC RCC_APB2Periph_GPIOB
#define BOARD_EXT_WDG_PIN GPIOB, GPIO_Pin_1

/*BKP*/
#define BOARD_BKP_ENABLE 1
#define BOARD_BKP_RCC RCC_APB1Periph_PWR|RCC_APB1Periph_BKP
#define BOARD_BKP_RTC_ADDR BKP_DR1
#define BOARD_BKP_DEV_CRC_ADDR BKP_DR2
#define BOARD_BKP_DEV_ADDR BKP_DR3

/*RTC*/
#define BOARD_RTC_ENABLE 1
#define BOARD_RTC_BKP_SETFLAG  0x5A5A
#define BOARD_RTC_DEFAULT  1525104000   //2018-05-01 00:00:00

/*USART1*/
#define BOARD_USART1_ENABLE 1   //USART1 for debug
#define BOARD_IQR_PRIO_USART1 1
#define BOARD_IQR_SUB_PRIO_USART1 2
#define BOARD_USART1_RCC RCC_APB2Periph_USART1
#define BOARD_USART1_IO_RCC RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO
#define BOARD_USART1_TX GPIOA, GPIO_Pin_9
#define BOARD_USART1_RX GPIOA, GPIO_Pin_10
#define BOARD_USART1_RX_BUFFSIZE 256

/*USART2*/
#define BOARD_USART2_ENABLE 1   //USART2
#define BOARD_IQR_PRIO_USART2 1
#define BOARD_IQR_SUB_PRIO_USART2 2
#define BOARD_USART2_RCC RCC_APB1Periph_USART2
#define BOARD_USART2_IO_RCC RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO
#define BOARD_USART2_TX GPIOA, GPIO_Pin_2
#define BOARD_USART2_RX GPIOA, GPIO_Pin_3
#define BOARD_USART2_RX_BUFFSIZE 32

/*USART3*/
#define BOARD_USART3_ENABLE 1   //USART3
#define BOARD_IQR_PRIO_USART3 1
#define BOARD_IQR_SUB_PRIO_USART3 1
#define BOARD_USART3_RCC RCC_APB1Periph_USART3
#define BOARD_USART3_IO_RCC RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO
#define BOARD_USART3_TX GPIOB, GPIO_Pin_10
#define BOARD_USART3_RX GPIOB, GPIO_Pin_11
#define BOARD_USART3_RX_BUFFSIZE 2048

#define BOARD_M26_IO_RCC RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC
#define BOARD_M26_PWR GPIOC, GPIO_Pin_6
#define BOARD_M26_PWRKEY GPIOA, GPIO_Pin_8

/*USART5*/
#define BOARD_USART5_ENABLE 1   //USART5
#define BOARD_IQR_PRIO_UART5 1
#define BOARD_IQR_SUB_PRIO_UART5 1
#define BOARD_USART5_RCC RCC_APB1Periph_UART5
#define BOARD_USART5_IO_RCC RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO
#define BOARD_UART5_TX GPIOC, GPIO_Pin_12
#define BOARD_UART5_RX GPIOD, GPIO_Pin_2
#define BOARD_UART5_RX_BUFFSIZE 128

#endif /* __BOARD_CONFIG_H__ */

