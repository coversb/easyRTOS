/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_sw_i2c.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   software i2c
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-23      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include "board_config.h"
#include "hal_sw_i2c.h"
#include "hal_rcc.h"
#include "hal_gpio.h"
#include "os_trace_log.h"

/******************************************************************************
* Macro
******************************************************************************/
static void hal_sw_i2c_delay(void)
{
    uint32 delay = 5;
    while(delay) { delay--; }
}

#define SCL_HIGH(io, pin) hal_gpio_set(io, pin, HAL_GPIO_HIGH)
#define SCL_LOW(io, pin) hal_gpio_set(io, pin, HAL_GPIO_LOW)
#define SDA_HIGH(io, pin) hal_gpio_set(io, pin, HAL_GPIO_HIGH)
#define SDA_LOW(io, pin) hal_gpio_set(io, pin, HAL_GPIO_LOW)
#define SDA_VAL(io, pin) hal_gpio_val(io, pin)
#define I2C_DELAY() hal_sw_i2c_delay()

/******************************************************************************
* Function    : hal_sw_i2c_start
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool hal_sw_i2c_start(GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    SDA_HIGH(sdaIo, sdaPin);
    SCL_HIGH(sclIo, sclPin);
    
    I2C_DELAY();
    if (!SDA_VAL(sdaIo, sdaPin))
    {
        return false;
    }
    
    SDA_LOW(sdaIo, sdaPin);
    I2C_DELAY();
    if (SDA_VAL(sdaIo, sdaPin))
    {
        return false;
    }
    
    SDA_LOW(sdaIo, sdaPin);
    I2C_DELAY();

    return true;
}

/******************************************************************************
* Function    : hal_sw_i2c_stop
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_sw_i2c_stop(GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    SCL_LOW(sclIo, sclPin);
    I2C_DELAY();
    SDA_LOW(sdaIo, sdaPin);
    I2C_DELAY();

    SCL_HIGH(sclIo, sclPin);
    I2C_DELAY();
    SDA_HIGH(sdaIo, sdaPin);
    I2C_DELAY();
}

/******************************************************************************
* Function    : hal_sw_i2c_ack
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_sw_i2c_ack(GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    SCL_LOW(sclIo, sclPin);
    I2C_DELAY();
    SDA_LOW(sdaIo, sdaPin);
    I2C_DELAY();

    SCL_HIGH(sclIo, sclPin);
    I2C_DELAY();
    SCL_LOW(sclIo, sclPin);
    I2C_DELAY();
}

/******************************************************************************
* Function    : hal_sw_i2c_nack
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_sw_i2c_nack(GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    SCL_LOW(sclIo, sclPin);
    I2C_DELAY();
    SDA_HIGH(sdaIo, sdaPin);
    I2C_DELAY();

    SCL_HIGH(sclIo, sclPin);
    I2C_DELAY();
    SCL_LOW(sclIo, sclPin);
    I2C_DELAY();
}

/******************************************************************************
* Function    : hal_sw_i2c_wait_ack
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool hal_sw_i2c_wait_ack(GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    bool ret = false;
    
    SCL_LOW(sclIo, sclPin);
    I2C_DELAY();
    SDA_HIGH(sdaIo, sdaPin);
    I2C_DELAY();
    
    SCL_HIGH(sclIo, sclPin);
    I2C_DELAY();
    if (!SDA_VAL(sdaIo, sdaPin))
    {
        ret = true;
    }
    
    SCL_LOW(sclIo, sclPin);
    return ret;
}

/******************************************************************************
* Function    : hal_sw_i2c_send_byte
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_sw_i2c_send_byte(uint8 byte, GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    uint8 idx = 8;
    while (idx--)
    {
        SCL_LOW(sclIo, sclPin);
        I2C_DELAY();
        
        if(byte & 0x80)
        {
            SDA_HIGH(sdaIo, sdaPin);
        }
        else
        {
            SDA_LOW(sdaIo, sdaPin);
        }
        byte <<= 1;
        I2C_DELAY();
        SCL_HIGH(sclIo, sclPin);
        I2C_DELAY();
    }
    SCL_LOW(sclIo, sclPin);
}

/******************************************************************************
* Function    : hal_sw_i2c_recv_byte
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static uint8 hal_sw_i2c_recv_byte(GPIO_TypeDef* sclIo, uint16 sclPin, GPIO_TypeDef* sdaIo, uint16 sdaPin)
{
    uint8 idx = 8;
    uint8 byte = 0;

    SDA_HIGH(sdaIo, sdaPin);
    while (idx--)
    {
        byte <<= 1;
        SCL_LOW(sclIo, sclPin);
        I2C_DELAY();
        SCL_HIGH(sclIo, sclPin);
        I2C_DELAY();
        
        if (SDA_VAL(sdaIo, sdaPin))
        {
            byte |= 0x01;
        }
    }
    SCL_LOW(sclIo, sclPin);
    
    return byte;
}

#if (BOARD_SW_I2C1_ENABLE == 1)
/******************************************************************************
* Function    : hal_sw_i2c1_read_byte
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool hal_sw_i2c1_read_byte(uint8 devAddr, uint8 regAddr, uint8 *pdata)
{
    bool ret = true;
    
    if(!hal_sw_i2c_start(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    hal_sw_i2c_send_byte(devAddr, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    hal_sw_i2c_send_byte(regAddr, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    if(!hal_sw_i2c_start(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }
    hal_sw_i2c_send_byte(devAddr|0x01, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    *pdata = hal_sw_i2c_recv_byte(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    hal_sw_i2c_nack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);

err:
    hal_sw_i2c_stop(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);

    if (!ret)
    {
        OS_DBG_ERR(DBG_MOD_HAL, "I2C1 read byte err");
    }
    
    return ret;
}


/******************************************************************************
* Function    : hal_sw_i2c1_read_bytes
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool hal_sw_i2c1_read_bytes(uint8 devAddr, uint8 regAddr, uint8 *pdata, uint32 len)
{
    bool ret = true;
    
    if(!hal_sw_i2c_start(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }
    //dev addr
    hal_sw_i2c_send_byte(devAddr, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }
    //reg addr
    hal_sw_i2c_send_byte(regAddr, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }
    //start to read
    if(!hal_sw_i2c_start(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }
    hal_sw_i2c_send_byte(devAddr|0x01, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    for (uint8 idx = 0; idx < len - 1; ++idx)
    {
        *pdata++ = hal_sw_i2c_recv_byte(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
        hal_sw_i2c_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    }
    *pdata = hal_sw_i2c_recv_byte(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    hal_sw_i2c_nack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);


err:
    hal_sw_i2c_stop(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);

    if (!ret)
    {
        OS_DBG_ERR(DBG_MOD_HAL, "I2C1 read bytes err");
    }
    
    return ret;
}

/******************************************************************************
* Function    : hal_sw_i2c1_write_byte
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool hal_sw_i2c1_write_byte(uint8 devAddr, uint8 regAddr, uint8 data)
{
    bool ret = true;
    
    if(!hal_sw_i2c_start(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
    OS_INFO("start");
        ret = false;
        goto err;
    }

    hal_sw_i2c_send_byte(devAddr, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    hal_sw_i2c_send_byte(regAddr, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

    hal_sw_i2c_send_byte(data, BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);
    if (!hal_sw_i2c_wait_ack(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA))
    {
        ret = false;
        goto err;
    }

err:
    hal_sw_i2c_stop(BOARD_SW_I2C1_SCL, BOARD_SW_I2C1_SDA);

    if (!ret)
    {
        OS_DBG_ERR(DBG_MOD_HAL, "I2C1 write byte err");
    }
    
    return ret;
}

/******************************************************************************
* Function    : hal_sw_i2c1_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : init pin
******************************************************************************/
static void hal_sw_i2c1_init(void)
{
    hal_rcc_enable(BOARD_SW_I2C1_RCC);
    hal_gpio_set_mode(BOARD_SW_I2C1_SCL, GPIO_Mode_Out_OD);
    hal_gpio_set_mode(BOARD_SW_I2C1_SDA, GPIO_Mode_Out_OD);
}

const HAL_SW_I2C_TYPE swI2C1 = 
{
    hal_sw_i2c1_init,
    hal_sw_i2c1_read_byte,
    hal_sw_i2c1_read_bytes,
    hal_sw_i2c1_write_byte
};
#endif /*BOARD_SW_I2C1_ENABLE*/

