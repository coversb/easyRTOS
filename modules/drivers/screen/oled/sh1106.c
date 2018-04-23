/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          sh1106.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   sh1106 oled driver
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-23      1.00                     Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <string.h>
#include "board_config.h"
#include "os_trace_log.h"
#include "hal_sw_i2c.h"
#include "sh1106.h"
#include "font.h"

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
static HAL_SW_I2C_TYPE *SH1106_I2C = NULL;

/******************************************************************************
* Function    : sh1106_config
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_config(uint8 cmd)
{
    if (!SH1106_I2C->wirteByte(0x78, 0x00, cmd))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "SH1106 conf err[%d]", cmd);
    }
}

/******************************************************************************
* Function    : sh1106_write_data
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_write_data(uint8 data)
{
    if (!SH1106_I2C->wirteByte(0x78, 0x40, data))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "SH1106 write data err[%d]", data);
    }
}

/******************************************************************************
* Function    : sh1106_set_pos
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_set_pos(uint8 column,uint8 row)
{
    column *= 8;
    sh1106_config(0xb0 + row);
    sh1106_config(((column & 0xf0) >> 4) | 0x10);
    sh1106_config((column & 0x0f) | 0x02);
}

/******************************************************************************
* Function    : sh1106_reverse_byte
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static uint8 sh1106_reverse_byte(uint8 data)
{
    uint8 result = 0;
    
    for (uint8 idx =0; idx < 8; ++idx)
    {
        result = (result << 1) | ((data >> idx) & 0x01);
    }
    
    return result;
}

/******************************************************************************
* Function    : sh1106_show_bitmap
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_show_bitmap(uint8 column,uint8 row, uint8 *map)
{
    sh1106_set_pos(column, row * 2);
    for(uint8 idx = 0; idx < 8; ++idx)
    {
        sh1106_write_data(map[idx]);
    }

    sh1106_set_pos(column, row * 2 + 1);
    for(uint8 idx = 0; idx < 8; ++idx)
    {
        sh1106_write_data(map[8 + idx]);
    }
}

/******************************************************************************
* Function    : sh1106_show_bitmap_reverse
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_show_bitmap_reverse(uint8 column,uint8 row, uint8 *map)
{
    column = 15 - column;
    row = 3 - row;

    sh1106_set_pos(column, row * 2);
    for(uint8 idx = 0; idx < 8; ++idx)
    {
        sh1106_write_data(sh1106_reverse_byte(map[15 - idx]));
    }

    sh1106_set_pos(column, row * 2 + 1);
    for(uint8 idx = 0; idx < 8; ++idx)
    {
        sh1106_write_data(sh1106_reverse_byte(map[7 - idx]));
    }
}

/******************************************************************************
* Function    : sh1106_show
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_show(uint8 column, uint8 row, char *pdata, bool reverse)
{
    for (uint8 idx = 0; pdata[idx] != 0 && idx < 16 - column; idx++)
    {
        if (pdata[idx] >= ' ' && pdata[idx] <= '~')
        {
            if (reverse)
            {
                sh1106_show_bitmap_reverse(column + idx, row, (uint8*)&FONT_CODE[pdata[idx] - ' '][0]);
            }
            else
            {
                sh1106_show_bitmap(column + idx, row, (uint8*)&FONT_CODE[pdata[idx] - ' '][0]);
            }
        }
        else
        {
            if (reverse)
            {
                sh1106_show_bitmap_reverse(column + idx, row, (uint8*)&FONT_CODE[0][0]);
            }
            else
            {
                sh1106_show_bitmap(column + idx, row, (uint8*)&FONT_CODE[0][0]);
            }
        }            
    }
}

/******************************************************************************
* Function    : sh1106_clear
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void sh1106_clear(void)
{
    for(uint8 m = 0; m < 8; m++)
    {
        sh1106_config(0xb0 + m);    //page0-page1
        sh1106_config(0x02);    //low column start address
        sh1106_config(0x10);    //high column start address
        for(uint8 n = 0; n < 128; n++)
        {
            sh1106_write_data(0);
        }
    }
}

/******************************************************************************
* Function    : sh1106_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool sh1106_init(HAL_SW_I2C_TYPE *i2c)
{
    if (i2c == NULL)
    {
        OS_DBG_ERR(DBG_MOD_DEV, "SH1106 I2C is invalid");
        return false;
    }
    SH1106_I2C = i2c;
    SH1106_I2C->init();

    sh1106_config(0xAE);    /*display off*/
    sh1106_config(0x02);    /*set lower column address*/
    sh1106_config(0x10);    /*set higher column address*/
    sh1106_config(0x40);    /*set display start line*/
    sh1106_config(0xB0);    /*set page address*/
    sh1106_config(0x81);    /*contract control*/
    sh1106_config(0xCF);    /*128*/
    sh1106_config(0xA1);    /*set segment remap*/
    sh1106_config(0xA6);    /*normal / reverse*/
    sh1106_config(0xA8);    /*multiplex ratio*/
    sh1106_config(0x3F);    /*duty = 1/64*/
    sh1106_config(0xC8);    /*Com scan direction*/
    sh1106_config(0xD3);    /*set display offset*/
    sh1106_config(0x00);
    sh1106_config(0xD5);    /*set osc division*/
    sh1106_config(0x80);
    sh1106_config(0xD9);    /*set pre-charge period*/
    sh1106_config(0Xf1);
    sh1106_config(0xDA);    /*set COM pins*/
    sh1106_config(0x12);
    sh1106_config(0xdb);    /*set vcomh*/
    sh1106_config(0x40);
    sh1106_config(0x8d);    //…Ë÷√µÁ∫…±√
    sh1106_config(0x14);		//ø™∆ÙµÁ∫…±√
    sh1106_config(0xAF);    //OLEDªΩ–—
    
    sh1106_clear();
    return true;
}

const DEV_TYPE_SH1106 devSH1106 = 
{
    sh1106_init,
    sh1106_clear,
    sh1106_show
};

