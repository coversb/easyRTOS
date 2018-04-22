/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          os_trace_log.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   trace log
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-11      1.00                    Chen Hao
*
******************************************************************************/
#ifndef __OS_TRACE_LOG_H__
#define __OS_TRACE_LOG_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "basetype.h"
#include "os_config.h"

/******************************************************************************
* Macros
******************************************************************************/
#if ( OS_TRACE_LOG == 1 )
//os info
#define OS_INFO(...) os_trace_info(__VA_ARGS__)
//os debug log
#define OS_DBG_ERR(DBG_IDX, ...)  os_trace_debug(DBG_IDX, DBG_ERR, __FILE__, __LINE__, __VA_ARGS__)
#define OS_DBG_TRACE(DBG_IDX, DBG_LV, ...)  os_trace_debug(DBG_IDX, DBG_LV, __FILE__, __LINE__, __VA_ARGS__)
#else
#define OS_INFO(...) 
#define OS_DBG_ERR(DBG_IDX, ...)  
#define OS_DBG_TRACE(DBG_IDX, DBG_LV, ...)  
#endif /*OS_TRACE_LOG*/

/******************************************************************************
* Enums
******************************************************************************/
typedef enum
{
    DBG_MOD_OS,     //0x0001 os log
    DBG_MOD_HAL,   //0x0002 hal log
    DBG_MOD_DEV,
    DBG_MOD_END
}DBG_TRACE_MOD;

typedef enum
{
    DBG_ERR = 0,
    DBG_WARN,
    DBG_INFO,
    DBG_END
}DBG_TRACE_LEVEL;
    
/******************************************************************************
* Types
******************************************************************************/

/******************************************************************************
* Global Variables 
******************************************************************************/


/******************************************************************************
* Global Functions
******************************************************************************/
extern void os_trace_log_set_mod(const uint32 mod, const uint32 level);
extern void os_trace_info(const char *fmt, ...);
extern void os_trace_debug(const uint8 idx, const uint8 level, const char *file, const uint32 line, const char *fmt, ...);
extern uint16 os_trace_get_hex_str(uint8 *str, uint16 strLen, uint8 *hex, uint16 hexLen);

#endif /* __OS_TRACE_LOG_H__ */

