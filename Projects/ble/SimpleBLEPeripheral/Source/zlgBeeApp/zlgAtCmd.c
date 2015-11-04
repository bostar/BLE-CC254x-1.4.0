#include "zlgAtCmd.h"
#include "zlg_protocol.h"
#include "npi.h"
#include "hal_zlg.h"
#include "OSAL.h"
#include "zigbeeApp.h"
#include "zlg_bsp.h"
#include "string.h"
#include "hal_flash.h"
#include "OnBoard.h"

const unsigned short broadcastAddr = 0xffff;
const uint16 firmware_version @ "VERSION" = 0X0006;
//uint16 firmware_version = 0x0001;

dev_info_t * stDevInfo;
uartReturnStatus_t * uartReturnFlag;
search_info_t searchInfo;
unsigned short localAddress;
unsigned char zlgIOLevel;

#define UART_WRITE_BUF_LEN      100
#define UART_READ_BUF_LEN      100

static unsigned char wbuf[UART_WRITE_BUF_LEN];// , rbuf[UART_READ_BUF_LEN] ;

void read_local_cfg( void )
{   
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enReadLoacalCfg;
    wbuf[4] = wbuf[0] + wbuf[1] + wbuf[2] + wbuf[3];

    NPI_WriteTransport( wbuf, 5 );   
}

void set_channel_nv( uint8 nv )
{
    uint8 i,sum = 0;
 
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enSetChannelNv;
    wbuf[4] = nv;
    for(i = 0; i < 5; i++)
    {
      sum += wbuf[i];
    }
    wbuf[5] = sum;
    
    NPI_WriteTransport( wbuf, 6 );
}

void search_node( void )
{
    uint8 i,sum = 0;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enSearchNode;
    for(i = 0 ; i < 4; i++)
      sum += wbuf[i];    
    wbuf[4] = sum;
    
    NPI_WriteTransport( wbuf, 5 );
}

void get_remote_info( uint16 DstAddr )
{
    uint8 i,sum = 0;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enGetRemoteInfo;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    for(i = 0; i < 6; i++)
      sum += wbuf[i];
    wbuf[6] = sum;
    
    NPI_WriteTransport( wbuf, 7 );
}

void write_remote_cfg( uint16 DstAddr, dev_info_t *DevInfo )
{
    int i;
    unsigned char sum = 0;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enModifyCfg;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    osal_memcpy( &wbuf[6], DevInfo, 65 );
    for(i = 0;i < (6 + 65);i++)
    {
        sum += wbuf[i];
    }   
    wbuf[6 + 65] = sum;

    NPI_WriteTransport( wbuf, 6 + 65 + 1 );
}

void write_local_cfg( uint16 LocAddr, dev_info_t *DevInfo )
{
    int i;
    unsigned char sum = 0;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enModifyCfg;
    wbuf[4] = LocAddr >> 8;
    wbuf[5] = LocAddr;  

    osal_memcpy( &wbuf[6], DevInfo, 65 );
    for(i = 0;i < (6 + 65);i++)
    {
        sum += wbuf[i];
    }    
    wbuf[6 + 65] = sum;

    NPI_WriteTransport( wbuf, 6 + 65 + 1 );
}

void reset_node( uint16 DstAddr )
{
    int i;
    unsigned char sum = 0;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enResetNode;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = 0x00;
    wbuf[7] = 0x01;
    for( i = 0; i < 8; i++ )
      sum += wbuf[i];
    wbuf[8] = sum;
    
    NPI_WriteTransport( wbuf, 9 );
}

void restore_factory_settings( uint16 DstAddr )
{
    int i;
    unsigned char sum = 0;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enResetCfg;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = 0x00;
    wbuf[7] = 0x01;
    for(i = 0; i < 8; i++)
      sum += wbuf[i];
    wbuf[8] = sum;
    
    NPI_WriteTransport( wbuf, 9 );
}

void sf_reset_node( void )
{
    uint8 i;
    uint8 sum;

    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;  
    wbuf[3] = enResetNode;  
    wbuf[4] = stDevInfo->devLoacalNetAddr[0];
    wbuf[5] = stDevInfo->devLoacalNetAddr[1];
    wbuf[6] = 0x00;
    wbuf[7] = stDevInfo->devMode;
    for(i = 0 ; i < 8 ; i++)
    {
        sum += wbuf[i];
    }
    wbuf[8] = sum;

    NPI_WriteTransport( wbuf, 9 );
}

