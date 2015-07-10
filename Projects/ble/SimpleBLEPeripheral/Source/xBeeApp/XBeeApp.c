#include "OSAL.h"
#include "hal_uart.h"
#include "hal_sensor.h"
#include "npi.h"
#include "OnBoard.h"

#if defined _USE_XBEE_

#include "zigbeeApp.h"
#include "hal_zlg.h"
#include "zlgAtCmd.h"
#include "zlg_protocol.h"
#include "zlg_bsp.h"

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
static uint8 zigbee_TaskID;   // Task ID for internal task/event processing
static uint8 rbuf[255];
static uint8 idx = 0;
   
static void npiCBack_uart( uint8 port, uint8 events );
static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd);

void Zigbee_Init( uint8 task_id )
{
  zigbee_TaskID = task_id;

  NPI_InitTransport(npiCBack_uart);
  // Setup a delayed profile startup
  osal_set_event( zigbee_TaskID, ZIGBEE_START_DEVICE_EVT );
  //HalSensorReadReg(0x0a,NULL,3);
}


uint16 Zigbee_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;
  static unsigned char test_state = 0;
  
  if ( events & ZIGBEE_START_DEVICE_EVT )
  {
    osal_set_event( zigbee_TaskID, ZIGBEE_RESET_ZM516X_EVT );

    return ( events ^ ZIGBEE_START_DEVICE_EVT );
  }
  

  if(events & UART_RECEIVE_EVT)
  {

    return ( events ^ UART_RECEIVE_EVT );
  }
  // Discard unknown events
  return 0;
}

static void npiCBack_uart( uint8 port, uint8 events )
{

  return;
}

#endif