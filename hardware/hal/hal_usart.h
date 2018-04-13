/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_usart.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   usart driver
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-12      1.00                    Chen Hao
*
******************************************************************************/
#ifndef __HAL_USART_H__
#define __HAL_USART_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "basetype.h"
#include "board_config.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Types
******************************************************************************/
typedef struct
{
    void (*begin)(uint32 baundrate);
    void (*end)(void);
    uint16 (*available)(void);
    void (*write)(uint8 byte);
    uint16 (*writeBytes)(uint8 *buff, uint16 len);
    uint8 (*read)(void);
    uint16 (*readBytes)(uint8 *buff, uint16 len);
    void (*print)(char* str);
    void (*println)(char* str);
}HAL_USART_TYPE;

/******************************************************************************
* Extern variable
******************************************************************************/
extern const HAL_USART_TYPE hwSerial1;
extern const HAL_USART_TYPE hwSerial2;
extern const HAL_USART_TYPE hwSerial3;
extern const HAL_USART_TYPE hwSerial5;

#endif /*__HAL_USART_H__*/

