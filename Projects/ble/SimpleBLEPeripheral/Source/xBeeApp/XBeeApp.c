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
//#include "hal_sensor.h"
#include "npi.h"
#include "XBeeProtocol.h"
#include "XBeeBsp.h"
#include "hal_types.h"
#include "OnBoard.h"
#include "xbee_api_opt.h"
#include "hal_uart.h"
#include "hal_xbee.h"
#include <math.h>
#include <stdlib.h>

//#define __TEST


#if defined _XBEE_APP_



__xdata uint8 XBeeTaskID;                   // Task ID for internal task/event processing       
__xdata XBeeUartRecDataDef XBeeUartRec;     //���ڽ��ջ�������  
__xdata FlagJionNetType FlagJionNet;        //���������־
__xdata uint8 FlagPowerON=0;                //������־
//static uint8 FlagXBeeTrans=0;             //xbee���ݷ���״̬
__xdata uint8 SendTimes;                    //����ʹ���
__xdata XBeeAdrType XBeeAdr;                //IEEE��ַ�͵�ǰ�������ַ
TaskSendType TaskSend;                      //���ݷ��ʹ���
ToReadUARTType ToReadUART=ReadHead;         //��ȡ����״̬
ToReadUARTType CtlToReadUART=ReadNone;      //���ƶ�ȡ����״̬
uint8 XBeeUartEn=0;                            //���ڶ�ȡʹ��
uint8 LcokState;                            //��״̬��־
DeviceTypeDef DeviceType;                   //��ǰ�豸����
ParkingStateType parkingState;              //��ǰ��λ״̬
uint8 XBeeSOW;                              //xbee���߱�־
uint8 test123;


void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //��ʼ��UART 
    InitUart1();  //��ʼ������1
    RegisterForKeys( XBeeTaskID );
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //�����¼�
    //osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
}


uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;    
  
    if ( events & XBEE_START_DEVICE_EVT )       //��ʼ����
    { 
        //�ж�xbee�Ƿ�������
        if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
        {
            XBeeSleepMode1();
            XBeePinWake();
            test123=1;
        }
        osal_start_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT, 600 );
        return ( events ^ XBEE_START_DEVICE_EVT );
    }
    
    if( events & XBEE_JOIN_NET_EVT)             //��������
    {
        switch(FlagJionNet)
        {
            case JoinNet:
                XBeeRourerJoinNet();
                test123=2;
                break;
            case GetSH:
                XBeeReadSH();
                test123=3;
                break;
            case GetSL:
                XBeeReadSL();
                break;
            case GetMY:
                XBeeReadMY(RES);
                break;
            case JoinPark:
                XBeeReqJionPark();
                break;  
            default:
                break;
            }
        osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 600 );
        return ( events ^ XBEE_JOIN_NET_EVT );
    }
   
    if( events & XBEE_MCU_UART_SEND_EVT )       //xbee��������
    {
        return ( events ^ XBEE_MCU_UART_SEND_EVT );
    }

    if(events & XBEE_HMC5983_EVT)               //������������
    {
        Uart1_Send_Byte("get",osal_strlen("get"));
        if(hmc5983Data.state!=0)
        {
            osal_start_reload_timer( task_id, XBEE_HMC5983_EVT,1000);
            return ( events ^ XBEE_HMC5983_EVT );
        }      
        hmc5983Data.state = 1;
        if( abs(hmc5983DataStandard.x - hmc5983Data.x) > 150 || abs(hmc5983DataStandard.y - hmc5983Data.y) > 150 \
                                                          || abs(hmc5983DataStandard.z - hmc5983Data.z) > 150)  
    
        {   
            if(parkingState.vehicleState == ParkingUnUsed)
            {  
                parkingState.vehicleState = ParkingUsed;
                if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==0)  //0  ����
                {
                    //����xbee
                    //��ʱ
                    XBeeParkState(ParkingUsed);  
                }
                else
                    XBeeParkState(ParkingUsed);              
            }
            if(parkingState.vehicleState == ParkingUsed)
            {
                parkingState.vehicleState = ParkingUnUsed;
                if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==0)  //0  ����
                {
                    //����xbee
                    //��ʱ
                    XBeeParkState(ParkingUnUsed);  
                }
                else
                    XBeeParkState(ParkingUnUsed);
            }
        }                   
        osal_start_reload_timer( task_id, XBEE_HMC5983_EVT,1000);
        return ( events ^ XBEE_HMC5983_EVT );
    }
    
    if( events & XBEE_CTL_MCU_UART_READ_EVT )   //��ȡ����
    {
        return ( events ^ XBEE_CTL_MCU_UART_READ_EVT );
    }
    
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //�������յ���xbee����,������ϣ����XBeeUartRec.num
    { 
        //uint16 CmdState;
        uint8 FrameTypeState;
    
        if(XBeeUartRec.num==0)
        {
            UAR_TXBEE_EN; 
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        }
        FrameTypeState = XBeeUartRec.data[3];
        switch(FrameTypeState)
        {
            case 0x90:  //�����յ���RF��
                if(XBeeUartRec.data[15]=='C' && XBeeUartRec.data[16]=='F' && XBeeUartRec.data[17]=='G')
                {
                    CFGProcess((uint8*)&XBeeUartRec.data[18]);
                    osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT ); 
                }
                if(XBeeUartRec.data[15]=='C' && XBeeUartRec.data[16]=='T' && XBeeUartRec.data[17]=='L')
                    CTLProcess(XBeeUartRec.data[18]);
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
                    }
                    else
                        FlagJionNet = JoinNet;
                    XBeeUartRec.num=0;
                    UAR_TXBEE_EN;   //ʹ�ܴ��ڽ���     
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
                    UAR_TXBEE_EN;
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
                    UAR_TXBEE_EN;
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
                    UAR_TXBEE_EN;
                    osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
                    return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
                } 
                if(XBeeUartRec.data[5]=='S' && XBeeUartRec.data[6]=='M')
                {
                    if(XBeeUartRec.data[7] == 0)
                    {
                        FlagJionNet = JoinNet;
                        osal_stop_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT);
                        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
                    }
                    XBeeUartRec.num = 0;    
                    UAR_TXBEE_EN;                    
                    return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
                }
                break;
            case 0x8A:        //Zigbeeģ��״̬���Զ��ط�֡���ݲ�����
                break;
            case 0x8B:        //�����ͷ���ֵ     
                break;     
            default:
                break;
        }     
        XBeeUartRec.num=0;
        UAR_TXBEE_EN; 
        return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
    }  
  
    if( events & XBEE_SLEEP_EVT )               //����xbee����
    {
        return ( events ^ XBEE_SLEEP_EVT );
    }
    
    if( events & XBEE_WAKEUP_EVT )              //����xbee����
    {  
        return ( events ^ XBEE_WAKEUP_EVT );
    }
  
    if( events & XBEE_MOTOO_CTL_EVT )           //����MOTO����
    {
        return ( events ^ XBEE_SLEEP_EVT );
    }
  
    if( events & XBEE_TEST_EVT )                //������
    {      
        if(SendTimes==0)
        {
            XBeeReadRegCmd("D0");
            SendTimes=1;
        }
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 600 );
        return ( events ^ XBEE_TEST_EVT );
    }
   
    if( events & XBEE_IDLE_EVT )                //������������ʧ�ܺ���룬�ٴ�������Ҫ����
    { 
        return ( events ^ XBEE_IDLE_EVT );
    }
 
    return 0;
}
/**********************************************************
**brief ��ȡxbee���͵���������
**********************************************************/
static void npiCBack_uart( uint8 port, uint8 events )
{
    uint16 rev_data_temp;
    uint8 cnt;
    static uint8 checksum=0;
    uint16 numBytes=0,RecLen=0;
    static uint16 APICmdLen=0;
  
    if(XBeeUartEn == 1)  //Ĭ��ֵΪ0 ʹ��
        return;
    numBytes = NPI_RxBufLen();
    if(numBytes==0)
        return; 
    switch(ToReadUART)
    {
        case ReadHead:
            RecLen = NPI_ReadTransport( XBeeUartRec.data, 1); 
            if(*XBeeUartRec.data == 0x7E)
            {
                XBeeUartRec.num = 1;
                ToReadUART = ReadLen;
            }
            break;
        case ReadLen:
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
            ToReadUART = ReadData;
            break;
        case ReadData:
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
            ToReadUART = ReadHead; 
            UAR_TXBEE_DIS;
            osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT ); 
            break;
        default:
            break;
    }
    return;  
}



#endif