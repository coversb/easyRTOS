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
/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board_config.h"
#include "hal_rcc.h"
#include "hal_gpio.h"
#include "os_middleware.h"
#include "os_trace_log.h"
#include "m26.h"

/******************************************************************************
* Macros
******************************************************************************/
#define M26_RSP_MAX_LEN 128
#define M26_AT_CMD_MAX_LEN 128

#define M26_ATCMD_TIMEOUT (3*DELAY_1_S)
#define M26_RSP_MIN_LEN 4

#define M26_CONF_TIMEOUT (5*DELAY_1_S)   //5 seconds
#define M26_SIM_BUSY_TIMEOUT (10*DELAY_1_S)   // 10 seconds
#define M26_GSM_ATTACH_TIMEOUT (30*DELAY_1_S)   // 30 seconds
#define M26_GPRS_ATTACH_TIMEOUT (30*DELAY_1_S)   // 30 seconds
#define M26_SOCK_OPEN_TIMEOUT (10*DELAY_1_S) // 10 seconds
#define M26_RECV_TIMEOUT (DELAY_1_S)    // 1 second
#define M26_SEND_TIMEOUT (DELAY_1_S)    // 1 second

#define M26_GSM_INFO_DEFAULT "UNKNOWN"

//config at command
#define M26_AT_CMD_SIM_STAT "AT+CPIN?"
#define M26_AT_CMD_CREG "AT+CREG?"
#define M26_AT_CMD_APN "AT+QIREGAPP="
#define M26_AT_CMD_GET_APN "AT+QIREGAPP?"
#define M26_AT_CMD_GPRS_ATT "AT+QIFGCNT=0"
#define M26_AT_CMD_GPRS_STAT "AT+CGATT?"
#define M26_AT_CMD_SOCK_STAT "AT+QISTATE"
#define M26_AT_CMD_SOCK_OPEN "AT+QIOPEN=\"TCP\","
#define M26_AT_CMD_RECV "AT+QIRD=0,1,0,542" //stay same with PB_OTA_RECV_BUFF_SIZE
#define M26_AT_CMD_CELL_LOC "AT+QCELLLOC=1"

//respond
#define M26_RSP_OK "OK"
#define M26_RSP_ERROR "ERROR"
#define M26_RSP_CREG_OK "+CREG: 0,1"
#define M26_RSP_CREG_ROAM "+CREG: 0,5"
#define M26_RSP_CSQ "+CSQ: "
#define M26_RSP_SIM_NOTINSERT "+CME ERROR: 10"
#define M26_RSP_SIM_BUSY "+CME ERROR: 14"
#define M26_RSP_SIM_RDY "+CPIN: READY"
#define M26_RSP_GPRS_ATTACH "+CGATT: 1"
#define M26_RSP_SOCK_OPENED "STATE: CONNECT OK"
#define M26_RSP_SOCK_CLOSE "STATE: IP CLOSE"
#define M26_RSP_RDYSEND ">"
#define M26_RSP_RDYRECV "+QIRD:"
#define M26_RSP_CELL_LOC "+QCELLLOC: "
#define M26_RSP_MODULE "Revision: "

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
static HAL_USART_TYPE *M26_COM = NULL;
static uint32 M26_COM_BAUDRATE = 115200;
static OS_MUTEX_TYPE M26_MUTEX = NULL;

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : m26_check_timeout
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : check is at command timeout
******************************************************************************/
static bool m26_check_timeout(uint32 start, uint32 timeout)
{
    if (os_get_tick_count() - start >= timeout)
    {
        return true;
    }
    return false;
}

/******************************************************************************
* Function    : m26_at_cmd
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : send at command and get respond
******************************************************************************/
static uint16 m26_at_cmd(const char *at, char *rsp, uint16 rspMaxLen)
{
    uint32 sendTime = 0;
    uint16 offset = 0;
    os_mutex_lock(&M26_MUTEX);

    if (at != NULL)
    {
        while (M26_COM->available())
        {
            M26_COM->read();
        }

        M26_COM->println((char*)at);
        sendTime = os_get_tick_count();
        OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "AT[%s]", at);
    }

wait:
    while (!M26_COM->available())
    {
        os_scheduler_delay(DELAY_50_MS);
        if (m26_check_timeout(sendTime, M26_ATCMD_TIMEOUT))
        {
            offset = 0;
            goto err;
        }
    }

    while (M26_COM->available() > 0 && (offset + 1) < rspMaxLen)
    {
        rsp[offset++] = M26_COM->read();
    }

    if (offset < M26_RSP_MIN_LEN)//filter
    {
        offset = 0;
        goto wait;
    }

