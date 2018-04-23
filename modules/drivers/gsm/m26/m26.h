/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          m26.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   quectel m26 operator
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-20      1.00                    Chen Hao
*
******************************************************************************/
#ifndef __M26_H__
#define __M26_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "basetype.h"
#include "hal_usart.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Enums
******************************************************************************/

/******************************************************************************
* Types
******************************************************************************/
typedef struct
{
    bool (*init)(HAL_USART_TYPE *com, const uint32 baudrate);
    bool (*reset)(void);
    bool (*config)(const char *apn, const char *user, const char *pass);
    bool (*connect)(const char *domain, const uint16 port);
    bool(*disconnect)(void);
    uint16 (*recv)(uint8 *pdata, const uint16 max);
    uint16 (*send)(const uint8 *pdata, const uint16 size);
    bool (*isConnected)(void);
    void (*getCsq)(uint8 *rssi, uint8 *ber);
    void (*getModulInfo)(char *ver, const uint8 verLen, char *imei, const uint8 imeiLen, char *imsi, const uint8 imsiLen, char *iccid, const uint8 iccidLen);
    bool (*getCellularLocation)(char *longitude, char *latitude);
}DEV_TYPE_M26;

/******************************************************************************
* Global Variables
******************************************************************************/
extern const DEV_TYPE_M26 devM26;

#endif /* __M26_H__ */

