/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_usart.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   usart driver
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-12      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <string.h>
#include "hal_usart.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "os_datastruct.h"
#include "os_middleware.h"

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
* Function    : hal_usart_config
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : config usart function
******************************************************************************/
static void hal_usart_config(USART_TypeDef* USARTx, u32 baundrate)
{
    USART_DeInit(USARTx);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baundrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //8 data bit
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  //stop bit
    USART_InitStructure.USART_Parity = USART_Parity_No; //no parity
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);
}

#if ( BOARD_USART1_ENABLE == 1 )
/*USART1 begin*/
static OS_DS_QUEUE_TYPE hal_usart1_rx_que;
static uint8 HAL_USART1_RX_BUFF[BOARD_USART1_RX_BUFFSIZE];
static OS_MUTEX_TYPE hal_usart1_mutex = NULL;

/******************************************************************************
* Function    : hal_usart1_begin
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : setup and enable usart1 with baundrate
******************************************************************************/
static void hal_usart1_begin(u32 baundrate)
{
    os_mutex_lock_init(&hal_usart1_mutex);
    memset(HAL_USART1_RX_BUFF, 0x00, sizeof(HAL_USART1_RX_BUFF));
    os_ds_que_create(&hal_usart1_rx_que, HAL_USART1_RX_BUFF, sizeof(HAL_USART1_RX_BUFF));

    /*RCC config*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);

    /*GPIO config*/
    hal_gpio_set_mode(BOARD_USART1_TX, GPIO_Mode_AF_PP);
    hal_gpio_set_mode(BOARD_USART1_RX, GPIO_Mode_IN_FLOATING);

    /*Usart 1 config*/
    hal_usart_config(USART1, baundrate);
    //enable rx irq
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    //enable uasrt1
    USART_Cmd(USART1, ENABLE);
    //clear send flag
    USART_ClearFlag(USART1, USART_FLAG_TC);      

    /*irq priority config*/
    hal_board_nvic_set_irq(USART1_IRQn, BOARD_IQR_PRIO_USART1, BOARD_IQR_SUB_PRIO_USART1, ENABLE);
}

/******************************************************************************
* Function    : hal_usart1_end
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : disable usart1
******************************************************************************/
static void hal_usart1_end(void)
{
    USART_Cmd(USART1, DISABLE);
    os_ds_que_destroy(&hal_usart1_rx_que);
    memset(HAL_USART1_RX_BUFF, 0x00, sizeof(HAL_USART1_RX_BUFF));
    os_mutex_lock_deinit(&hal_usart1_mutex);
}

/******************************************************************************
* Function    : hal_usart1_available
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get queue vaild data size
******************************************************************************/
static uint16 hal_usart1_available(void)
{
    return os_ds_que_size(&hal_usart1_rx_que);
}

/******************************************************************************
* Function    : hal_usart1_write
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : wtire one byte to usart1
******************************************************************************/
static void hal_usart1_write(uint8 byte)
{
    os_mutex_lock(&hal_usart1_mutex);
    
    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_SendData(USART1, byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {};

    os_mutex_unlock(&hal_usart1_mutex);
}

/******************************************************************************
* Function    : hal_usart1_write_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : write bytes to USART1
******************************************************************************/
static uint16 hal_usart1_write_bytes(uint8 *buff, uint16 length)
{
    uint16 i = 0;

    os_mutex_lock(&hal_usart1_mutex);

    USART_ClearFlag(USART1, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART1, buff[i]);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){};
    }
    
    os_mutex_unlock(&hal_usart1_mutex);

    return length;
}

/******************************************************************************
* Function    : hal_usart1_read
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read one byte from usart1
******************************************************************************/
static uint8 hal_usart1_read(void)
{
    uint8 byte;
    
    os_mutex_lock(&hal_usart1_mutex);

    byte = os_ds_que_pop(&hal_usart1_rx_que);

    os_mutex_unlock(&hal_usart1_mutex);

    return byte;
}

/******************************************************************************
* Function    : hal_usart1_read_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read bytes to buffer
******************************************************************************/
static uint16 hal_usart1_read_bytes(uint8 *buff, uint16 length)
{
    uint16 len = 0;
    
    os_mutex_lock(&hal_usart1_mutex);
    
    len = os_ds_que_packet_out(&hal_usart1_rx_que, (uint8*)buff, length);
    
    os_mutex_unlock(&hal_usart1_mutex);

    return len;
}

