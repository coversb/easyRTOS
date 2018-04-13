/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          pb_trace.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   trace log
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2017-8-16             1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "basetype.h"
#include "os_config.h"
#include "os_trace_log.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
const char *TRACE_MOD_NAME[DBG_MOD_END + 1] =
{
    "OS",
    "NULL"
};

const char *TRACE_LV_NAME[DBG_END + 1] =
{
    "ERR",
    "WARN",
    "INFO",
    "NULL"
};

static uint32 os_trace_module = 0x00000000;
static uint32 os_trace_level = 0x00000000;

/******************************************************************************
* printf redirect to debug com begin
******************************************************************************/
#pragma import(__use_no_semihosting)             
struct __FILE 
{ 
    int handle; 
};

FILE __stdout;

void _sys_exit(int x) 
{ 
    x = x; 
} 

int fputc(int ch, FILE *f)
{
    uint8 str[1] = {0};
    str[0] = (uint8)ch;
    OS_TRACE_COM.write(str[0]);
    
    return ch;
}

void _ttywrch(int ch)
{
    ch = ch;
}

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : os_trace_log_set_mod
*
* Author      : Chen Hao
*
* Parameters  :  mode; 8 Bits debug level, 24 Bits debug module map
*   debug level 0 is highest, each level less than or equal to this level is allowed to output
*   each module has one bit in debug module map
*
* Return      :
*
* Description : set trace log mode
******************************************************************************/
void os_trace_log_set_mod(const uint32 mod, const uint32 level)
{
    os_trace_module = mod;
    os_trace_level = level;
}

/******************************************************************************
* Function    : os_trace_info
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description : print system trace log
******************************************************************************/
void os_trace_info(const char *fmt, ...)
{
    char buff[OS_TRACE_LOG_SIZE + 1];
    uint32 len;

    va_list va;
    va_start(va, fmt);
    len = vsnprintf(buff, OS_TRACE_LOG_SIZE, fmt, va);
    va_end(va);
    buff[len] = '\0';
    
    printf("%s\r\n", buff);
}

/******************************************************************************
* Function    : pb_trace_debug
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description : print system debug log
******************************************************************************/
void os_trace_debug(const uint8 idx, const uint8 level, const char *file, const uint32 line, const char *fmt, ...)
{
    if (idx >= DBG_MOD_END)
    {
        //bad log module
        return;
    }

    if (level != DBG_ERR)
    {
        if ((os_trace_module & (1 << idx)) == 0)    
        {
            //debug log is off
            return;
        }
        if (level >= os_trace_level)
        {
            //debug level is off
            return;
        }
    }

    char buff[OS_TRACE_LOG_SIZE + 1];
    uint32 len = 0;

    //add log contont
    len = sprintf(buff, "[%s][%s]:", TRACE_MOD_NAME[idx], TRACE_LV_NAME[level]);
    
    va_list va;
    va_start(va, fmt);
    len += vsnprintf((buff + len), (OS_TRACE_LOG_SIZE - len), fmt, va);
    va_end(va);
    
    //add file name and line
    char *pFileName = NULL;
    pFileName = strrchr (file, '\\');   //find the last '\' to get file name without path
    if (pFileName == NULL)
    {
        pFileName = (char*)file;
    }
    else
    {
        pFileName++;
    }
    len += snprintf((buff + len), (OS_TRACE_LOG_SIZE - len), " F-%s, L-%d", pFileName, line);
    buff[len] = '\0';
    
    printf("%s\r\n", buff);
}

/******************************************************************************
* Function    : os_trace_get_hex_str
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description : get hex data's string to print
******************************************************************************/
uint16 os_trace_get_hex_str(uint8 *str, uint16 strLen, uint8 *hex, uint16 hexLen)
{
    uint16 printLen = 0;
    memset(str, 0, strLen);

    for (uint16 idx = 0; idx < hexLen; ++idx)
    {
        printLen += snprintf((char*)&str[printLen], (strLen - printLen), "%02X", hex[idx]);
    }
    str[printLen] = '\0';

    return printLen;
}

