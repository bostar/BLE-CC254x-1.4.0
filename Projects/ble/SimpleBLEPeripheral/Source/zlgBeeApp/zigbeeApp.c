#include "bcomdef.h"
#include "OSAL.h"
#include "hal_uart.h"
#include "hal_sensor.h"
#include "npi.h"
#include "OnBoard.h"
#include "hal_key.h"
#include <math.h>
#include <stdlib.h>

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
uint8 zigbee_TaskID;   // Task ID for internal task/event processing
static uint8 rbuf[255];
static uint8 idx = 0;
parkingState_t parkingState;
static sleepOrwake_t zlgSleepOrwake;

static void npiCBack_uart( uint8 port, uint8 events );
static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd);

#if defined ( STARBO_BOARD )
static void zigBee_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void zigBee_HandleKeys( uint8 shift, uint8 keys );
#endif

void Zigbee_Init( uint8 task_id )
{
  zigbee_TaskID = task_id;

  NPI_InitTransport(npiCBack_uart);
  InitUart1();
#if defined( STARBO_BOARD )

  //SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zigbee_TaskID );
  
#endif // !STARBO_BOARD
  // Setup a delayed profile startup
//  osal_set_event( zigbee_TaskID, ZIGBEE_START_DEVICE_EVT );
  osal_start_timerEx(zigbee_TaskID, ZIGBEE_START_DEVICE_EVT ,1000);
  //HalSensorReadReg(0x0a,NULL,3);
}


