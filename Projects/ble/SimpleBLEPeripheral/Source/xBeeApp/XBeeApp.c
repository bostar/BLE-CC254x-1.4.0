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



__xdata uint8 XBeeTaskID;                   // Task ID for internal task/event processing       
__xdata XBeeUartRecDataDef XBeeUartRec;             //���ڽ��ջ�������  
__xdata FlagJionNetType FlagJionNet;                //���������־
__xdata uint8 FlagPowerON=0;               //������־
//static uint8 FlagXBeeTrans=0;             //xbee���ݷ���״̬
__xdata uint8 SendTimes;                    //����ʹ���
__xdata XBeeAdrType XBeeAdr;                //IEEE��ַ�͵�ǰ�������ַ
TaskSendType TaskSend;                      //���ݷ��ʹ���
ToReadUARTType ToReadUART=ReadHead;         //��ȡ����״̬
ToReadUARTType CtlToReadUART=ReadNone;      //���ƶ�ȡ����״̬

void XBeeInit( uint8 task_id )
{
  XBeeTaskID = task_id;
  NPI_InitTransport(npiCBack_uart);         //��ʼ��UART 
  osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //�����¼�
}


uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
  VOID  task_id;    
  if ( events & XBEE_START_DEVICE_EVT )  //��ʼ����
  { 
    //�ϵ��־λ��λ
    //�鿴�Ƿ��Ѽ������磬����λ��־
   if( FlagJionNet != NetOK && FlagPowerON == 0)
   {
     FlagJionNet = JoinNet;
     FlagPowerON++;
     osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 10 );
   }
  else
   {
     FlagPowerON++;
     osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 100 );    
   }
   return ( events ^ XBEE_START_DEVICE_EVT );
  }
  
  if( events & XBEE_JOIN_NET_EVT)  //��������
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
    //���ͼ��������Ѿ���ɣ���һ��������ؽ��մ�����
    osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 100 );
    return ( events ^ XBEE_JOIN_NET_EVT );
  }
  if( events & XBEE_MCU_UART_SEND_EVT )   //���ڷ�������
  {
    if(SendTimes==0){
      //XBeeReadAI(RES);
      //XBeeRourerJoinNet();
      //XBeeRourerJoinNet();
      XBeeReadMY(RES);
      //XBeeLeaveNet();
      SendTimes++;
    }
    osal_start_timerEx( XBeeTaskID, XBEE_CTL_MCU_UART_READ_EVT, 2 );
    return ( events ^ XBEE_MCU_UART_SEND_EVT );
  }
  
  if( events & XBEE_CTL_MCU_UART_READ_EVT )  //��ȡ����
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
    osal_start_timerEx( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT, 10 );
    return ( events ^ XBEE_CTL_MCU_UART_READ_EVT );
  }
  
  if( events & XBEE_REC_DATA_PROCESS_EVT )  //�������յ�����,������ϣ����XBeeUartRec.num
  { 
 //   uint16 CmdState;
    uint8 FrameTypeState;
    
    if(XBeeUartRec.num==0)
    {
      osal_start_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT, 100 );  
      return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
    }
    FrameTypeState = XBeeUartRec.data[3];
    switch(FrameTypeState)
    {
      case 0x90:  //�����յ���RF��
        if(XBeeUartRec.data[15]=='C' && XBeeUartRec.data[16]=='F' && XBeeUartRec.data[17]=='G')
          CFGProcess(XBeeUartRec.data[18]);
        if(XBeeUartRec.data[15]=='C' && XBeeUartRec.data[16]=='T' && XBeeUartRec.data[17]=='L')
        {}  
        if(XBeeUartRec.data[15]=='S' && XBeeUartRec.data[16]=='E' && XBeeUartRec.data[17]=='N')
        {}
        if(XBeeUartRec.data[15]=='O' && XBeeUartRec.data[16]=='T' && XBeeUartRec.data[17]=='A')
        {}
        if(XBeeUartRec.data[15]=='T' && XBeeUartRec.data[16]=='S' && XBeeUartRec.data[17]=='T')
        {}        
        break;
      case 0x88:  //�����յ���ATָ���ֵ
        if(XBeeUartRec.data[5]=='N' && XBeeUartRec.data[6]=='J')
        {
          //���û��ִ��OK���ٴ�ִ��
        }
        if(XBeeUartRec.data[5]=='A' && XBeeUartRec.data[6]=='I')
        {
          //���û��ִ��OK���ٴ�ִ��
          if(XBeeUartRec.data[7]==0 && XBeeUartRec.data[8]==0)
          {
            FlagJionNet = GetSH;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
          else
          {
             FlagJionNet = JoinNet;
             XBeeUartRec.num=0;
             osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
             return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
        }
        if(XBeeUartRec.data[5]=='S' && XBeeUartRec.data[6]=='H')
        {
          if(XBeeUartRec.data[7]==0)
          {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
              XBeeAdr.IEEEadr[cnt] = XBeeUartRec.data[8+cnt];
            FlagJionNet = GetSL;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
          else
          {
            FlagJionNet = GetSH;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
        }
        if(XBeeUartRec.data[5]=='S' && XBeeUartRec.data[6]=='L')
        {
          if(XBeeUartRec.data[7]==0)
          {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
              XBeeAdr.IEEEadr[4+cnt] = XBeeUartRec.data[8+cnt];
            FlagJionNet = GetMY;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
          else
          {
            FlagJionNet = GetSL;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
        }
        if(XBeeUartRec.data[5]=='M' && XBeeUartRec.data[6]=='Y')
        {
          if(XBeeUartRec.data[7]==0)
          {
            uint8 cnt;
            for(cnt=0;cnt<2;cnt++)
              XBeeAdr.netadr[cnt] = XBeeUartRec.data[8+cnt];
            FlagJionNet = JoinPark;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
          else
          {
            FlagJionNet = GetMY;
            XBeeUartRec.num=0;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
          }
        }
        
        
        break;
      case 0x8A:        //Zigbeeģ��״̬���Զ��ط�֡���ݲ�����
        
        break;
      case 0x8B:  //�����ͷ���ֵ
        
        break;
      
      default:
        break;
    }
      
    XBeeUartRec.num=0;
    osal_start_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT, 100 );  
    return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
  }  
  return 0;
}
/**********************************************************
**brief ��xbee���͵��������ݶ�ȡ
**********************************************************/
static void npiCBack_uart( uint8 port, uint8 events )
{
  uint16 rev_data_temp;
  uint8 cnt;
  static uint8 checksum=0;
  uint16 numBytes=0,RecLen=0;
  static uint16 APICmdLen=0;
  
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
    osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT );  
  } 
  return;  
}

/*static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd)
{
  uint8 i,j;
  i=*command;
  j=cmd;
   return 0;
}*/





#endif