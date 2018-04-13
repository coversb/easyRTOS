/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          os_datastruct.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   define the common data struct
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-11      1.00                    Chen Hao
*
******************************************************************************/
#ifndef __OS_DATASTRUCT_H__
#define __OS_DATASTRUCT_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "basetype.h"

/******************************************************************************
* Types
******************************************************************************/
typedef volatile struct
{
    volatile uint16 buffSize;
    volatile uint8 *pHead;
    volatile uint8 *pTail;
    volatile uint8 *pBuff;
}OS_DS_QUEUE_TYPE;

typedef struct os_ds_list_node_t
{
	struct os_ds_list_node_t *nextNode_p;
	uint16 length;
	uint8 data[1];
}OS_DS_LIST_NODE_TYPE;

typedef struct os_ds_list_queue_t
{
	uint8 queueSize;
	OS_DS_LIST_NODE_TYPE *head_p;
	OS_DS_LIST_NODE_TYPE *tail_p;
}OS_DS_LIST_QUEUE_TYPE;

/******************************************************************************
* Global Functions
******************************************************************************/
extern uint16 os_ds_que_create(OS_DS_QUEUE_TYPE *q, uint8 *buff, uint16 size);
extern uint16 os_ds_que_destroy(OS_DS_QUEUE_TYPE *q);
extern uint16 os_ds_que_size(OS_DS_QUEUE_TYPE *q);
extern uint16 os_ds_que_push(OS_DS_QUEUE_TYPE *q, uint8 byte);
extern uint16 os_ds_que_pop(OS_DS_QUEUE_TYPE *q);
extern uint16 os_ds_que_packet_in(OS_DS_QUEUE_TYPE *q, uint8 *buff, uint16 len);
extern uint16 os_ds_que_packet_out(OS_DS_QUEUE_TYPE *q, uint8 *buff, uint16 len);

extern uint16 os_ds_list_que_size(OS_DS_LIST_QUEUE_TYPE *q);
extern bool os_ds_list_que_append(OS_DS_LIST_QUEUE_TYPE *q, const uint16 queMaxSize, const uint8* data, uint16 maxLen, uint16 len);
extern void os_ds_list_que_remove_head(OS_DS_LIST_QUEUE_TYPE *q);
extern bool os_ds_list_que_head_data(OS_DS_LIST_QUEUE_TYPE *q, uint8 **pdata, uint16 *len);

#endif /*__OS_DATASTRUCT_H__*/