/******************************************************************************
* Function    : hal_usart1_print
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_usart1_print(char* str)
{
    uint16 i = 0;
    uint16 length = strlen(str);

    os_mutex_lock(&hal_usart1_mutex);

    USART_ClearFlag(USART1, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART1, str[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {};
    }
    
    os_mutex_unlock(&hal_usart1_mutex);
}

/******************************************************************************
* Function    : hal_usart1_println
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_usart1_println(char* str)
{
    hal_usart1_print(str);
    hal_usart1_print("\r\n");
}

const HAL_USART_TYPE hwSerial1 = 
{
    hal_usart1_begin,
    hal_usart1_end,
    hal_usart1_available,
    hal_usart1_write,
    hal_usart1_write_bytes,
    hal_usart1_read,
    hal_usart1_read_bytes,
    hal_usart1_print,
    hal_usart1_println
};
/*USART1 end*/
#endif /*BOARD_USART1_ENABLE*/

#if ( BOARD_USART2_ENABLE == 1 )
/*USART2 begin*/
static OS_DS_QUEUE_TYPE hal_usart2_rx_que;
static uint8 HAL_USART2_RX_BUFF[BOARD_USART2_RX_BUFFSIZE];
static OS_MUTEX_TYPE hal_usart2_mutex = NULL;

/******************************************************************************
* Function    : hal_usart2_begin
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : setup and enable usart2 with baundrate
******************************************************************************/
static void hal_usart2_begin(u32 baundrate)
{
    os_mutex_lock_init(&hal_usart2_mutex);
    
    memset(HAL_USART2_RX_BUFF, 0x00, sizeof(HAL_USART2_RX_BUFF));
    os_ds_que_create(&hal_usart2_rx_que, HAL_USART2_RX_BUFF, sizeof(HAL_USART2_RX_BUFF));

    /*RCC config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);

    /*GPIO config*/
    hal_gpio_set_mode(BOARD_USART2_TX, GPIO_Mode_AF_PP);   //USART2 TX
    hal_gpio_set_mode(BOARD_USART2_RX, GPIO_Mode_IN_FLOATING);    //USART2 RX

    /*Usart 2 config*/
    hal_usart_config(USART2, baundrate);
    //enable rx irq
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    //enable uasrt1
    USART_Cmd(USART2, ENABLE);
    //clear send flag
    USART_ClearFlag(USART2, USART_FLAG_TC);      

    /*irq priority config*/
    hal_board_nvic_set_irq(USART2_IRQn, BOARD_IQR_PRIO_USART2, BOARD_IQR_SUB_PRIO_USART2, ENABLE);
}

/******************************************************************************
* Function    : hal_usart2_end
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : disable usart2
******************************************************************************/
static void hal_usart2_end(void)
{
    USART_Cmd(USART2, DISABLE);
    os_ds_que_destroy(&hal_usart2_rx_que);
    memset(HAL_USART2_RX_BUFF, 0x00, sizeof(HAL_USART2_RX_BUFF));
    
    os_mutex_lock_deinit(&hal_usart2_mutex);
}

/******************************************************************************
* Function    : hal_usart2_available
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get queue vaild data size
******************************************************************************/
static uint16 hal_usart2_available(void)
{
    return os_ds_que_size(&hal_usart2_rx_que);
}

/******************************************************************************
* Function    : hal_usart2_write
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : wtire one byte to usart2
******************************************************************************/
static void hal_usart2_write(uint8 byte)
{
    os_mutex_lock(&hal_usart2_mutex);
    
    USART_ClearFlag(USART2, USART_FLAG_TC);
    USART_SendData(USART2, byte);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){};

    os_mutex_unlock(&hal_usart2_mutex);
}

/******************************************************************************
* Function    : hal_usart2_write_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : write bytes to USART2
******************************************************************************/
static uint16 hal_usart2_write_bytes(uint8 *buff, uint16 length)
{
    uint16 i = 0;

    os_mutex_lock(&hal_usart2_mutex);

    USART_ClearFlag(USART2, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART2, buff[i]);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {};
    }
    
    os_mutex_unlock(&hal_usart2_mutex);

    return length;
}

/******************************************************************************
* Function    : hal_usart2_read
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read one byte from usart2
******************************************************************************/
static uint8 hal_usart2_read(void)
{
    uint8 byte;
    
    os_mutex_lock(&hal_usart2_mutex);
    
    byte = os_ds_que_pop(&hal_usart2_rx_que);

    os_mutex_unlock(&hal_usart2_mutex);

    return byte;
}