void set_temporary_channel( uint8 channel )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;  
    wbuf[3] = enSetChannel;  
    wbuf[4] = channel;  
    NPI_WriteTransport( wbuf , 5 );
  
}

void set_temporary_DestAddr( uint16 DestAddr )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;  
    wbuf[3] = enSetDestAddr; 
    wbuf[4] = DestAddr >> 8;
    wbuf[5] = DestAddr;

    NPI_WriteTransport( wbuf , 6 );
}

void set_temporary_ShowSrcAddr( showSrcAddr_cmd_t cmd )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enShowSrcAddr;  
    wbuf[4] = cmd;

    NPI_WriteTransport( wbuf , 5 );
}

void set_temporary_io_dir( uint16 DstAddr, uint8 IO_Dir )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef; 
    wbuf[3] = enSetIoDirection;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = IO_Dir;

    NPI_WriteTransport( wbuf , 7 );  
}

void read_temporary_io_level( uint16 DstAddr )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enReadIoStatus;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;

    NPI_WriteTransport( wbuf , 6 );  
}

void set_temporary_io_level( uint16 DstAddr, uint8 IO_Level )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enSetIoStatus;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = IO_Level;

    NPI_WriteTransport( wbuf , 7 );  
}

void read_temporary_adc_value( uint16 DstAddr )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enReadAdcValue;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = 0x00;

    NPI_WriteTransport( wbuf , 7 );
}

void temporary_enter_sleep_mode( void )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enEnterSleepMode;
    wbuf[4] = 0x01;

    NPI_WriteTransport( wbuf , 5 );
}

void set_temporary_cast_mode( cast_mode_t mode )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enSetUnicastOrBroadcast;
    wbuf[4] = mode;

    NPI_WriteTransport( wbuf , 5 );
}

void read_temporary_node_rssi( uint16 DstAddr )
{
    wbuf[0] = 0xde;
    wbuf[1] = 0xdf;
    wbuf[2] = 0xef;
    wbuf[3] = enReadNodeRssi; 
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;

    NPI_WriteTransport( wbuf , 6 );
}

