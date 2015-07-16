#include "OSAL.h"
#include "hal_uart.h"
#include "hal_sensor.h"
#include "npi.h"
#include "OnBoard.h"

#if defined _USE_ZM516X_

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
  
  if(events & ZIGBEE_RESET_ZM516X_EVT)
  {
     static unsigned char reset_state = stateInit;
     
     switch( reset_state )
     {
     case stateInit:
       HalGpioSet( HAL_GPIO_ZM516X_RESET, 0 );
       reset_state = stateStart;
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_RESET_ZM516X_EVT, 100 );
       break;
     case stateStart:
       HalGpioSet( HAL_GPIO_ZM516X_RESET, 1 );
       reset_state = stateInit;
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT, 100 );
       break;
     default:
       break;
     }
     
     return ( events ^ ZIGBEE_RESET_ZM516X_EVT );
  }
  
  if(events & ZIGBEE_READ_ZM516X_INFO_EVT)
  {
     if(!uartReturnFlag.readLocalCfg_SUCCESS)
     {
        read_local_cfg();
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT ,1000 );
        return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
     }
     uartReturnFlag.readLocalCfg_SUCCESS = 0;
     osal_stop_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT);
     if((stDevInfo->devDestNetAddr[0] != 0x00) || (stDevInfo->devDestNetAddr[1] != 0x00))
     {
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT,10);
       return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
     }
     else
     {
       if(!uartReturnFlag.applyNetWork_SUCCESS)
       {
         osal_start_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT ,10);
         return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
       }
       else
       {
         test_state = 0;
         osal_start_timerEx( zigbee_TaskID, BOARD_TEST_EVT ,10 );
//         return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
       }
     }
     
     return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
  }
  
  
  if(events & ZIGBEE_APPLY_NETWORK_EVT)
  {
    if(!uartReturnFlag.applyNetWork_SUCCESS)
    {
       applicateForNetwork( &stDevInfo->devLoacalIEEEAddr[0] );
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT ,1000 );
       return ( events ^ ZIGBEE_APPLY_NETWORK_EVT );
    }
    else
    {
  //     uartReturnFlag.applyNetWork_SUCCESS = 0;
       osal_stop_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT);
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT, 10);
       return ( events ^ ZIGBEE_APPLY_NETWORK_EVT );
    }
  }

  if(events & ZIGBEE_WRITE_ZM516X_INFO_EVT)
  {
     if( !uartReturnFlag.writeLocalCfg_SUCCESS )
     {
        stDevInfo->devDestNetAddr[0] = 0x00;
        stDevInfo->devDestNetAddr[1] = 0x00;
        write_local_cfg(localAddress, stDevInfo);
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT ,1000 );
        return ( events ^ ZIGBEE_WRITE_ZM516X_INFO_EVT );
     }
     uartReturnFlag.writeLocalCfg_SUCCESS = 0;
     osal_stop_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT);
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_RESET_ZM516X_EVT, 100 );
     return ( events ^ ZIGBEE_WRITE_ZM516X_INFO_EVT );
  }
  
  if(events & BOARD_TEST_EVT)
  {
     switch(test_state)
     {
     case 0:
       if(!uartReturnFlag.gpioDirectionSet_SUCCESS)
       {
          initZlgBsp();
          osal_start_timerEx( zigbee_TaskID, BOARD_TEST_EVT, 1000 );
          return ( events ^ BOARD_TEST_EVT );
       }
       else
       {
          osal_stop_timerEx( zigbee_TaskID, BOARD_TEST_EVT );
       }
//       test_state = 1;
       break;
     case 1:
       setBeepOn();
//       test_state = 2;
       break;
     case 2:
       setBeepOff();
//       test_state = 1;
       break;
     case 3:
       setLedBit(uartReturnFlag.ledBitState);
       break;
     default:
       break;
     }
     return ( events ^ BOARD_TEST_EVT );
  }
  
  if(events & UART_RECEIVE_EVT)
  {
//    uint16 numBytes;
//    static unsigned char numBytes_old = 0;
    unsigned char state_back;
//    numBytes = NPI_RxBufLen();
//    if( numBytes > 0 )
//    {
//      if(numBytes_old == numBytes)
//      {
//        
        state_back = receive_data( rbuf, idx+1 ); 
        osal_memset(rbuf,0,idx++);
        idx = 0;
        switch(state_back)
        {
        case stateReadCfg:
          osal_set_event( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT );
          break;
        case stateWriteCfg:
          osal_set_event( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT );
          break;
        case stateReset:
          osal_set_event( zigbee_TaskID, ZIGBEE_RESET_ZM516X_EVT );
          break;
        case stateApplyNetwork:
          osal_set_event( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT );
          break;
        case stateBeepOn:
          test_state = 1;
          osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
          break;
        case stateBeepOff:
          test_state = 2;
          osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
          break;
        case stateLedTest:
          test_state = 3;
          osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
          break;
        case stateGpioSet:
          test_state = 0;
          osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
          break;
        default:
          break;
        }
//        numBytes_old = 0;
//      }
//      else
//      {
//        numBytes_old = numBytes;
//        osal_stop_timerEx( zigbee_TaskID, UART_RECEIVE_EVT );
//        osal_start_timerEx( zigbee_TaskID, UART_RECEIVE_EVT, 3 );
//      }
//    }
    return ( events ^ UART_RECEIVE_EVT );
  }
  // Discard unknown events
  return 0;
}