err:
    os_mutex_unlock(&M26_MUTEX);

    return offset;
}

/******************************************************************************
* Function    : m26_at_cmd_check_rsp
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : send at command and check respond
******************************************************************************/
static bool m26_at_cmd_check_rsp(const char *at, const char *rsp)
{
    bool ret = false;
    char buff[M26_RSP_MAX_LEN + 1] = {0};

    if (0 != m26_at_cmd(at, buff, M26_RSP_MAX_LEN))
    {
        if (NULL != strstr((char*)buff, rsp))
        {
            ret = true;
        }
    }

    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "RSP[%s]", buff);

    return ret;    
}

/******************************************************************************
* Function    : m26_at_cmd_wait_rsp
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_at_cmd_wait_rsp(const char *at, const char *rsp, uint32 timeout)
{
    uint32 startTime = os_get_tick_count();

    while (false == m26_at_cmd_check_rsp(at, rsp))
    {
        if (m26_check_timeout(startTime, timeout))
        {
            return false;
        }
        os_scheduler_delay(DELAY_500_MS);
    }

    return true;
}

/******************************************************************************
* Function    : m26_get_ver
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_get_ver(char *pdata, uint16 len)
{
    char buff[32+1];
    char *pVer = NULL;
    uint16 buffLen = 0;

    m26_at_cmd("AT+GMR", buff, 32);
    if (NULL == (pVer = strstr(buff, M26_RSP_MODULE)))
    {
        return false;
    }

    pVer += strlen(M26_RSP_MODULE);
    buffLen = strlen(pVer);
    
    if (buffLen > 0 && buffLen <= len)
    {
        strncpy(pdata, pVer, buffLen);
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************
* Function    : m26_get_imei
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_get_imei(char *pdata, uint16 len)
{
    char buff[32+1];
    uint16 buffLen = 0;

    m26_at_cmd("AT+GSN", buff, 32);
    buffLen = strlen(buff);
    
    if (buffLen > 0 && buffLen <= len)
    {
        strncpy(pdata, buff, buffLen);
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************
* Function    : m26_get_imsi
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_get_imsi(char *pdata, uint16 len)
{
    char buff[32+1];
    uint16 buffLen = 0;

    m26_at_cmd("AT+CIMI", buff, 32);
    buffLen = strlen(buff);
    
    if (buffLen > 0 && buffLen <= len)
    {
        strncpy(pdata, buff, buffLen);
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************
* Function    : m26_get_iccid
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_get_iccid(char *pdata, uint16 len)
{
    char buff[32+1];
    uint16 buffLen = 0;

    m26_at_cmd("AT+QCCID", buff, 32);
    buffLen = strlen(buff);
    
    if (buffLen > 0 && buffLen <= len)
    {
        strncpy(pdata, buff, buffLen);
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************
* Function    : m26_gsm_info
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static void m26_gsm_info(char *ver, const uint8 verLen, char *imei, const uint8 imeiLen, char *imsi, const uint8 imsiLen, char *iccid, const uint8 iccidLen)
{
    char strTmp[32 + 1];

    //get GSM module version
    memset(strTmp, 0, sizeof(strTmp));
    if (!m26_get_ver(strTmp, verLen))
    {
        memcpy(ver, M26_GSM_INFO_DEFAULT, strlen(M26_GSM_INFO_DEFAULT));
    }
    else
    {
        memcpy(ver, strTmp, strlen(strTmp));
    }

    //get GSM module IMEI
    memset(strTmp, 0, sizeof(strTmp));
    if (!m26_get_imei(strTmp, imeiLen))
    {
        memcpy(imei, M26_GSM_INFO_DEFAULT, strlen(M26_GSM_INFO_DEFAULT));
    }
    else
    {
        memcpy(imei, strTmp, strlen(strTmp));
    }

    //get SIM IMSI
    memset(strTmp, 0, sizeof(strTmp));
    if (!m26_get_imsi(strTmp, imsiLen))
    {
        memcpy(imsi, M26_GSM_INFO_DEFAULT, strlen(M26_GSM_INFO_DEFAULT));
    }
    else
    {
        memcpy(imsi, strTmp, strlen(strTmp));
    }

    //get SIM ICCID
    memset(strTmp, 0, sizeof(strTmp));
    if (!m26_get_iccid(strTmp, iccidLen))
    {
        memcpy(iccid, M26_GSM_INFO_DEFAULT, strlen(M26_GSM_INFO_DEFAULT));
    }
    else
    {
        memcpy(iccid, strTmp, strlen(strTmp));
    }
}

/******************************************************************************
* Function    : m26_get_csq
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : update GPRS csq val
******************************************************************************/
static void m26_get_csq(uint8 *rssi, uint8 *ber)
{
    char rsp[M26_RSP_MAX_LEN+1];
    char *pBer = NULL;
    char *pRssi = NULL;
    uint8 len = 0;

    memset(rsp, 0, sizeof(rsp));
    m26_at_cmd("AT+CSQ", rsp, M26_RSP_MAX_LEN);

    if (NULL != (pBer = (strrchr(rsp, ','))))//find last ','
    {
        *ber = atoi(pBer+1);        

        if (NULL != (pRssi = (strstr(rsp, M26_RSP_CSQ))))
        {
            pRssi += strlen(M26_RSP_CSQ);
            len = pBer - pRssi;

            char tmp[3];
            if (len <= 2)
            {
                memcpy(tmp, pRssi, len);
                *rssi = atoi(tmp);
            }
            else
            {
                *rssi =0xFF;
            }
        }
        else
        {
            *rssi = 0xFF;
        }
    }
    else
    {
        *ber = 0xFF;
    }
}

