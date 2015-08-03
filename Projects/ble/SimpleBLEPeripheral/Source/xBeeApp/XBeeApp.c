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



//#define __TEST


#if defined _XBEE_APP_



__xdata uint8 XBeeTaskID;                   // Task ID for internal task/event processing       
__xdata XBeeUartRecDataDef XBeeUartRec;     //串口接收缓存数据  
__xdata FlagJionNetType FlagJionNet;        //加入网络标志
__xdata uint8 FlagPowerON=0;                //启动标志
//static uint8 FlagXBeeTrans=0;             //xbee数据发送状态
__xdata uint8 SendTimes;                    //命令发送次数
__xdata XBeeAdrType XBeeAdr;                //IEEE地址和当前的网络地址
TaskSendType TaskSend;                      //数据发送次数
ToReadUARTType ToReadUART=ReadHead;         //读取串口状态
ToReadUARTType CtlToReadUART=ReadNone;      //控制读取串口状态
uint8 UartCtl=0;                            //串口读取使能
uint8 LcokState;                            //锁状态标志
DeviceTypeDef DeviceType;                   //当前设备类型



void XBeeInit( uint8 task_id )
{
  XBeeTaskID = task_id;
  NPI_InitTransport(npiCBack_uart);         //初始化UART 
  osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //触发事件
}


uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;    
  
  if ( events & XBEE_START_DEVICE_EVT )  //起始任务
  { 
#ifdef __TEST
    osal_set_event( XBeeTaskID, XBEE_TEST_EVT );  //开始测试
    return ( events ^ XBEE_START_DEVICE_EVT );
#else
   if(FlagPowerON == 0 && SendTimes == 0)
   {
      XBeeReadSM();
      //XBeeSendSM(Disable,RES);
      SendTimes++;
   } 
    //查看是否已加入网络，并置位标志
   if( FlagJionNet != NetOK && FlagPowerON == 1)
   {
     FlagJionNet = JoinNet;
     osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 10 );
   }
   osal_start_timerEx( XBeeTaskID, XBEE_MCU_UART_SEND_EVT, 100 );    
   //FlagPowerON = 2;  //上电标志位置位
   return ( events ^ XBEE_START_DEVICE_EVT );
