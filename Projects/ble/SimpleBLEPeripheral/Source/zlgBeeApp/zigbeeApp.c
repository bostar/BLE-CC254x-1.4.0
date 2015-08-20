#include "bcomdef.h"
#include "OSAL.h"
#include "hal_uart.h"
#include "hal_sensor.h"
#include "npi.h"
#include "OnBoard.h"
#include "hal_key.h"
#include <math.h>
#include <stdlib.h>
#include "hal_adc.h"

#if defined _USE_ZM516X_

#if defined ( STARBO_BOARD )
    #include "simplekeys.h"
#endif

#include "zigbeeApp.h"
#include "hal_zlg.h"
#include "zlgAtCmd.h"
#include "zlg_protocol.h"
#include "zlg_bsp.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 zigbee_TaskID;   // Task ID for internal task/event processing
static uint8 rbuf[255];
static uint8 idx = 0;
parkingState_t * parkingState;
static sleepOrwake_t zlgSleepOrwake;

static void npiCBack_uart( uint8 port, uint8 events );
static unsigned char referenceCmdLength( unsigned char * const command,unsigned char cmd );
static void eventReportToGateway( parkingEvent_t event );

#if defined ( STARBO_BOARD )
static void zigBee_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void zigBee_HandleKeys( uint8 shift, uint8 keys );
#endif

int16 vbt = 0,sen = 0,avdd = 0;
float sen_v = 0,vbt_v = 0,avdd_v = 0;
  
void Zigbee_Init( uint8 task_id )
{
  zigbee_TaskID = task_id;

  NPI_InitTransport(npiCBack_uart);
  InitUart1();
  HalAdcSetReference ( HAL_ADC_REF_AVDD );
  sen = HalAdcRead (HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_8);
  HAL_GPIO_CHANGE_DELAY();
//  for(int loop = 0;loop < 1000;loop ++)
  vbt = HalAdcRead (HAL_ADC_CHANNEL_1, HAL_ADC_RESOLUTION_8);
  HAL_GPIO_CHANGE_DELAY();
  sen_v = 3.482 * (float)sen / 0x7f;
  vbt_v = 3.482 * (float)vbt / 0x7f;
  vbt_v = vbt_v;
//  sen_v = sen_v;

#if defined( STARBO_BOARD )

  //SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zigbee_TaskID );
  
#endif // !STARBO_BOARD
  // Setup a delayed profile startup
//  osal_set_event( zigbee_TaskID, ZIGBEE_START_DEVICE_EVT );
  osal_start_timerEx(zigbee_TaskID, ZIGBEE_START_DEVICE_EVT ,1000);
}


