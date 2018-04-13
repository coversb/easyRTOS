/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_gpio.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   gpio
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-12      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
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
/******************************************************************************
* Function    : hal_gpio_set_mode
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : set gpio pin mode
******************************************************************************/
void hal_gpio_set_mode(GPIO_TypeDef* io, uint16 pin, GPIOMode_TypeDef mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_Mode = mode;    
    GPIO_Init(io, &GPIO_InitStructure);
}

/******************************************************************************
* Function    : hal_drv_gpio_set
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : set gpio pin value
******************************************************************************/
void hal_gpio_set(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin, HAL_GPIO_VAL val)
{
    if (val == HAL_GPIO_LOW)
    {
        GPIO_ResetBits(GPIOx, GPIO_Pin);
    }
    else
    if (val == HAL_GPIO_HIGH)
    {
        GPIO_SetBits(GPIOx, GPIO_Pin);
    }
}

/******************************************************************************
* Function    : hal_gpio_val
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get gpio pin value
******************************************************************************/
HAL_GPIO_VAL hal_gpio_val(GPIO_TypeDef* GPIOx, uint16 GPIO_Pin)
{
    return (HAL_GPIO_VAL)GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
}