uint8 receive_data( uint8 *rbuf, uint16 len )
{
    uint8 state = stateNoWork;

    if( ( *rbuf == 0xab ) && ( *(rbuf+1) == 0xbc ) && ( *(rbuf+2) == 0xcd ) )
    {
        switch( *(rbuf+3) )
        {
            case enReadLoacalCfg:
              osal_memcpy( stDevInfo, rbuf+4, sizeof(dev_info_t) );
              localAddress = stDevInfo->devLoacalNetAddr[0]<<8 | stDevInfo->devLoacalNetAddr[1];
              uartReturnFlag->readLocalCfg_SUCCESS = 1;
              state = stateReadCfg;
              break;
            case enSetChannelNv:
              if( *(rbuf+4) == 0x00 )
                return enSetChannelNv;
              break;
            case enSearchNode:
              osal_memcpy( &searchInfo, rbuf+4, sizeof( search_info_t ) );
              break;
            case enGetRemoteInfo:
              osal_memcpy( stDevInfo, rbuf+4, sizeof( dev_info_t ) );
              break;
            case enModifyCfg:
              //rbuf[4-5]:网络地址 rbuf[6]:00成功
              if( *(rbuf+6) == 0x00 )
                uartReturnFlag->writeLocalCfg_SUCCESS = 1;
              state = stateWriteCfg;
              break;
            case enResetCfg:
              //rbuf[4-5]:网络地址 rbuf[6-7]:设备类型 rbuf[8]:00成功
              if( *(rbuf+8) == 0x00 )
                uartReturnFlag->restoreSuccessFlag = 1;
              state = stateRestoreFactoryConfig;
              break;
            default:      
              break;
        }
    }
    if( (*rbuf == 0xde) && (*(rbuf+1) == 0xdf) && (*(rbuf+2) == 0xef) )
    {
        switch( *(rbuf+3) )
        {
            case enSetChannel:
              if( *(rbuf+4) == 0x00 )
                return enSetChannel;
              break;
            case enSetDestAddr:
              if( *(rbuf+4) == 0x00 )
                return enSetDestAddr;
              break;
            case enShowSrcAddr:
              if( *(rbuf+4) == 0x00 )
                return enShowSrcAddr;
              break;
            case enSetIoDirection:
              if( *(rbuf+6) == 0x00 )
                uartReturnFlag->gpioDirectionSet_SUCCESS = 1;
              state = stateGpioSet;
              break;
            case enReadIoStatus:
              //buf[4-5]:addr;buf[6]:io_level
              zlgIOLevel = *(rbuf+6);
              break;
            case enSetIoStatus:
              if( *(rbuf+6) == 0x00 )
                //return enSetIoStatus;
              break;
            case enReadAdcValue:
              //buf[4-5]:addr;buf[6-7]:adc_value
              uartReturnFlag->readAdcSuccessFlag = 1;
              uartReturnFlag->adc_value = ( uint16 )*(rbuf+6) << 8 | *(rbuf+7);
              break;
            case enSetUnicastOrBroadcast:
              if( *(rbuf+4) == 0x00 )
                //return enSetUnicastOrBroadcast;
              break;
            case enReadNodeRssi:
              //buf[4]:rssi
              break;
            default:
              break;
        }
    }
    if( ( *rbuf == 'C' ) && ( *(rbuf+1) == 'F' ) && ( *(rbuf+2) == 'G' ) )
    {
        switch( *(rbuf+3) )
        {
            case cmdAckCheckIn:
                if( !memcmp( &stDevInfo->devLoacalIEEEAddr[0], rbuf+4, 8 ) && *(rbuf+12) == 0x01 )
                {
                  stDevInfo->devLoacalNetAddr[0] = *(rbuf+13);
                  stDevInfo->devLoacalNetAddr[1] = *(rbuf+14);
                  stDevInfo->devPanid[0] = *(rbuf+15);
                  stDevInfo->devPanid[1] = *(rbuf+16);
                  stDevInfo->devChannel = *(rbuf+17);
                  state = stateApplyNetwork;
                  uartReturnFlag->applyNetWork_SUCCESS = 1;
                }
                break;
            case cmdLinkTest:
                if( !memcmp( &stDevInfo->devLoacalIEEEAddr[0], rbuf+4, 8 ) )
                {
                    state = stateAckLinkTest;//ackLinkTest();
                }
                break;
            case cmdRestoreFactoryConfig:
                if( ( stDevInfo->devLoacalNetAddr[0] == *(rbuf+4) && \
                      stDevInfo->devLoacalNetAddr[1] == *(rbuf+5) ) || \
                      ( 0xff == *(rbuf+4) && 0xff == *(rbuf+5) ) )
                {
                  //
                  state = stateRestoreFactoryConfig;
                }
                break;
            default:
                break;
        }
    }
    if( ( *rbuf == 'T' ) &&( *(rbuf+1) == 'S' ) && ( *(rbuf+2) == 'T' ) )
    {
        switch( *(rbuf+3) )
        {
            case cmdBeepTest:
                if( ( stDevInfo->devLoacalNetAddr[0] == *(rbuf+5) && \
                      stDevInfo->devLoacalNetAddr[1] == *(rbuf+6) ) || \
                      ( 0xff == *(rbuf+5) && 0xff == *(rbuf+6) ) )
                {
                      if( cmdBuzz == *(rbuf+4) )
                        state = stateBeepOn;
                        //setBeepOn();
                      else if( cmdSilence == *(rbuf+4) )
                        //setBeepOff();
                        state = stateBeepOff;
                }
              break;
            case cmdLedTest:
                if( ( stDevInfo->devLoacalNetAddr[0] == *(rbuf+5) && \
                      stDevInfo->devLoacalNetAddr[1] == *(rbuf+6) ) || \
                      ( 0xff == *(rbuf+5) && 0xff == *(rbuf+6) ) )
                {
                      uartReturnFlag->ledBitState = *(rbuf+4);
                      state = stateLedTest;
                }
              break;
            case cmdMotorTest:
                if( ( stDevInfo->devLoacalNetAddr[0] == *(rbuf+5) && \
                      stDevInfo->devLoacalNetAddr[1] == *(rbuf+6) ) || \
                      ( 0xff == *(rbuf+5) && 0xff == *(rbuf+6) ) )
                {
                      switch( *(rbuf+4) )
                      {
                      case cmdStop:
                        setMotorStop();
                        break;
                      case cmdForward:
                        setMotorForward();
                        break;
                      case cmdReverse:
                        setMotorReverse();
                        break;
                      default:
                        break;
                      }
                }
              break;
            default:
              break;      
        }
    }
    if( ( *rbuf == 'C' ) && ( *(rbuf+1) == 'T' ) && ( *(rbuf+2) == 'L' ) )
    {
        switch( *(rbuf+3) )
        {
            case 0x00:
                if( ( stDevInfo->devLoacalNetAddr[0] == *(rbuf+5) && \
                      stDevInfo->devLoacalNetAddr[1] == *(rbuf+6) ) || \
                      ( 0xff == *(rbuf+5) && 0xff == *(rbuf+6) ) )
                {
                    if( *(rbuf+4) == 0x00 )
                    {
                    //          setMotorForward();unlock
                      state = stateMotorForward;
                    }
                    else if( *(rbuf+4) == 0x01 )
                    {
                    //          setMotorReverse();//lock
                      state = stateMotorReverse;
                    }
                    else if( *(rbuf+4) == 0x03 )
                    {
                      state = stateOTAPrepare;
                    }
                    else if( *(rbuf+4) == 0x04)
                    {
                      HAL_SYSTEM_RESET();
                    }
                    else if( *(rbuf+4) == 0x05)
                    {
                      state = stateSynSleep;
                    }
//                    else if( *(rbuf+4) == 0x06)
//                    {
//                      state = stateTestLink;
//                    }
                    else if( *(rbuf+4) == 0x07)
                    {
                      state = stateKeepWake;
                    }
                    else if( *(rbuf+4) == 0x08 )
                    {
                      state =  stateResetSensor;
                    }
                    else
                    {
                      setMotorStop();
                    }
                }
                break;
            default:
                break;
        }
    }
    if( ( *rbuf == 'S' ) && ( *(rbuf+1 ) == 'E' ) && ( *(rbuf+2) == 'N' ) )
    {
        switch( *(rbuf+3) )
        {
        case cmdSensorCalibration:
            //开始标定
            break;
        default:
            break;
        }
    }
    if( ( *rbuf == 'O' ) && ( *(rbuf+1) == 'T' ) && ( *(rbuf+2) == 'A' ) )
    {
        static uint16 firmwareVersion,firmwareSize;
        switch( *(rbuf+3) )
        {
        case 0x00:
            firmwareVersion = *(uint16*)(rbuf+4);
            firmwareSize =  *(uint16*)(rbuf+6);
            if( firmwareVersion > firmware_version )
            {
              SET_ZM516X_WAKEUP();
              osal_memset(rbuf,0,4);
              HalFlashWrite(0x890 / 4,(uint8*)rbuf,1);
              //HalFlashErase(0x800 / 4 / (2048 / 4));//SBL_PAGE_SIZE);// CHECKSUM & CRC_SHDW
              HAL_SYSTEM_RESET();
              //Onboard_soft_reset();
            }
            VOID firmwareVersion;
            VOID firmwareSize;
            break;
        default:
            break;
        }
    }
    return state;
}

void send_data_to_local_node( uint16 DstAddr, uint8 *data, uint8 len )
{
//    set_temporary_DestAddr(DstAddr);
//    set_temporary_cast_mode( unicast );
    osal_memcpy( wbuf, data, len );
    NPI_WriteTransport ( wbuf , len );
}

void send_data_to_remote_node( uint16 destAddr, unsigned char *data, int len )
{
    uint8 responseLen;

    if( destAddr == broadcastAddr )
    {
        set_temporary_cast_mode( broadcast );
    }
    else
    {
      do
      {
        set_temporary_DestAddr( destAddr );
        responseLen = NPI_RxBufLen();
      }while( responseLen != 5 );
    }
    
    NPI_WriteTransport( data, len );
}