uint16 Zigbee_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;
  static unsigned char test_state = 0;
  static unsigned char justOnPower = 0;
  if ( events & ZIGBEE_START_DEVICE_EVT )
  {
    setMotorStop();
    
    //init global variable
    stDevInfo = osal_mem_alloc( sizeof( dev_info_t ) );
    osal_memset( stDevInfo,0x00,sizeof( dev_info_t ) );
    uartReturnFlag = osal_mem_alloc( sizeof( uartReturnStatus_t ) );
    osal_memset( uartReturnFlag,0x00,sizeof( uartReturnStatus_t ) );
    eventReportData = osal_mem_alloc( sizeof( eventReport_t ) );
    osal_memset( eventReportData,0x00,sizeof( eventReport_t ) );
    parkingState = osal_mem_alloc( sizeof( parkingState_t ) );
    parkingState->vehicleState = cmdVehicleLeave;
    parkingState->lockState = 0xff;
    
    zlgSleepOrwake = wakeState;
    justOnPower = 1;   
    Uart1_Send_Byte("get",osal_strlen("get"));
    osal_set_event( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT );   
    return ( events ^ ZIGBEE_START_DEVICE_EVT );
  }
  
  if(events & ZIGBEE_RESET_ZM516X_EVT)
  {     
     SET_ZM516X_RESET();
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT, 10 );    
     return ( events ^ ZIGBEE_RESET_ZM516X_EVT );
  }
  
  if(events & ZIGBEE_READ_ZM516X_INFO_EVT)
  {
     if(!uartReturnFlag->readLocalCfg_SUCCESS)
     {
        read_local_cfg();
//        HAL_GPIO_CHANGE_DELAY();
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT ,1000 );
        return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
     }
     uartReturnFlag->readLocalCfg_SUCCESS = 0;
     osal_stop_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT);
     if(justOnPower)
     {
      if((stDevInfo->devDestNetAddr[0] != 0x00) || (stDevInfo->devDestNetAddr[1] != 0x00)\
       || (stDevInfo->devChannel != 15) || (stDevInfo->devLoacalNetAddr[0] != 0xFF) || \
         (stDevInfo->devLoacalNetAddr[1] != 0xFE) || (stDevInfo->devPanid[0] != 0x00) || \
           (stDevInfo->devPanid[1] != 0x00) )
      {
          osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT,10);
          return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
      }
      else
        justOnPower = 0;
     }
     if(!justOnPower)
     {
       if(!uartReturnFlag->applyNetWork_SUCCESS)
       {
         osal_start_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT ,10 );
       }
       else
       {
         test_state = 0;
         uartReturnFlag->applyNetWork_SUCCESS = 0;
         osal_start_timerEx( zigbee_TaskID, BOARD_TEST_EVT ,100 );
         osal_start_timerEx( zigbee_TaskID, UART1_READ_HMC5983_EVT ,500 );
//         osal_set_event( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT );
         osal_start_timerEx( zigbee_TaskID, READ_ZIGBEE_ADC_EVT, 10);//add to test
//         setMotorForward();
       }
     }
     
     return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
  }
  
  if(events & ZIGBEE_APPLY_NETWORK_EVT)
  {
    if(!uartReturnFlag->applyNetWork_SUCCESS)
    {
       applicateForNetwork( &stDevInfo->devLoacalIEEEAddr[0] );
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT ,1000 );
       return ( events ^ ZIGBEE_APPLY_NETWORK_EVT );
    }
    else
    {
       osal_stop_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT);
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT, 10);
       return ( events ^ ZIGBEE_APPLY_NETWORK_EVT );
    }
  }
  
  if( events & UART1_READ_HMC5983_EVT )
  {
      Uart1_Send_Byte("get",osal_strlen("get"));
      if(!mag_xyz.checked)
      { 
          mag_xyz.checked = 1;
          
          if( (abs(old_mag_xyz.x - mag_xyz.x) > 100) || \
              (abs(old_mag_xyz.y - mag_xyz.y) > 100) || \
              (abs(old_mag_xyz.z - mag_xyz.z) > 100) )
          {
            if(parkingState->vehicleState == cmdVehicleLeave)
            {                
                parkingState->vehicleState = cmdVehicleComming;
                eventReportToGateway( cmdVehicleComming );
            }
          }
          else
          {
            if(parkingState->vehicleState == cmdVehicleComming)
            {
                parkingState->vehicleState = cmdVehicleLeave;
                eventReportToGateway( cmdVehicleLeave );
            }
          }             
      }
      osal_start_reload_timer( zigbee_TaskID, UART1_READ_HMC5983_EVT,1000);
      return ( events ^ UART1_READ_HMC5983_EVT );
  }

  if(events & ZIGBEE_WRITE_ZM516X_INFO_EVT)
  {
     if( !uartReturnFlag->writeLocalCfg_SUCCESS )
     {
        if(justOnPower)
        {
          stDevInfo->devDestNetAddr[0] = 0x00;
          stDevInfo->devDestNetAddr[1] = 0x00;
          stDevInfo->devChannel = 15;
          stDevInfo->devLoacalNetAddr[0] = 0xFF;
          stDevInfo->devLoacalNetAddr[1] = 0xFE;
          stDevInfo->devPanid[0] = 0x00;
          stDevInfo->devPanid[1] = 0x00;
        }
        write_local_cfg(localAddress, stDevInfo);
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT ,1000 );
        return ( events ^ ZIGBEE_WRITE_ZM516X_INFO_EVT );
     }
     uartReturnFlag->writeLocalCfg_SUCCESS = 0;
     osal_stop_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT);
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_RESET_ZM516X_EVT, 100 );
     return ( events ^ ZIGBEE_WRITE_ZM516X_INFO_EVT );
  }
  
  if(events & BOARD_TEST_EVT)
  {
     switch(test_state)
     {
     case 0:
       if(!uartReturnFlag->gpioDirectionSet_SUCCESS)
       {
          initZlgBsp();
          osal_start_timerEx( zigbee_TaskID, BOARD_TEST_EVT, 1000 );
          return ( events ^ BOARD_TEST_EVT );
       }
       else
       {
          osal_start_timerEx( zigbee_TaskID, BOARD_TEST_EVT,500 );
//          osal_stop_timerEx( zigbee_TaskID, BOARD_TEST_EVT );
       }
       test_state = 7;
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
       setLedBit(uartReturnFlag->ledBitState);
       break;
     case 4:
       setMotorForward();
       parkingState->lockState = cmdUnlocking;
       test_state = 6;       
       break;
     case 5:
       setMotorReverse();
       parkingState->lockState = cmdLocking;
       test_state = 6;
       break;
     case 6:
       setMotorStop();
       test_state = 8;
       break;
     case 7:
       lowZlgAllIOs();
       osal_set_event( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT );//
       break;
     default:
       break;
     }
     return ( events ^ BOARD_TEST_EVT );
  }
  
  if(events & ZIGBEE_LINK_TEST_EVT)
  {
    if(zlgSleepOrwake == sleepState)
    {
      osal_stop_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT);
      SET_ZM516X_WAKEUP();
      zlgSleepOrwake = wakeState;
      osal_start_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT ,1000 );
      ackLinkTest( &stDevInfo->devLoacalIEEEAddr[0] );
    }
    else
      ackLinkTest( &stDevInfo->devLoacalIEEEAddr[0] );
     return ( events ^ ZIGBEE_LINK_TEST_EVT );
  }
  
  if(events & READ_ZIGBEE_ADC_EVT)
  {
      sen = HalAdcRead (HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_8);
      HAL_GPIO_CHANGE_DELAY();
      sen_v = 3.482 * (float)sen / 0x7f;
//          motorStopAdcReport(uartReturnFlag->adc_value);   
      if(sen_v > 1.0)
      {
          if( parkingState->lockState == cmdLocking )  
          {
              parkingState->lockState = cmdLockFailed;
              setMotorForward();
              eventReportToGateway( cmdLockFailed );
          }
          else if( parkingState->lockState == cmdUnlocking )  
          {
              parkingState->lockState = cmdUnlockFailed;                 
              setMotorReverse();
              eventReportToGateway( cmdUnlockFailed );
          }
          else if(parkingState->lockState != cmdLockFailed && \
            parkingState->lockState != cmdUnlockFailed)
          {
            setMotorStop();
          }
      }     
      osal_start_reload_timer( zigbee_TaskID, READ_ZIGBEE_ADC_EVT,1000 );
      return ( events ^ READ_ZIGBEE_ADC_EVT );
  }
  
  if( events & EVENT_REPORT_EVT )
  {
      if(eventReportData->reportSuccess)
      {
          osal_stop_timerEx( zigbee_TaskID, EVENT_REPORT_EVT);
          return ( events ^ EVENT_REPORT_EVT );
      }
      if(zlgSleepOrwake == sleepState)
      {
          osal_stop_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT);//stop timer
          SET_ZM516X_WAKEUP();//and wake up right now
          zlgSleepOrwake = wakeState;
          osal_start_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT ,2000 );
      }     
      else
      {
        if(osal_get_timeoutEx( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT ) > 0)
        {
          osal_stop_timerEx( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT);
          osal_start_timerEx( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT ,1000 );
        }
      }
      eventReport( (parkingEvent_t)eventReportData->event );
      osal_start_timerEx( zigbee_TaskID, EVENT_REPORT_EVT, 2000 );
      return ( events ^ EVENT_REPORT_EVT );
  }
     
  if(events & ZIGBEE_RESTORE_FACTORY_EVT)
  {
     if(!uartReturnFlag->restoreSuccessFlag)
     {
        restore_factory_settings( localAddress );
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_RESTORE_FACTORY_EVT, 1000 );
        return ( events ^ ZIGBEE_RESTORE_FACTORY_EVT );
     }
     uartReturnFlag->restoreSuccessFlag = 0;
     osal_stop_timerEx( zigbee_TaskID, ZIGBEE_RESTORE_FACTORY_EVT);
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_START_DEVICE_EVT ,100 );
     return ( events ^ ZIGBEE_RESTORE_FACTORY_EVT );
  }
  
  if( events & ZIGBEE_SLEEP_ZM516X_EVT )
  {
     SET_ZM516X_SLEEP();
     zlgSleepOrwake = sleepState;
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT ,1850 );
     return ( events ^ ZIGBEE_SLEEP_ZM516X_EVT );
  }
  
  if( events & ZIGBEE_WAKE_ZM516X_EVT )
  { 
     if( zlgSleepOrwake != wakeState )
     {
        SET_ZM516X_WAKEUP();
        zlgSleepOrwake = wakeState;
     }
     dateRequset();
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT ,150 );
     return ( events ^ ZIGBEE_WAKE_ZM516X_EVT );
  }
  
  if( events & UART_RECEIVE_EVT )
  {
        unsigned char state_back;
 
        state_back = receive_data( rbuf, idx ); 
        osal_memset(rbuf,0,idx);
        idx = 0;
        switch( state_back )
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
            case stateAckLinkTest:
              osal_set_event( zigbee_TaskID, ZIGBEE_LINK_TEST_EVT );
              break;
            case stateRestoreFactoryConfig:
              osal_set_event( zigbee_TaskID, ZIGBEE_RESTORE_FACTORY_EVT );
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
            case stateMotorForward:
              if( parkingState->lockState == cmdUnlockSuccess )
              {
                eventReportToGateway( cmdUnlockSuccess );
                break;
              }
              test_state = 4;
              osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
              break;
            case stateMotorReverse:
              if( parkingState->lockState == cmdLockSuccess )
              {
                eventReportToGateway( cmdLockSuccess );
                break;
              }
              test_state = 5;
              osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
              break;
            default:
              break;
        }
    return ( events ^ UART_RECEIVE_EVT );
  }