static void npiCBack_uart( uint8 port, uint8 events )
{
  static npi_serial_parse_state_t pktState = NPI_SERIAL_PACK_HEAD;
  static ptk_t revPara;
  static command_word_t command_word = ZLG_FLASH_SET;
  uint8  done = FALSE;
  uint16 numBytes;
  if (events & HAL_UART_RX_TIMEOUT)
  {
//    osal_set_event( zigbee_TaskID, UART_RECEIVE_EVT );
   
    numBytes = NPI_RxBufLen();
      
    while((numBytes > 0) && (!done))
    {   
      switch( pktState )
      {
      case NPI_SERIAL_PACK_HEAD:
        {         
          if (numBytes < 3)
          {
            // not enough data to progress, so leave it in driver buffer
            done = TRUE;
            break;
          }
          do
          {
              if( idx != 0 )
              {
                // not enough data to progress, so leave it in driver buffer
                done = TRUE;
                break;
              }
              NPI_ReadTransport( &revPara.header[0], 1 );             
              numBytes -= 1;
              if(!numBytes)
                  return;
          }while( !(revPara.header[0] == 0xab || revPara.header[0] == 0xde || revPara.header[0] == 'C' \
                || revPara.header[0] == 'T') );
          NPI_ReadTransport( &revPara.header[1], 2 );
          numBytes -= 2;
          rbuf[idx++] = revPara.header[0];
          rbuf[idx++] = revPara.header[1];
          rbuf[idx++] = revPara.header[2];

          if((revPara.header[0] == 0xab) && (revPara.header[1] == 0xbc) && (revPara.header[2] == 0xcd))
          {
            command_word = ZLG_FLASH_SET;
          }
          else if((revPara.header[0] == 0xde) && (revPara.header[1] == 0xdf) && (revPara.header[2] == 0xef))
          {
            command_word = ZLG_RAM_SET;
          }
          else if((revPara.header[0] == 'C') && (revPara.header[1] == 'F') && (revPara.header[2] == 'G'))
          {
            command_word = BASE_STATION_CFG;
          }
          else if((revPara.header[0] == 'T') && (revPara.header[1] == 'S') && (revPara.header[2] == 'T'))
          {
            command_word = BASE_STATION_TST;
          }
          else if((revPara.header[0] == 'C') && (revPara.header[1] == 'T') && (revPara.header[2] == 'L'))
          {
            command_word = BASE_STATION_CTL;
          }
          else
            return;
          VOID command_word;
          pktState = NPI_SERIAL_PACK_CMD;
        }
        break;
      case NPI_SERIAL_PACK_CMD:
        {
          if (numBytes < 1)
          {
            // not enough data to progress, so leave it in driver buffer
            done = TRUE;
            break;
          }
          NPI_ReadTransport(&revPara.cmd,1);
          numBytes -= 1;
          rbuf[idx++] = revPara.cmd;
          revPara.len = referenceCmdLength(revPara.header,revPara.cmd);
          if(revPara.len == 0)
          {
            pktState = NPI_SERIAL_PACK_HEAD;
            done = TRUE;
            osal_set_event( zigbee_TaskID, UART_RECEIVE_EVT );
          }
          else
            pktState = NPI_SERIAL_PACK_DATA;
        }
        break;
      case NPI_SERIAL_PACK_DATA:
        // check if there is enough serial port data to finish reading the payload
        if ( numBytes < revPara.len )
        {
          // not enough data to progress, so leave it in driver buffer
          done = TRUE;
          break;
        }
        NPI_ReadTransport(&rbuf[idx],revPara.len);
        idx += revPara.len;
        pktState = NPI_SERIAL_PACK_HEAD;
        done = TRUE;
        osal_set_event( zigbee_TaskID, UART_RECEIVE_EVT );
        break;
      default:
        break;
      }
    }
  }
  return;
}

static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd)
{
   unsigned char cmd_len = 0;
   
   if(*command == 0xab && *(command+1) == 0xbc && *(command+2) == 0xcd)
   {
     switch(cmd)
     {
     case enReadLoacalCfg:
       cmd_len = 70;
       break;
     case enSetChannelNv:
       cmd_len = 1;
       break;
     case enSearchNode:
       cmd_len = 9;
       break;
     case enGetRemoteInfo:
       cmd_len = 70;
       break;
     case enModifyCfg:
       cmd_len = 3;
       break;
     case enResetCfg:
       cmd_len = 5;
       break;
     default:
       break;
     }
   }
   if(*command == 0xde && *(command+1) == 0xdf && *(command+2) == 0xef)
   {
     switch(cmd)
     {
     case enSetChannel:
     case enSetDestAddr:
     case enShowSrcAddr:
     case enSetUnicastOrBroadcast:
     case enReadNodeRssi:
       cmd_len = 1;
       break;
     case enSetIoDirection:
     case enReadIoStatus:
     case enSetIoStatus:
       cmd_len = 3;     
       break;
     case enReadAdcValue:
       cmd_len = 4;     
       break;
     default:
       break;
     }
   }
   if(*command == 'C' && *(command+1) == 'F' && *(command+2) == 'G')
   {
     switch(cmd)
     {
     case cmdAckCheckIn:
       cmd_len = 11;
       break;
     case cmdAckChangeNodeType:
       cmd_len = 9;
       break;
     case cmdChangePanidChannel:
       cmd_len = 3;     
       break;
     default:
       break;
     }
   }
   if(*command == 'T' && *(command+1) == 'S' && *(command+2) == 'T')
   {
     switch(cmd)
     {
     case cmdBeepTest:
     case cmdLedTest:
     case cmdMotorTest:
       cmd_len = 1;     
       break;
     default:
       break;
     }
   }
   if(*command == 'C' && *(command+1) == 'T' && *(command+2) == 'L')
   {
     switch(cmd)
     {
     case 0x00:
       cmd_len = 1;
       break;
     default:
       break;
     }
   }
   return cmd_len;
}

#endif