/******************************************************************************
* Function    : m26_net_check_stat
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_net_check_stat(void)
{
    //check sim card state first 
    if (m26_at_cmd_check_rsp(M26_AT_CMD_SIM_STAT, M26_RSP_SIM_NOTINSERT))
    {
        OS_INFO("SIM CARD NOT INSERTED");
        return false;
    }

    char buff[M26_RSP_MAX_LEN+1] = {0};

    #if 0
    //discard cache data
    while (m26_com.available())
    {
        m26_com.read();
    }
    #endif

    m26_at_cmd(M26_AT_CMD_SOCK_STAT, buff, M26_RSP_MAX_LEN);
    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Netstat[%s]", buff);

    if (NULL != strstr(buff, M26_RSP_SOCK_OPENED))
    {
        return true;
    }
    else
    {
        OS_DBG_ERR(DBG_MOD_DEV, "Net err %s", buff);
        return false;
    }
}

/******************************************************************************
* Function    : m26_wait_send_rdy
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : wait '>' to check send rdy
******************************************************************************/
static bool m26_wait_send_rdy(uint32 sendTime)
{
    int8 nread = 0;
    uint8 total_read = 0;
    uint8 respond[M26_RSP_MAX_LEN + 1];

    do
    {
        if (m26_check_timeout(sendTime, M26_SEND_TIMEOUT))
        {
            OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Send timeout");
            return false;
        }

        memset(respond, '\0', sizeof(respond));
        nread = M26_COM->readBytes(respond, M26_RSP_MAX_LEN);

        if (nread <= 0)
        {
            os_scheduler_delay(DELAY_1_MS*10);//wait for respond
            continue;
        }
        OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "%s", respond);

        total_read += nread;

        if (NULL != strstr((char*)respond, M26_RSP_RDYSEND))
        {
            return true;
        }
        if (NULL != strstr((char*)respond, M26_RSP_ERROR))
        {
            return false;
        }

    }while (total_read < 64);

    return false;
}