#if defined ( STARBO_BOARD )
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( zigbee_TaskID )) != NULL )
    {
      zigBee_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
#endif
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
                || revPara.header[0] == 'T' || revPara.header[0] == 'S') );
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
          else if((revPara.header[0] == 'S') && (revPara.header[1] == 'E') && (revPara.header[2] == 'N'))
          {
            command_word = BASE_STATION_SEN;
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
          //to get heart beat pkg len
          if((revPara.header[0] == 'C') && (revPara.header[1] == 'F') && \
              (revPara.header[2] == 'G') && (revPara.cmd == cmdHeartBeatPkg))
          {
              pktState = NPI_SERIAL_PACK_HEARTBEAT_LEN;
              break;
          }
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
      case NPI_SERIAL_PACK_HEARTBEAT_LEN:
        {
          if (numBytes < 1)
          {
            // not enough data to progress, so leave it in driver buffer
            done = TRUE;
            break;
          }
          NPI_ReadTransport(&revPara.len,1);
          numBytes -= 1;
          rbuf[idx++] = revPara.len;
          revPara.len = revPara.len*2;
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
        numBytes -= revPara.len;
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
       cmd_len = 14;
       break;
/*     case cmdAckChangeNodeType:
       cmd_len = 9;
       break;
     case cmdChangePanidChannel:
       cmd_len = 3;     
       break;*/
     case cmdLinkTest:
       cmd_len = 8;
       break;
     case cmdRestoreFactoryConfig:
       cmd_len = 2;
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
       cmd_len = 3;     
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
       cmd_len = 3;
       break;
     default:
       break;
     }
   }
   if(*command == 'S' && *(command+1) == 'E' && *(command+2) == 'N')
   {
     switch(cmd)
     {
     case 0x04:
       cmd_len = 3;
       break;
     default:
       break;
     }
   }
   return cmd_len;
}