/******************************************************************************
* Function    : hal_usart2_read_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read bytes to buffer
******************************************************************************/
static uint16 hal_usart2_read_bytes(uint8 *buff, uint16 length)
{
    uint16 len = 0;
    
    os_mutex_lock(&hal_usart2_mutex);
    
    len = os_ds_que_packet_out(&hal_usart2_rx_que, (uint8*)buff, length);

    os_mutex_unlock(&hal_usart2_mutex);

    return len;
}

/******************************************************************************
* Function    : hal_usart2_print
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_usart2_print(char* str)
{
    uint16 i = 0;
    uint16 length = strlen(str);

    os_mutex_lock(&hal_usart2_mutex);

    USART_ClearFlag(USART2, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART2, str[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){};
    }
    
    os_mutex_unlock(&hal_usart2_mutex);
}

/******************************************************************************
* Function    : hal_usart2_println
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_usart2_println(char* str)
{
    hal_usart2_print(str);
    hal_usart2_print("\r\n");
}

const HAL_USART_TYPE hwSerial2 = 
{
    hal_usart2_begin,
    hal_usart2_end,
    hal_usart2_available,
    hal_usart2_write,
    hal_usart2_write_bytes,
    hal_usart2_read,
    hal_usart2_read_bytes,
    hal_usart2_print,
    hal_usart2_println
};
/*USART2 end*/
#endif /*BOARD_USART2_ENABLE*/

#if ( BOARD_USART3_ENABLE == 1 )
/*USART3 begin*/
static OS_DS_QUEUE_TYPE hal_usart3_rx_que;
static uint8 HAL_USART3_RX_BUFF[BOARD_USART3_RX_BUFFSIZE];
static OS_MUTEX_TYPE hal_usart3_mutex = NULL;

/******************************************************************************
* Function    : hal_usart3_begin
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : setup and enable usart3 with baundrate
******************************************************************************/
static void hal_usart3_begin(u32 baundrate)
{
    os_mutex_lock_init(&hal_usart3_mutex);
    
    memset(HAL_USART3_RX_BUFF, 0x00, sizeof(HAL_USART3_RX_BUFF));
    os_ds_que_create(&hal_usart3_rx_que, HAL_USART3_RX_BUFF, sizeof(HAL_USART3_RX_BUFF));

    /*RCC config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);

    /*GPIO config*/
    hal_gpio_set_mode(BOARD_USART3_TX, GPIO_Mode_AF_PP);   //USART3 TX
    hal_gpio_set_mode(BOARD_USART3_RX, GPIO_Mode_IN_FLOATING);    //USART3 RX

    /*Usart 3 config*/
    hal_usart_config(USART3, baundrate);
    //enable dma for usart3
    //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
    //USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
    //USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
    //enable idle irq
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
    //enable uasrt3
    USART_Cmd(USART3, ENABLE);
    //clear send flag
    USART_ClearFlag(USART3, USART_FLAG_TC);

    /*irq priority config*/
    hal_board_nvic_set_irq(USART3_IRQn, BOARD_IQR_PRIO_USART3, BOARD_IQR_SUB_PRIO_USART3, ENABLE);
}

/******************************************************************************
* Function    : hal_usart3_end
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : disable usart3
******************************************************************************/
static void hal_usart3_end(void)
{
    //USART_DMACmd(USART3, USART_DMAReq_Tx, DISABLE);
    //USART_DMACmd(USART3, USART_DMAReq_Rx, DISABLE);

    USART_Cmd(USART3, DISABLE);
    os_ds_que_destroy(&hal_usart3_rx_que);
    memset(HAL_USART3_RX_BUFF, 0x00, sizeof(HAL_USART3_RX_BUFF));
    
    os_mutex_lock_deinit(&hal_usart3_mutex);
}

/******************************************************************************
* Function    : hal_usart3_available
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get queue vaild data size
******************************************************************************/
static uint16 hal_usart3_available(void)
{
    return os_ds_que_size(&hal_usart3_rx_que);
}

/******************************************************************************
* Function    : hal_usart3_write
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : wtire one byte to usart3
******************************************************************************/
static void hal_usart3_write(uint8 byte)
{
    os_mutex_lock(&hal_usart3_mutex);

    USART_ClearFlag(USART3, USART_FLAG_TC);
    USART_SendData(USART3, byte);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET){};

    os_mutex_unlock(&hal_usart3_mutex);
}