uint16 Zigbee_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;
  static unsigned char test_state = 0;
  static unsigned char justOnPower = 0;
  if ( events & ZIGBEE_START_DEVICE_EVT )
  {
    setMotorStop();
    parkingState.vehicleState = cmdVehicleLeave;
    parkingState.lockState = 0xff;//cmdUnlockSuccess;
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
     if(!uartReturnFlag.readLocalCfg_SUCCESS)
     {
        read_local_cfg();
        HAL_GPIO_CHANGE_DELAY();
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_READ_ZM516X_INFO_EVT ,1000 );
        return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
     }
     uartReturnFlag.readLocalCfg_SUCCESS = 0;
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
       if(!uartReturnFlag.applyNetWork_SUCCESS)
       {
         osal_start_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT ,10);
         osal_start_timerEx( zigbee_TaskID, READ_ZIGBEE_ADC_EVT,20);//add to test
       }
       else
       {
         test_state = 0;
         uartReturnFlag.applyNetWork_SUCCESS = 0;
         osal_start_timerEx( zigbee_TaskID, BOARD_TEST_EVT ,10 );
         osal_start_timerEx( zigbee_TaskID, UART1_READ_HMC5983_EVT ,20 );
       }
     }
     
     return ( events ^ ZIGBEE_READ_ZM516X_INFO_EVT );
  }
  
  if( events & UART1_READ_HMC5983_EVT )
  {
      Uart1_Send_Byte("get",osal_strlen("get"));
      if(!mag_xyz.checked)
      { 
          mag_xyz.checked = 1;
          
          if( (abs(old_mag_xyz.x - mag_xyz.x) > 150) || (abs(old_mag_xyz.y - mag_xyz.y) > 150) || (abs(old_mag_xyz.z - mag_xyz.z) > 150))
          {
            if(parkingState.vehicleState == cmdVehicleLeave)
            {                
                parkingState.vehicleState = cmdVehicleComming;
                eventReportData.event =  cmdVehicleComming ;
                osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
            }
          }
          else
          {
            if(parkingState.vehicleState == cmdVehicleComming)
            {
                parkingState.vehicleState = cmdVehicleLeave;
                eventReportData.event =  cmdVehicleLeave ;
                osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
            }
          }             
      }
      osal_start_reload_timer( zigbee_TaskID, UART1_READ_HMC5983_EVT,1000);
      return ( events ^ UART1_READ_HMC5983_EVT );
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
       osal_stop_timerEx( zigbee_TaskID, ZIGBEE_APPLY_NETWORK_EVT);
       osal_start_timerEx( zigbee_TaskID, ZIGBEE_WRITE_ZM516X_INFO_EVT, 10);
       return ( events ^ ZIGBEE_APPLY_NETWORK_EVT );
    }
  }

  if(events & ZIGBEE_WRITE_ZM516X_INFO_EVT)
  {
     if( !uartReturnFlag.writeLocalCfg_SUCCESS )
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
     case 4:
       setMotorForward();
       parkingState.lockState = cmdUnlocking;
       test_state = 6;       
       break;
     case 5:
       setMotorReverse();
       parkingState.lockState = cmdLocking;
       test_state = 6;
       break;
     case 6:
       setMotorStop();
       test_state = 7;
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
      ackLinkTest( &stDevInfo->devLoacalIEEEAddr[0] );
    }
    else
      ackLinkTest( &stDevInfo->devLoacalIEEEAddr[0] );
     return ( events ^ ZIGBEE_LINK_TEST_EVT );
  }
  
  if(events & READ_ZIGBEE_ADC_EVT)
  {
      if(zlgSleepOrwake == sleepState)
      {
        osal_stop_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT);
        SET_ZM516X_WAKEUP();
        zlgSleepOrwake = wakeState;
      }
      read_temporary_adc_value(localAddress);
      HAL_GPIO_CHANGE_DELAY();
      if(uartReturnFlag.readAdcSuccessFlag)
      {
          uartReturnFlag.readAdcSuccessFlag = 0;
          motorStopAdcReport(uartReturnFlag.adc_value);   
          if(uartReturnFlag.adc_value > 0x0030)
          {
              if( parkingState.lockState == cmdLocking )  
              {
                  parkingState.lockState = cmdLockFailed;
                  setMotorForward();
                  eventReportData.event =  cmdLockFailed ;
                  osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
              }
              else if( parkingState.lockState == cmdUnlocking )  
              {
                  parkingState.lockState = cmdUnlockFailed;                 
                  setMotorReverse();
                  eventReportData.event =  cmdUnlockFailed ;
                  osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
              }
              else if(parkingState.lockState != cmdLockFailed && \
                parkingState.lockState != cmdUnlockFailed)
              {
                setMotorStop();
              }
          }     
      }
      osal_start_reload_timer( zigbee_TaskID, READ_ZIGBEE_ADC_EVT,1000);
      return ( events ^ READ_ZIGBEE_ADC_EVT );
  }
  
  if( events & EVENT_REPORT_EVT )
  {
      if(!eventReportData.reportSuccess)
      {}
      else
      {
          eventReportData.reportSuccess = 0;
          osal_stop_timerEx( zigbee_TaskID, EVENT_REPORT_EVT);
          return ( events ^ EVENT_REPORT_EVT );
      }
      if(zlgSleepOrwake == sleepState)
      {
        osal_stop_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT);
        SET_ZM516X_WAKEUP();
        zlgSleepOrwake = wakeState;
      }     
      eventReport( (parkingEvent_t)eventReportData.event );
      osal_start_timerEx( zigbee_TaskID, EVENT_REPORT_EVT, 1000 );
      return ( events ^ EVENT_REPORT_EVT );
  }
     
  if(events & ZIGBEE_RESTORE_FACTORY_EVT)
  {
     if(!uartReturnFlag.restoreSuccessFlag)
     {
        restore_factory_settings( localAddress );
        osal_start_timerEx( zigbee_TaskID, ZIGBEE_RESTORE_FACTORY_EVT, 1000 );
        return ( events ^ ZIGBEE_RESTORE_FACTORY_EVT );
     }
     uartReturnFlag.restoreSuccessFlag = 0;
     osal_stop_timerEx( zigbee_TaskID, ZIGBEE_RESTORE_FACTORY_EVT);
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_START_DEVICE_EVT ,100 );
     return ( events ^ ZIGBEE_RESTORE_FACTORY_EVT );
  }
  
  if( events & ZIGBEE_SLEEP_ZM516X_EVT )
  {
     SET_ZM516X_SLEEP();
     zlgSleepOrwake = sleepState;
     osal_start_timerEx( zigbee_TaskID, ZIGBEE_WAKE_ZM516X_EVT ,1000 );
     return ( events ^ ZIGBEE_SLEEP_ZM516X_EVT );
  }
  
  if( events & ZIGBEE_WAKE_ZM516X_EVT )
  {
     SET_ZM516X_WAKEUP();
     zlgSleepOrwake = wakeState;
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
            case stateHeartBeat:
              osal_set_event( zigbee_TaskID, ZIGBEE_SLEEP_ZM516X_EVT );
              break;
            case stateDataRequset:
              dateRequset();
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
              if( parkingState.lockState == cmdUnlockSuccess )
              {
                eventReportData.event =  cmdUnlockSuccess ;
                osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
                break;
              }
              test_state = 4;
              osal_set_event( zigbee_TaskID, BOARD_TEST_EVT );
              break;
            case stateMotorReverse:
              if( parkingState.lockState == cmdLockSuccess )
              {
                eventReportData.event =  cmdLockSuccess ;
                osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
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
       cmd_len = 0;
       break;
     case cmdHeartBeatPkg:
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
   if(*command == 'S' && *(command+1) == 'E' && *(command+2) == 'N')
   {
     switch(cmd)
     {
     case 0x04:
       cmd_len = 1;
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
    if( parkingState.lockState == cmdUnlocking )
    {
        eventReportData.event =  cmdUnlockSuccess ;
        osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
    }
    parkingState.lockState = cmdUnlockSuccess;
  }
//lock
  else if ( keys == 0 )
  { 
    setMotorStop();    
    if( parkingState.lockState == cmdLocking )
    {
        eventReportData.event =  cmdLockSuccess ;
        osal_set_event( zigbee_TaskID, EVENT_REPORT_EVT );
    }
    parkingState.lockState = cmdLockSuccess;
  }
//over limit
  else if ( keys == 0x01 )
  {  
    setMotorForward();
  }
  else
  {
    if( parkingState.lockState != cmdLocking && parkingState.lockState != cmdUnlocking )
    {
      if(parkingState.lockState == cmdLockSuccess)
      {
        setMotorReverse();
        
      }
      else if(parkingState.lockState == cmdUnlockSuccess)
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

#endif