#if defined ( STARBO_BOARD )
static void zigBee_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    case KEY_CHANGE:
      zigBee_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;

  default:
    break;
  }
}

static void zigBee_HandleKeys( uint8 shift, uint8 keys )
{

  VOID shift;  // Intentionally unreferenced parameter
//unlock
  if ( keys == 0x06 )
  {
    setMotorStop();
    if( parkingState->lockState == cmdUnlocking )
    {
        eventReportToGateway( cmdUnlockSuccess );
    }
    parkingState->lockState = cmdUnlockSuccess;
  }
//lock
  else if ( keys == 0 )
  { 
    setMotorStop();    
    if( parkingState->lockState == cmdLocking )
    {
        eventReportToGateway( cmdLockSuccess );
    }
    parkingState->lockState = cmdLockSuccess;
  }
//over limit
  else if ( keys == 0x01 )
  {  
    setMotorForward();
  }
  else
  {
    if( parkingState->lockState != cmdLocking && parkingState->lockState != cmdUnlocking )
    {
      if(parkingState->lockState == cmdLockSuccess)
      {
        setMotorReverse();
        
      }
      else if(parkingState->lockState == cmdUnlockSuccess)
      {
        setMotorForward();
      }
    }
  }
  // Set the value of the keys state to the Simple Keys Profile;
  // This will send out a notification of the keys state if enabled
  //SK_SetParameter( SK_KEY_ATTR, sizeof ( uint8 ), &SK_Keys );
}
#endif // !STARBO_BOARD

static void eventReportToGateway( parkingEvent_t event )
{
    eventReportData->event =  event ;
    eventReportData->reportSuccess = 0;
    if( osal_get_timeoutEx( zigbee_TaskID, EVENT_REPORT_EVT ) > 0)
      osal_stop_timerEx( zigbee_TaskID, EVENT_REPORT_EVT );
    osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT);
}

#endif
