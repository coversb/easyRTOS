/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hal_flash.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   flash driver
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-17      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include "hal_flash.h"
#include "os_middleware.h"
#include "os_trace_log.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
static OS_MUTEX_TYPE hal_flash_mutex = NULL;

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : hal_flash_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_flash_init(void)
{
    os_mutex_lock_init(&hal_flash_mutex);
}

/******************************************************************************
* Function    : hal_flash_deinit
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void hal_flash_deinit(void)
{
    os_mutex_lock_init(&hal_flash_mutex);
}

/******************************************************************************
* Function    : hal_flash_erase_page
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : erase one page of flash
******************************************************************************/
static bool hal_flash_erase_page(uint32 addr)
{
    FLASH_Status stat = FLASH_COMPLETE;
    
    os_mutex_lock(&hal_flash_mutex);
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    stat = FLASH_ErasePage(addr);
    FLASH_Lock();

    os_mutex_unlock(&hal_flash_mutex);

    if (stat != FLASH_COMPLETE)
    {
        OS_DBG_ERR(DBG_MOD_HAL, "flash erase err");
        return false;
    }

    return true;
}

/******************************************************************************
* Function    : hal_flash_read
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static uint32 hal_flash_read(uint32 addr, uint8 *buff, uint32 len)
{
    int DataNum = 0;

    os_mutex_lock(&hal_flash_mutex);
    
    while (DataNum < len) 
    {
        *(buff + DataNum) = *(volatile uint8*) addr++;
        DataNum++;
    }

    os_mutex_unlock(&hal_flash_mutex);
    
    return DataNum;
}

/******************************************************************************
* Function    : hal_flash_write
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : write data to flash
******************************************************************************/
int32 hal_flash_write(uint32 addr, uint32  *buff, uint32 len)
{
    uint16 i_write = 0;
    uint32 write_addr = addr;
    uint16 WordLength = (len + 3) / 4;

    hal_flash_erase_page(addr);

    os_mutex_lock(&hal_flash_mutex);
    
    FLASH_Unlock();
    for(i_write = 0; i_write < WordLength; i_write++)
    {
        FLASH_ProgramWord(write_addr, *buff);
        if((*(u32*)write_addr) != (*buff))
        {
            OS_DBG_ERR(DBG_MOD_HAL, "flash write err");
            goto flash_programe_EXIT;
        }
        write_addr += 4;
        buff++;
    }
flash_programe_EXIT:	
    FLASH_Lock();

    os_mutex_unlock(&hal_flash_mutex);

    if(i_write == WordLength)
    {
        return 0;
    }
    
    return (-i_write);
}

const HAL_FLASH_TYPE hwFlash = 
{
    hal_flash_init,
    hal_flash_deinit,
    hal_flash_erase_page,
    hal_flash_read,
    hal_flash_write
};

