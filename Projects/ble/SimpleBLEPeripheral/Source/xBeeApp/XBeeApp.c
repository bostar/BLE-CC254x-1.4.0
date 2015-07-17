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
static uint8 XBeeTaskID;   // Task ID for internal task/event processing
/*static uint8 rbuf[255];
static uint8 idx = 0;*/
   

void XBeeInit( uint8 task_id )
{
  XBeeTaskID = task_id;
  NPI_InitTransport(npiCBack_uart);             //初始化UART 
  osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //触发事件
}


uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;
  if ( events & XBEE_START_DEVICE_EVT )
  {
    osal_set_event( XBeeTaskID, XBEE_IO_TEST );
   
    return ( events ^ XBEE_START_DEVICE_EVT );
  }
  if( events & XBEE_IO_TEST )
  {
    //xbee_api_atcmd_set_led(XBEE_LED1,LED_ON);
    XBeeOpenBuzzer;
    osal_start_timerEx( XBeeTaskID, XBEE_CLOSE_LED, 1000 );
    return ( events ^ XBEE_IO_TEST );
  }
  if( events & XBEE_CLOSE_LED )
  {   
    //xbee_api_atcmd_set_led(XBEE_LED1,LED_OFF);
    XBeeCloseBuzzer;
    return ( events ^ XBEE_CLOSE_LED );
  }

  return 0;
}
static void npiCBack_uart( uint8 port, uint8 events )
{
  /*uint8 i,j;
  i=port;
  j=events;*/
}

/*static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd)
{
  uint8 i,j;
  i=*command;
  j=cmd;
   return 0;
}*/





#endif