/******************************************************************************
* Function    : m26_check_send_ok
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static uint16 m26_check_send_ok(uint32 sendTime)
{
    char buff[M26_RSP_MAX_LEN + 1] = {0};
    uint16 offset = 0;
    bool ret = false;

wait:
    while (!M26_COM->available())
    {
        os_scheduler_delay(DELAY_50_MS);
        if (m26_check_timeout(sendTime, M26_SEND_TIMEOUT))
        {
            offset = 0;
            goto err;
        }
    }

    while (M26_COM->available() > 0 && (offset + 1) < M26_RSP_MAX_LEN)
    {
        buff[offset++] = M26_COM->read();
    }

    if (offset < M26_RSP_MIN_LEN)//filter
    {
        offset = 0;
        goto wait;
    }
    if (NULL != strstr((char*)buff, "SEND OK"))
    {
        ret = true;
    }

err:
    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "RSP[%s]", buff);
    return ret;
}

/******************************************************************************
* Function    : m26_net_send
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static uint16 m26_net_send(const uint8 *pdata, const uint16 size)
{
    uint32 sendTime = 0;
    uint16 realSend = 0;
    char sendCmd[32];
    snprintf(sendCmd, 32, "AT+QISEND=%d", size);

    os_mutex_lock(&M26_MUTEX);

    while (M26_COM->available())
    {
        M26_COM->read();
    }

    M26_COM->println(sendCmd);
    sendTime = os_get_tick_count();
    
    if (m26_wait_send_rdy(sendTime))
    {
        M26_COM->writeBytes((uint8*)pdata, size);

        if (m26_check_send_ok(sendTime))
        {
            realSend = size;
        }
    }
    os_mutex_unlock(&M26_MUTEX);

    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Send %d", realSend);
    return realSend;
}

/******************************************************************************
* Function    : m26_wait_recv_data
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : get recv data length
******************************************************************************/
static uint16 m26_wait_recv_data(uint32 sendTime)
{
    uint8 total_read = 0;
    uint8 readByte;
    uint8 respond[M26_RSP_MAX_LEN + 1];
    char *pFind = NULL;
    
    memset(respond, 0, sizeof(respond));
    do
    {
        if (m26_check_timeout(sendTime, M26_RECV_TIMEOUT))
        {
            OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Recv timeout");
            return 0;
        }

        if (M26_COM->available() == 0)
        {
            os_scheduler_delay(DELAY_1_MS*10);//wait for respond
            continue;
        }

        readByte = M26_COM->read();
        if ((total_read < strlen(M26_RSP_RDYRECV))
            && (readByte == '\r' || readByte == '\n'))
        {
            continue;
        }
        
        respond[total_read++] = readByte;
        if (NULL != strstr ((char*)respond, "+QIRDI: "))
        {
            memset(respond, 0, sizeof(respond));
            total_read = 0;
            continue;
        }

        if ((NULL != strstr((char*)respond, M26_RSP_RDYRECV))
            && (NULL != (pFind = strstr((char*)respond, "TCP,")))
            && (NULL != strstr((char*)respond, "\n")))
        {
            OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "--------TCP\r\n%s--------\r\n", respond);
            uint8 findOffset = 0;
            char recvLen[4+1] = {0};

            pFind += 4;//skip "TCP,"
            while ((findOffset < 4) && (*pFind != '\n'))
            {
                recvLen[findOffset] = *pFind++;
                findOffset++;
            }
            recvLen[findOffset] = '\0';

            return atoi(recvLen);
        }
        if (NULL != strstr((char*)respond, M26_RSP_ERROR))
        {
            return 0;
        }

    }while (total_read < 128);

    return 0;
}

/******************************************************************************
* Function    : m26_net_recv
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : recv data
******************************************************************************/
static uint16 m26_net_recv(uint8 *pdata, const uint16 max)
{
    uint32 sendTime = 0;
    uint16 recvLen;
    uint16 readCnt = 0;

    os_mutex_lock(&M26_MUTEX);

    M26_COM->println(M26_AT_CMD_RECV);
    sendTime = os_get_tick_count();
    
    recvLen = m26_wait_recv_data(sendTime);
    recvLen = MIN_VALUE(recvLen, max);

    if (recvLen == 0)
    {
        while (M26_COM->available())
        {
            M26_COM->read();
        }
        goto err;
    }   

    while (M26_COM->available() != 0 && readCnt < recvLen)
    {
        pdata[readCnt] = M26_COM->read();
        readCnt++;
    }

    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Recv[%d]", recvLen);
err:
    os_mutex_unlock(&M26_MUTEX);

    return readCnt;
}

/******************************************************************************
* Function    : m26_net_disconnect
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : close socket and de-attach pdp and gprs prepare to reset m26
******************************************************************************/
static bool m26_net_disconnect(void)
{
    //Close socket
    m26_at_cmd_check_rsp("AT+QICLOSE", "CLOSE OK");
    
    //pdp deattach
    m26_at_cmd_check_rsp("AT+CGACT=0,1", "NO CARRIER");

    //GPRS de-act
    m26_at_cmd_check_rsp("AT+CGATT=0", M26_RSP_OK);

    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Net disconnected");

    return true;
}

