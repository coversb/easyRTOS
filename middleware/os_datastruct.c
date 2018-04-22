/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          os_datastruct.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   define the common data structure
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-11      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <string.h>
#include "basetype.h"
#include "os_config.h"
#include "os_datastruct.h"
#include "os_middleware.h"
#include "os_trace_log.h"

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : os_ds_que_create
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : create a new queue datastruct for "q"
******************************************************************************/
uint16 os_ds_que_create(OS_DS_QUEUE_TYPE *q, uint8 *buff, uint16 size)
{
    q->buffSize = size;
    q->pBuff = buff;
    q->pHead = buff;
    q->pTail = buff;

    return 0;
}

/******************************************************************************
* Function    : os_ds_que_destroy
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : destroy queue
******************************************************************************/
uint16 os_ds_que_destroy(OS_DS_QUEUE_TYPE *q)
{
    q->buffSize = 0;
    q->pBuff = NULL;
    q->pHead = NULL;
    q->pTail = NULL;

    return 0;
}

/******************************************************************************
* Function    : os_ds_que_size
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get que size
******************************************************************************/
uint16 os_ds_que_size(OS_DS_QUEUE_TYPE *q)
{
    volatile uint8 *pHead = NULL;
    volatile uint8 *pTail = NULL;
    uint16 size = 0;

    pHead = q->pHead;
    pTail = q->pTail;
    
    if (pTail - pHead >= 0)
    {
        size = pTail - pHead;
    }
    else
    {
        size = pTail - pHead + q->buffSize;
    }

    return size;
}

/******************************************************************************
* Function    : os_ds_que_push
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : push one byte back to the q
******************************************************************************/
uint16 os_ds_que_push(OS_DS_QUEUE_TYPE *q, uint8 byte)
{
    volatile uint8 *pTail = NULL;

    pTail = q->pTail;

    if (++pTail >= (q->pBuff + q->buffSize))//back to buffer area header
    {
        pTail = q->pBuff;
    }

    if (pTail == q->pHead) //que is full
    {
        return 0;
    }

    *(q->pTail) = byte;

    q->pTail = pTail;

    return 1;
}

/******************************************************************************
* Function    : os_ds_que_pop
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : pop one byte from q front
******************************************************************************/
uint16 os_ds_que_pop(OS_DS_QUEUE_TYPE *q)
{
    uint8 byte = 0;
    
    if (q->pHead != q->pTail)
    {
        byte = *(q->pHead);
        q->pHead++;
        
        if (q->pHead >= q->pBuff + q->buffSize) 
        {
            q->pHead= q->pBuff;
        }
    }

    return byte;
}

/******************************************************************************
* Function    : os_ds_que_packet_in
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : put bytes back to the queue
******************************************************************************/
uint16 os_ds_que_packet_in(OS_DS_QUEUE_TYPE *q, uint8 *buff, uint16 len)
{
    volatile uint8 *pTail = NULL;
    uint16 idx = 0;

    pTail = q->pTail;
    
    for (idx = 0; idx < len; ++idx)
    {
        if (++pTail >= q->pBuff + q->buffSize)
        {
            pTail = q->pBuff;
        }
        if (pTail == q->pHead) 
        {
            break;
        }
        
        *(q->pTail) = *(buff);
        buff++;
        
        q->pTail = pTail;
    }
    
    return idx;
}

/******************************************************************************
* Function    : os_ds_que_packet_out
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get bytes from the queue front
******************************************************************************/
uint16 os_ds_que_packet_out(OS_DS_QUEUE_TYPE *q, uint8 *buff, uint16 len)
{
    uint16 idx = 0;

    while ((q->pHead != q->pTail) && (idx < len) && (idx < q->buffSize))
    {
        buff[idx++] = *(q->pHead);
        q->pHead++;
        
        if (q->pHead >= q->pBuff + q->buffSize) 
        {
            q->pHead = q->pBuff;
        }
    }

    return idx;
}

/******************************************************************************
* Function    : os_ds_list_que_size
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
uint16 os_ds_list_que_size(OS_DS_LIST_QUEUE_TYPE *q)
{
    return q->queueSize;
}

/******************************************************************************
* Function	  : os_ds_list_que_append
* 
* Author	  : Chen Hao
* 
* Parameters  : 
* 
* Return	  : 
* 
* Description : 
******************************************************************************/
bool os_ds_list_que_append(OS_DS_LIST_QUEUE_TYPE *q, const uint16 queMaxSize, const uint8* data, uint16 maxLen, uint16 len)
{
    if (len == 0)
    {
        return false;
    }
    
    bool ret = false;
    uint32 malloc_size = 0;
    OS_DS_LIST_NODE_TYPE *addNode_p = NULL;

    if ((q->queueSize + 1) > queMaxSize)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Que full[%d], max[%d]", q->queueSize, queMaxSize);
        goto addEnd;
    }

    malloc_size = MIN_VALUE(maxLen, len);

    addNode_p = (OS_DS_LIST_NODE_TYPE*)os_malloc(sizeof(OS_DS_LIST_NODE_TYPE) + malloc_size);
    if (NULL == addNode_p)
    {
        OS_DBG_ERR(DBG_MOD_OS, "malloc err");
        goto addEnd;
    }

    addNode_p->length = malloc_size;
    addNode_p->nextNode_p = NULL;
    memcpy((void*)addNode_p->data, data, malloc_size);

    if (NULL == q->tail_p)
    {
        q->head_p = addNode_p;
        q->tail_p = addNode_p;
    }
    else
    {
        q->tail_p->nextNode_p = addNode_p;
        q->tail_p = addNode_p;
    }
    q->queueSize++;

    ret = true;
addEnd:

    return ret;
}

/******************************************************************************
* Function	  : os_ds_list_que_remove_head
* 
* Author	  : Chen Hao
* 
* Parameters  : 
* 
* Return	  : 
* 
* Description : 
******************************************************************************/
void os_ds_list_que_remove_head(OS_DS_LIST_QUEUE_TYPE *q)
{
    OS_DS_LIST_NODE_TYPE *delNode_p = NULL;

    if ((0 != q->queueSize) && (NULL != q->head_p))
    {
        delNode_p = q->head_p;
        q->head_p = delNode_p->nextNode_p;

        if (q->tail_p == delNode_p)
        {
            q->head_p = NULL;
            q->tail_p = q->head_p;
            q->queueSize = 0;
        }
        else
        {
            q->queueSize--;
        }
        os_free(delNode_p);
    }
}

/******************************************************************************
* Function    : os_ds_list_que_head_data
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool os_ds_list_que_head_data(OS_DS_LIST_QUEUE_TYPE *q, uint8 **pdata, uint16 *len)
{
    if (os_ds_list_que_size(q) == 0)
    {
        OS_DBG_ERR(DBG_MOD_OS, "Que is empty");
        return false;
    }
    
    (*pdata) = q->head_p->data;
    (*len) = q->head_p->length;
    
    return true;
}

