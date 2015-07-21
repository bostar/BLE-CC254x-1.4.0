/*****************************************************************
**brief 
**
**version       
**data          
*****************************************************************/

#include "XBeeApp.h"
#include "hal_zlg.h"
#include "OSAL.h"
#include "XBeeAtCmd.h"
#include "hal_uart.h"
#include "hal_sensor.h"
#include "npi.h"
#include "XBeeProtocol.h"
#include "XBeeBsp.h"
#include "hal_types.h"
#include "OnBoard.h"
#include "xbee_api_opt.h"


#if defined _XBEE_APP_

/*
const uint8 temp[cmd][len] = {{0,12}{1,11}};

void check_pkg_head(uint8 * head,uint8 len)
{
  if(ch=='C')
  return TURE;
}
check_pkg_head("CFG",3);
const uint8 CFG_CMD_NONVOLATILE[3] = {0XAB,0XBC,0XCD};
*/
/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 XBeeTaskID;               // Task ID for internal task/event processing       
static XBeeUartRecDataDef XBeeUartRec; //串口接收缓存数据   

void XBeeInit( uint8 task_id )
{
  XBeeTaskID = task_id;
  NPI_InitTransport(npiCBack_uart);             //初始化UART 
  osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //触发事件
}


uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;    
  //static uint16 ToReBufLen=100,WrBufLen;
  //uint16 ReBufLen,WrBufLen;
  static uint8 i=0;
  if ( events & XBEE_START_DEVICE_EVT )
  {   
    osal_set_event( XBeeTaskID, XBEE_IO_TEST );   
    return ( events ^ XBEE_START_DEVICE_EVT );
  }
  if( events & XBEE_IO_TEST )
  {
    if(i==0){
      XBeeReadAI();
      i++;
    }
    osal_start_timerEx( XBeeTaskID, XBEE_IO_TEST, 100 );
    return ( events ^ XBEE_IO_TEST );
  }
  if( events & XBEE_REC_DATA_PROCESS_EVT )  //处理串口收到数据
  { 
    osal_start_timerEx( XBeeTaskID, XBEE_IO_TEST, 100 );  
    return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
  }
  
  return 0;
}
/**********************************************************
**brief 回调函数，实现对xbee发送到串口数据的读取
**********************************************************/
static void npiCBack_uart( uint8 port, uint8 events )
{
  static uint16 i;
  uint8 cnt,checksum=0;
  uint16 numBytes=0,RecLen=0;
  static uint16 APICmdLen=0;
  if(events & HAL_UART_RX_TIMEOUT)
  {
    numBytes = NPI_RxBufLen();
    if(numBytes==0)
      goto TheEnd; 
    if(XBeeUartRec.num != 0)
      goto TheEnd;
    RecLen = NPI_ReadTransport( XBeeUartRec.data, 1);
    XBeeUartRec.num += RecLen;
    if((numBytes -= RecLen)<=0)
      goto TheEnd; 
    if(*XBeeUartRec.data != 0x7E)
      goto TheEnd;
    RecLen = NPI_ReadTransport((XBeeUartRec.data+1), 2);
    XBeeUartRec.num += RecLen;
    if(RecLen<2 || (numBytes -= RecLen)<=0)
      goto TheEnd;
    i = (uint16)*(XBeeUartRec.data+1);
    i <<= 8;
    APICmdLen = i + (uint16)*(XBeeUartRec.data+2);
    RecLen = NPI_ReadTransport((XBeeUartRec.data+3), APICmdLen+1);
    XBeeUartRec.num += RecLen;
    if(RecLen<(APICmdLen+1))
      goto TheEnd;
    checksum = 0;
    for(cnt=3;cnt<APICmdLen+3;cnt++)
      checksum += *(XBeeUartRec.data + cnt);
    checksum = 0xFF - checksum;
    if(*(XBeeUartRec.data + APICmdLen+3) != checksum)
      goto TheEnd;
    osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT ); 
  }
TheEnd: return;
}

/*static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd)
{
  uint8 i,j;
  i=*command;
  j=cmd;
   return 0;
}*/





#endif