/******************************************************************************
* Function    : m26_net_connect
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : connet to server
******************************************************************************/
static bool m26_net_connect(const char *domain, const uint16 port)
{
    bool ret = false;
    char cmd[M26_AT_CMD_MAX_LEN+1] = {0};

    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Net connecting[%s:%d]", domain, port);

    snprintf(cmd, M26_AT_CMD_MAX_LEN, M26_AT_CMD_SOCK_OPEN"\"%s\",\"%d\"", domain, port);
    
    //open socket and connect to server
    if (!m26_at_cmd_wait_rsp(cmd, M26_RSP_OK, M26_SOCK_OPEN_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "Sock open err");
        goto err;
    }

    //check connect state
    if (!m26_at_cmd_wait_rsp(M26_AT_CMD_SOCK_STAT, M26_RSP_SOCK_OPENED, M26_SOCK_OPEN_TIMEOUT))
    {
        if (m26_at_cmd_check_rsp(M26_AT_CMD_SOCK_STAT, M26_RSP_SOCK_CLOSE))
        {
            OS_DBG_ERR(DBG_MOD_DEV, "Sock closed");
            goto err;
        }

        OS_DBG_ERR(DBG_MOD_DEV, "Sock not open");
        goto err;
    }    

    ret = true;
    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "Net connected");
err:
    return ret;
}

/******************************************************************************
* Function    : m26_check_creg
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_check_creg(uint32 timeout)
{
    uint32 startTime = os_get_tick_count();
    char buff[M26_RSP_MAX_LEN+1] = {0};

    while (1)
    {
        if (m26_check_timeout(startTime, timeout))
        {
            return false;
        }
        os_scheduler_delay(DELAY_500_MS);

        if (0 != m26_at_cmd(M26_AT_CMD_CREG, buff, M26_RSP_MAX_LEN))
        {
            OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "RSP[%s]", buff);
            if (NULL != strstr(buff, M26_RSP_CREG_OK) 
                || NULL != strstr(buff, M26_RSP_CREG_ROAM))
            {
                return true;
            }
        }
    }
}

/******************************************************************************
* Function    : m26_set_apn
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_set_apn(const char *apn, const char *user, const char *pass)
{
    char cmd[M26_AT_CMD_MAX_LEN] = {0};

    snprintf(cmd, M26_AT_CMD_MAX_LEN, M26_AT_CMD_APN"\"%s\",\"%s\",\"%s\"", apn, user, pass);
    
    return m26_at_cmd_check_rsp(cmd, M26_RSP_OK);
}

/******************************************************************************
* Function    : m26_check_gprs
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_check_gprs(uint32 timeout)
{
    uint32 startTime = os_get_tick_count();

    while (false == m26_at_cmd_check_rsp(M26_AT_CMD_GPRS_STAT, M26_RSP_GPRS_ATTACH))
    {
        //debug info
        m26_at_cmd_check_rsp("AT+CPIN?", M26_RSP_OK);
        m26_at_cmd_check_rsp("AT+COPS?", M26_RSP_OK);
        m26_at_cmd_check_rsp("AT+CREG?", M26_RSP_OK);
        m26_at_cmd_check_rsp("AT+CGREG?", M26_RSP_OK);
        m26_at_cmd_check_rsp("AT+CSQ", M26_RSP_OK);
        m26_at_cmd_check_rsp("AT+QISTATE", M26_RSP_OK);
        
        if (m26_check_timeout(startTime, timeout))
        {
            return false;
        }
        os_scheduler_delay(DELAY_500_MS);
    }

    return true;
}

/******************************************************************************
* Function    : m26_config
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
static bool m26_config(const char *apn, const char *user, const char *pass)
{
    bool ret = false;

    m26_at_cmd_check_rsp("ATE0", M26_RSP_OK);
    m26_at_cmd_check_rsp("AT+CFUN=1", M26_RSP_OK);

    //SIM card stat check
    if (m26_at_cmd_check_rsp(M26_AT_CMD_SIM_STAT, M26_RSP_SIM_NOTINSERT))
    {
        OS_INFO("SIM CARD NOT INSERTED");
        goto err;
    }

    //SIM card stat check
    if (!m26_at_cmd_wait_rsp(M26_AT_CMD_SIM_STAT, M26_RSP_SIM_RDY, M26_SIM_BUSY_TIMEOUT))
    {
        m26_at_cmd_wait_rsp(M26_AT_CMD_SIM_STAT, M26_RSP_SIM_BUSY, M26_SIM_BUSY_TIMEOUT);

        if (!m26_at_cmd_check_rsp(M26_AT_CMD_SIM_STAT, M26_RSP_SIM_RDY))
        {
            OS_DBG_ERR(DBG_MOD_DEV, "SIM BUSY");
            goto err;
        }
    }

    //check GSM network register
    if (!m26_check_creg(M26_GSM_ATTACH_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "GSM TIMEOUT");
        goto err;
    }

    //set APN when it's not null
    if (apn != NULL)
    {
        if (!m26_set_apn(apn, user, pass))
        {
            OS_DBG_ERR(DBG_MOD_DEV, "APN Err");
            goto err;
        }
        m26_at_cmd_check_rsp(M26_AT_CMD_GET_APN, M26_RSP_OK);
    }

    //show ip addr
    //m26_at_cmd_wait_rsp("AT+QILOCIP=0", PB_DEV_AT_RSP_OK);

    //set active conetxt
    if (!m26_at_cmd_wait_rsp(M26_AT_CMD_GPRS_ATT, M26_RSP_OK, M26_CONF_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "GPRS ATTACH Err");
        goto err;
    }
    //GPRS attach, this AT command is not used
    //m26_at_cmd_wait_rsp("AT+CGATT=1", PB_DEV_AT_RSP_OK);

    //check gprs status
    if (!m26_check_gprs(M26_GPRS_ATTACH_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "GPRS status err");
        goto err;
    }

    //cache recv data
    if (!m26_at_cmd_wait_rsp("AT+QINDI=1", M26_RSP_OK, M26_CONF_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "QINDI err");
        goto err;
    }

    //SEND showback off
    if (!m26_at_cmd_wait_rsp("AT+QISDE=0", M26_RSP_OK, M26_CONF_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "QISDE err");
        goto err;
    }

    //tcp connect by domain name
    if (!m26_at_cmd_wait_rsp("AT+QIDNSIP=1", M26_RSP_OK, M26_CONF_TIMEOUT))
    {
        OS_DBG_ERR(DBG_MOD_DEV, "QIDNSIP err");
        goto err;
    }

    ret = true;
    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "M26 config OK");
err:

    return ret;
}

/******************************************************************************
* Function    : m26_hw_reset
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : m26 module re-open and usart re-init
******************************************************************************/
static bool m26_hw_reset(void)
{
    //m26 power off and on
    hal_gpio_set(BOARD_M26_PWR, HAL_GPIO_LOW);
    os_scheduler_delay(DELAY_1_S);
    hal_gpio_set(BOARD_M26_PWR, HAL_GPIO_HIGH);
    
    //m26 powerkey reset
    hal_gpio_set(BOARD_M26_PWRKEY, HAL_GPIO_HIGH);
    os_scheduler_delay(DELAY_100_MS);
    hal_gpio_set(BOARD_M26_PWRKEY, HAL_GPIO_LOW);
    os_scheduler_delay(DELAY_500_MS);
    hal_gpio_set(BOARD_M26_PWRKEY, HAL_GPIO_HIGH);

    M26_COM->end();
    M26_COM->begin(M26_COM_BAUDRATE);
    //wait com and m26 stable
    os_scheduler_delay(DELAY_1_S*5);
    
    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "M26 RST OK");
    return true;
}