#endif
  }
  
  if( events & XBEE_JOIN_NET_EVT)  //加入网络
  {
    switch(FlagJionNet)
    {
      case JoinNet:
        XBeeRourerJoinNet();
        FlagJionNet = InNone;
        break;
      case GetSH:
        XBeeReadSH();
        FlagJionNet = InNone;
        break;
      case GetSL:
        XBeeReadSL();
        FlagJionNet = InNone;
        break;
      case GetMY:
        XBeeReadMY(RES);
        FlagJionNet = InNone;
        break;
      case JoinPark:
        XBeeReqJionPark();
        FlagJionNet = InNone;
        break;  
      default:
        break;
    }
    //发送加入请求已经完成，下一步完成网关接收处理函数
    osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 100 );
    return ( events ^ XBEE_JOIN_NET_EVT );
  }
  if( events & XBEE_MCU_UART_SEND_EVT )   //串口发送任务
  {
    osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 100 );
    return ( events ^ XBEE_MCU_UART_SEND_EVT );
  }
  
  if( events & XBEE_CTL_MCU_UART_READ_EVT )  //读取串口
  {
    switch(CtlToReadUART)
    {
      case ReadLen:
          ToReadUART = ReadLen;
          osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 1 );
          return ( events ^ XBEE_CTL_MCU_UART_READ_EVT );
          break;
        case ReadData:
          ToReadUART = ReadData;
          osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 1 );
          return ( events ^ XBEE_CTL_MCU_UART_READ_EVT );
          break;
        default:
          break;
    }
    osal_start_timerEx( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT, 100 );
    return ( events ^ XBEE_CTL_MCU_UART_READ_EVT );
  }
  
  if( events & XBEE_REC_DATA_PROCESS_EVT )  //处理串口收到数据,处理完毕，清除XBeeUartRec.num
  { 
 //   uint16 CmdState;
    uint8 FrameTypeState;
    
    if(XBeeUartRec.num==0)
    {
      UartCtl = 0;
      osal_start_timerEx( XBeeTaskID, XBEE_MCU_UART_SEND_EVT, 100 );  
      return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
    }
    FrameTypeState = XBeeUartRec.data[3];
    switch(FrameTypeState)
    {
      case 0x90:  //处理收到的RF包
        if(XBeeUartRec.data[15]=='C' && XBeeUartRec.data[16]=='F' && XBeeUartRec.data[17]=='G')
          CFGProcess(XBeeUartRec.data[18]);
        if(XBeeUartRec.data[15]=='C' && XBeeUartRec.data[16]=='T' && XBeeUartRec.data[17]=='L')
          CTLProcess(XBeeUartRec.data[18]);
        if(XBeeUartRec.data[15]=='S' && XBeeUartRec.data[16]=='E' && XBeeUartRec.data[17]=='N')
        {}
        if(XBeeUartRec.data[15]=='O' && XBeeUartRec.data[16]=='T' && XBeeUartRec.data[17]=='A')
        {}
        if(XBeeUartRec.data[15]=='T' && XBeeUartRec.data[16]=='S' && XBeeUartRec.data[17]=='T')
        {}        
        break;
      case 0x88:  //处理收到的AT指令返回值
        if(XBeeUartRec.data[5]=='N' && XBeeUartRec.data[6]=='J')
        {
          //如果没有执行OK，再次执行
        }
        if(XBeeUartRec.data[5]=='A' && XBeeUartRec.data[6]=='I')
        {
          //如果没有执行OK，再次执行
          if(XBeeUartRec.data[7]==0 && XBeeUartRec.data[8]==0)
            FlagJionNet = GetSH;
          else
             FlagJionNet = JoinNet;
          XBeeUartRec.num=0;
          UartCtl = 0;
          osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
          return ( events ^ XBEE_REC_DATA_PROCESS_EVT );        
        }
        if(XBeeUartRec.data[5]=='S' && XBeeUartRec.data[6]=='H')
        {
          if(XBeeUartRec.data[7]==0)
          {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
              XBeeAdr.IEEEadr[cnt] = XBeeUartRec.data[8+cnt];
            FlagJionNet = GetSL;
          }
          else
            FlagJionNet = GetSH;
          XBeeUartRec.num=0;
          UartCtl = 0;
          osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
          return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        }
        if(XBeeUartRec.data[5]=='S' && XBeeUartRec.data[6]=='L')
        {
          if(XBeeUartRec.data[7]==0)
          {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
              XBeeAdr.IEEEadr[4+cnt] = XBeeUartRec.data[8+cnt];
            FlagJionNet = GetMY;
          }
          else
            FlagJionNet = GetSL;
          XBeeUartRec.num=0;
          UartCtl = 0;
          osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
          return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        }
        if(XBeeUartRec.data[5]=='M' && XBeeUartRec.data[6]=='Y')
        {
          if(XBeeUartRec.data[7]==0)
          {
            uint8 cnt;
            for(cnt=0;cnt<2;cnt++)
              XBeeAdr.netadr[cnt] = XBeeUartRec.data[8+cnt];
            FlagJionNet = JoinPark;
          }
          else
            FlagJionNet = GetMY;
          XBeeUartRec.num=0;
          UartCtl = 0;
          osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
          return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        } 
        if(XBeeUartRec.data[5]=='S' && XBeeUartRec.data[6]=='M')
        {
          if(XBeeUartRec.data[7] != 0)
            goto theloop;  //跳转出 SM选项
          if(XBeeUartRec.data[8]==0)
            DeviceType = ROUTOR;
          else                                                   //if(XBeeUartRec.data[7]==0 && XBeeUartRec.data[8]==4)
          {  
            DeviceType = ENDER;
            XBeeSleepInit();
          }
          FlagPowerON =1;
        theloop : XBeeUartRec.num = SendTimes = 0;    
          UartCtl = 0;
          osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );
          return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        }
        break;
      case 0x8A:        //Zigbee模块状态，自动回发帧，暂不处理
        
        break;
      case 0x8B:  //处理发送返回值
        
        break;
      
      default:
        break;
    }     
    XBeeUartRec.num=0;
    UartCtl = 0;
    osal_start_timerEx( XBeeTaskID, XBEE_MCU_UART_SEND_EVT, 100 );  
    return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
  }  
  
  if( events & XBEE_SLEEP_EVT )   //控制xbee休眠
  {
    
    osal_start_timerEx( XBeeTaskID, XBEE_MCU_UART_SEND_EVT, 1000 );  
    return ( events ^ XBEE_SLEEP_EVT );
  }
  
  if( events & XBEE_SLEEP_EVT )   //控制xbee唤醒
  {
    
    osal_start_timerEx( XBeeTaskID, XBEE_MCU_UART_SEND_EVT, 1000 );  
    return ( events ^ XBEE_SLEEP_EVT );
  }
  
  if( events & XBEE_MOTOO_CTL_EVT )   //控制MOTO动作
  {
    
    osal_start_timerEx( XBeeTaskID, XBEE_MCU_UART_SEND_EVT, 1000 );  
    return ( events ^ XBEE_SLEEP_EVT );
  }
  
  if( events & XBEE_TEST_EVT )   //测试用
  {
    if(SendTimes == 0)
    {
      XBeeReadSM();
      SendTimes++;
    }  
    
    osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 10 );
    return ( events ^ XBEE_TEST_EVT );
  }
   
  if( events & XBEE_IDLE_EVT )  //空闲任务，入网失败后进入，再次入网需要重启
  {
    osal_start_timerEx( XBeeTaskID, XBEE_IDLE_EVT, 1000 );
    return ( events ^ XBEE_IDLE_EVT );
  }
 
  return 0;
}
/**********************************************************
**brief 对xbee发送到串口数据读取
**********************************************************/
static void npiCBack_uart( uint8 port, uint8 events )
{
  uint16 rev_data_temp;
  uint8 cnt;
  static uint8 checksum=0;
  uint16 numBytes=0,RecLen=0;
  static uint16 APICmdLen=0;
  
  if(UartCtl == 1)  //默认值为0 使能
    return;
  numBytes = NPI_RxBufLen();
  if(numBytes==0)
    return; 
  if(ToReadUART == ReadHead)
  {    
    RecLen = NPI_ReadTransport( XBeeUartRec.data, 1); 
    if(*XBeeUartRec.data == 0x7E)
    {
      XBeeUartRec.num = 1;
      CtlToReadUART = ReadLen;
      ToReadUART = ReadNone;
      osal_set_event( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT );
      return;
    }
    else
      return;
  }
  if(ToReadUART == ReadLen)
  {
    if(numBytes < 2)
      return; 
    RecLen = NPI_ReadTransport((XBeeUartRec.data+1), 2);
    if(RecLen<2)
    {
      ToReadUART = ReadHead;
      return;
    }
    XBeeUartRec.num += 2;
    rev_data_temp = (uint16)*(XBeeUartRec.data+1);
    rev_data_temp <<= 8;
    APICmdLen = rev_data_temp + (uint16)*(XBeeUartRec.data+2);
    CtlToReadUART = ReadData;
    ToReadUART = ReadNone;
    osal_set_event( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT );
    return;
  }
  if(ToReadUART == ReadData)
  {
    if(numBytes < APICmdLen+1)
      return;
    RecLen = NPI_ReadTransport((XBeeUartRec.data+3), APICmdLen+1);
    if(RecLen<(APICmdLen+1))
    {
      ToReadUART = ReadHead;
      return;
    }
    XBeeUartRec.num += RecLen;
    checksum = 0;
    for(cnt=3;cnt<APICmdLen+3;cnt++)
      checksum += *(XBeeUartRec.data + cnt);
    checksum = 0xFF - checksum;
    if(*(XBeeUartRec.data + APICmdLen+3) != checksum)
      return;
    CtlToReadUART = ReadNone;
    ToReadUART = ReadHead;
#if defined __TEST
    osal_set_event( XBeeTaskID, XBEE_TEST_EVT );
#else  
    osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT ); 
    UartCtl = 1;
#endif 
  } 
  return;  
}

#endif