/******************************************************************************
* Function    : hal_usart3_write_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : write bytes to USART3
******************************************************************************/
static uint16 hal_usart3_write_bytes(uint8 *buff, uint16 length)
{
    uint16 i = 0;

    os_mutex_lock(&hal_usart3_mutex);

    USART_ClearFlag(USART3, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART3, buff[i]);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET){};
    }

    os_mutex_unlock(&hal_usart3_mutex);

    return length;
}

/******************************************************************************
* Function    : hal_usart3_read
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read one byte from usart3
******************************************************************************/
static uint8 hal_usart3_read(void)
{
    uint8 byte;
    
    os_mutex_lock(&hal_usart3_mutex);

    byte = os_ds_que_pop(&hal_usart3_rx_que);

    os_mutex_unlock(&hal_usart3_mutex);

    return byte;
}

/******************************************************************************
* Function    : hal_usart3_read_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read bytes to buffer
******************************************************************************/
static uint16 hal_usart3_read_bytes(uint8 *buff, uint16 length)
{
    uint16 len = 0;
    
    os_mutex_lock(&hal_usart3_mutex);
    
    len = os_ds_que_packet_out(&hal_usart3_rx_que, (uint8*)buff, length);
    
    os_mutex_unlock(&hal_usart3_mutex);
    
    return len;
}

/******************************************************************************
* Function    : hal_usart1_print
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_usart3_print(char* str)
{
    uint16 i = 0;
    uint16 length = strlen(str);

    os_mutex_lock(&hal_usart3_mutex);

    USART_ClearFlag(USART3, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART3, str[i]);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET){};
    }
    
    os_mutex_unlock(&hal_usart3_mutex);
}

/******************************************************************************
* Function    : hal_usart3_println
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_usart3_println(char* str)
{
    hal_usart3_print(str);
    hal_usart3_print("\r\n");
}

const HAL_USART_TYPE hwSerial3 = 
{
    hal_usart3_begin,
    hal_usart3_end,
    hal_usart3_available,
    hal_usart3_write,
    hal_usart3_write_bytes,
    hal_usart3_read,
    hal_usart3_read_bytes,
    hal_usart3_print,
    hal_usart3_println
};
/*USART3 end*/
#endif /*BOARD_USART3_ENABLE*/

#if ( BOARD_USART5_ENABLE == 1 )
/*UART5 begin*/
static OS_DS_QUEUE_TYPE hal_uart5_rx_que;
static uint8 HAL_UART5_RX_BUFF[BOARD_UART5_RX_BUFFSIZE];
static OS_MUTEX_TYPE hal_usart5_mutex = NULL;

/******************************************************************************
* Function    : hal_uart5_begin
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : setup and enable uart5 with baundrate
******************************************************************************/
static void hal_uart5_begin(u32 baundrate)
{
    os_mutex_lock_init(&hal_usart5_mutex);
    
    memset(HAL_UART5_RX_BUFF, 0x00, sizeof(HAL_UART5_RX_BUFF));
    os_ds_que_create(&hal_uart5_rx_que, HAL_UART5_RX_BUFF, sizeof(HAL_UART5_RX_BUFF));

    /*RCC config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);

    /*GPIO config*/
    hal_gpio_set_mode(BOARD_UART5_TX, GPIO_Mode_AF_PP);   //UART5 TX
    hal_gpio_set_mode(BOARD_UART5_RX, GPIO_Mode_IN_FLOATING);    //UART5 RX

    /*Uart5 config*/
    hal_usart_config(UART5, baundrate);
    //enable idle irq
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
    //enable uasrt3
    USART_Cmd(UART5, ENABLE);
    //clear send flag
    USART_ClearFlag(UART5, USART_FLAG_TC);

    /*irq priority config*/
    hal_board_nvic_set_irq(UART5_IRQn, BOARD_IQR_PRIO_UART5, BOARD_IQR_SUB_PRIO_UART5, ENABLE);
}

/******************************************************************************
* Function    : hal_uart5_end
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : disable uart5
******************************************************************************/
static void hal_uart5_end(void)
{
    USART_Cmd(UART5, DISABLE);
    os_ds_que_destroy(&hal_uart5_rx_que);
    memset(HAL_UART5_RX_BUFF, 0x00, sizeof(HAL_UART5_RX_BUFF));
    
    os_mutex_lock_deinit(&hal_usart5_mutex);
}

/******************************************************************************
* Function    : hal_uart5_available
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get queue vaild data size
******************************************************************************/
static uint16 hal_uart5_available(void)
{
    return os_ds_que_size(&hal_uart5_rx_que);
}