/******************************************************************************
* Function    : m26_hw_init
* 
* Author      : Chen Hao
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : set up com usart and baudrate, and gpio init
******************************************************************************/
static bool m26_hw_init(HAL_USART_TYPE *com, const uint32 baudrate)
{
    if (com == NULL)
    {
        OS_DBG_ERR(DBG_MOD_DEV, "M26 com is invalid");
        return false;
    }
    M26_COM = com;
    M26_COM_BAUDRATE = baudrate;
    os_mutex_lock_init(&M26_MUTEX);

    hal_rcc_enable(BOARD_M26_IO_RCC);

    hal_gpio_set_mode(BOARD_M26_PWR, GPIO_Mode_Out_PP);
    hal_gpio_set_mode(BOARD_M26_PWRKEY, GPIO_Mode_Out_PP);

    OS_DBG_TRACE(DBG_MOD_DEV, DBG_INFO, "M26 INIT OK, BAUD[%d]", baudrate);
    return true;
}

const DEV_TYPE_M26 devM26 = 
{
    m26_hw_init,
    m26_hw_reset,
    m26_config,
    m26_net_connect,
    m26_net_disconnect,
    m26_net_recv,
    m26_net_send,
    m26_net_check_stat,
    m26_get_csq,
    m26_gsm_info,
    NULL
};