/******************************************************************************
* Function    : hal_uart5_write
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : wtire one byte to uart5
******************************************************************************/
static void hal_uart5_write(uint8 byte)
{
    os_mutex_lock(&hal_usart5_mutex);

    USART_ClearFlag(UART5, USART_FLAG_TC);
    USART_SendData(UART5, byte);
    while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET) {};

    os_mutex_unlock(&hal_usart5_mutex);
}

/******************************************************************************
* Function    : hal_uart5_write_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : write bytes to UART5
******************************************************************************/
static uint16 hal_uart5_write_bytes(uint8 *buff, uint16 length)
{
    uint16 i = 0;

    os_mutex_lock(&hal_usart5_mutex);

    USART_ClearFlag(UART5, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(UART5, buff[i]);
        while(USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET){};
    }

    os_mutex_unlock(&hal_usart5_mutex);

    return length;
}

/******************************************************************************
* Function    : hal_uart5_read
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read one byte from uart5
******************************************************************************/
static uint8 hal_uart5_read(void)
{
    uint8 byte;
    
    os_mutex_lock(&hal_usart5_mutex);

    byte = os_ds_que_pop(&hal_uart5_rx_que);

    os_mutex_unlock(&hal_usart5_mutex);

    return byte;
}

/******************************************************************************
* Function    : hal_uart5_read_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : read bytes to buffer
******************************************************************************/
static uint16 hal_uart5_read_bytes(uint8 *buff, uint16 length)
{
    uint16 len = 0;
    
    os_mutex_lock(&hal_usart5_mutex);
    
    len = os_ds_que_packet_out(&hal_uart5_rx_que, (uint8*)buff, length);
    
    os_mutex_unlock(&hal_usart5_mutex);
    
    return len;
}

/******************************************************************************
* Function    : hal_uart5_print
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_uart5_print(char* str)
{
    uint16 i = 0;
    uint16 length = strlen(str);

    os_mutex_lock(&hal_usart5_mutex);

    USART_ClearFlag(UART5, USART_FLAG_TC);
    for (i = 0; i < length; i++)
    {
        USART_SendData(UART5, str[i]);
        while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET) {};
    }
    
    os_mutex_unlock(&hal_usart5_mutex);
}

/******************************************************************************
* Function    : hal_uart5_println
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_uart5_println(char* str)
{
    hal_uart5_print(str);
    hal_uart5_print("\r\n");
}

const HAL_USART_TYPE hwSerial5 = 
{
    hal_uart5_begin,
    hal_uart5_end,
    hal_uart5_available,
    hal_uart5_write,
    hal_uart5_write_bytes,
    hal_uart5_read,
    hal_uart5_read_bytes,
    hal_uart5_print,
    hal_uart5_println
};
/*USART5 end*/
#endif /*BOARD_USART5_ENABLE*/

/******************************************************************************
* Function    : USART1_IRQHandler
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : USART1 irq handler
******************************************************************************/
void USART1_IRQHandler(void)
{
    #if ( BOARD_USART1_ENABLE == 1 )
    uint8 res = 0;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        res = USART_ReceiveData(USART1);
        os_ds_que_push(&hal_usart1_rx_que, res);
    }
    #endif /*BOARD_USART1_ENABLE*/
}

/******************************************************************************
* Function    : USART2_IRQHandler
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : USART2 irq handler
******************************************************************************/
void USART2_IRQHandler(void)
{
    #if ( BOARD_USART2_ENABLE == 1 )
    uint8 res = 0;
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        res = USART_ReceiveData(USART2);
        os_ds_que_push(&hal_usart2_rx_que, res);
    }
    #endif /*BOARD_USART2_ENABLE*/
}

/******************************************************************************
* Function    : USART3_IRQHandler
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : USART3 irq handler
******************************************************************************/
void USART3_IRQHandler(void)
{
    #if ( BOARD_USART3_ENABLE == 1 )
    uint8 res = 0;
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        res = USART_ReceiveData(USART3);
        os_ds_que_push(&hal_usart3_rx_que, res);
    }
    #endif /*BOARD_USART3_ENABLE*/
}

/******************************************************************************
* Function    : UART5_IRQHandler
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : UART5 irq handler
******************************************************************************/
void UART5_IRQHandler(void)
{
    #if ( BOARD_USART5_ENABLE == 1 )
    uint8 res = 0;
    if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    {
        res = USART_ReceiveData(UART5);
        os_ds_que_push(&hal_uart5_rx_que, res);
    }
    #endif /*BOARD_USART5_ENABLE*/
}

