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
uint8 SenFlag=0x88;                              //��������ֵ��־
uint8 test123;
LockCurrentStateType LockObjState;
SetSleepModeType SetSleepMode;                       //

void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //��ʼ��UART 
    InitUart1();  //��ʼ������1
    RegisterForKeys( XBeeTaskID );
    parkingState.vehicleState = ParkingUnUsed;
    MotorInit();
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //�����¼�
    //osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    //osal_set_event( XBeeTaskID, XBEE_TEST_EVT );
}

uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;    
    
    if ( events & XBEE_START_DEVICE_EVT )       //��ʼ����
    {
        //��������ģʽ
        if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
        {
            switch(SetSleepMode)
            {
                case SetMode:
                    XBeeSendSM(PinCyc,RES);
                    break;
                case SetSP:
                    XBeeSetSP(100,RES);
                    break;
                case SetST:
                    XBeeSetST(100,RES);
                    break;
                case SetSN:
                    break;
                default:
                    break;
            }
        }
        osal_start_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT, 100 );
        return (events ^ XBEE_START_DEVICE_EVT) ;
    }
    
    if( events & XBEE_JOIN_NET_EVT)             //��������
    {
        switch(FlagJionNet)
        {
            case JoinNet:
                XBeeRourerJoinNet();
                break;
            case GetSH:
                XBeeReadSH();
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
        osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 2000 );
        return (events ^ XBEE_JOIN_NET_EVT) ;
    }
    
    if(events & XBEE_HMC5983_EVT)               //������������
    {
        HMC5983DataType temp_hmc5983Data,temp_hmc5983DataStandard;
        static uint8 cnt=0;
        
        temp_hmc5983Data = hmc5983Data;
        temp_hmc5983DataStandard = hmc5983DataStandard;
        
        if(temp_hmc5983Data.state!=0x88)
        {
            Uart1_Send_Byte("get",osal_strlen("get"));
            osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
            return ( events ^ XBEE_HMC5983_EVT );
        }      
        hmc5983Data.state = 1;
        if( abs(temp_hmc5983DataStandard.x - temp_hmc5983Data.x) > 150 || abs(temp_hmc5983DataStandard.y - temp_hmc5983Data.y) > 150 \
                                                          || abs(temp_hmc5983DataStandard.z - temp_hmc5983Data.z) > 150)  
    
        {   
            if(parkingState.vehicleState == ParkingUnUsed)
            {  
                parkingState.vehicleState = ParkingUsed;
                XBeeParkState(ParkingUsed);               
            }
        } 
        else if(parkingState.vehicleState == ParkingUsed)
        {
            parkingState.vehicleState = ParkingUnUsed;
            XBeeParkState(ParkingUnUsed);  
        }
        cnt++;
        if(cnt > 10)
        {
            cnt = 0;
            XBeeParkState(ParkingUnUsed);
        }
        osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
        return (events ^ XBEE_HMC5983_EVT) ;
    }
    
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //�������յ���xbee����,������ϣ����XBeeUartRec.num
    { 
        //uint16 CmdState;
        uint8 FrameTypeState;
        static XBeeUartRecDataDef temp_rbuf;
        
        temp_rbuf = XBeeUartRec;
        if(temp_rbuf.num==0)
        {
            UART_XBEE_EN; 
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        }
        FrameTypeState = temp_rbuf.data[3];
        switch(FrameTypeState)
        {
            case 0x90:  //�����յ���RF��
                if(temp_rbuf.data[15]=='C' && temp_rbuf.data[16]=='F' && temp_rbuf.data[17]=='G')
                    CFGProcess((uint8*)&XBeeUartRec.data[18]);
                if(temp_rbuf.data[15]=='C' && temp_rbuf.data[16]=='T' && temp_rbuf.data[17]=='L')
                    CTLProcess((uint8*)&temp_rbuf.data[18]);
                if(temp_rbuf.data[15]=='S' && temp_rbuf.data[16]=='E' && temp_rbuf.data[17]=='N')
                    SENProcess((uint8*)&temp_rbuf.data[18]);
                if(temp_rbuf.data[15]=='O' && temp_rbuf.data[16]=='T' && temp_rbuf.data[17]=='A')
                {}
                if(temp_rbuf.data[15]=='T' && temp_rbuf.data[16]=='S' && temp_rbuf.data[17]=='T')
                {}        
                break;
            case 0x88:  //�����յ���ATָ���ֵ
                if(temp_rbuf.data[5]=='N' && temp_rbuf.data[6]=='J')
                {
                    //���û��ִ��OK���ٴ�ִ��
                }
                if(temp_rbuf.data[5]=='A' && temp_rbuf.data[6]=='I')
                {
                    if(temp_rbuf.data[7]==0 && temp_rbuf.data[8]==0)
                    {
                        FlagJionNet = GetSH;     
                        osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
                        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );           
                    }
                }
                if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='H')
                {
                    if(temp_rbuf.data[7]==0)
                    {
                        uint8 cnt;
                        for(cnt=0;cnt<4;cnt++)
                            XBeeAdr.IEEEadr[cnt] = temp_rbuf.data[8+cnt];
                        FlagJionNet = GetSL;
                        osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
                        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
                    }
                }
                if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='L')
                {
                    if(temp_rbuf.data[7]==0)
                    {
                        uint8 cnt;
                        for(cnt=0;cnt<4;cnt++)
                            XBeeAdr.IEEEadr[4+cnt] = temp_rbuf.data[8+cnt];
                        FlagJionNet = GetMY;
                        osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
                        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
                    }                 
                }
                if(temp_rbuf.data[5]=='M' && temp_rbuf.data[6]=='Y')
                {
                    if(temp_rbuf.data[7]==0)
                    {
                        uint8 cnt;
                        for(cnt=0;cnt<2;cnt++)
                        XBeeAdr.netadr[cnt] = temp_rbuf.data[8+cnt];
                        FlagJionNet = JoinPark;
                        osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
                        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
                    }
                } 
                if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='M')
                {
                    if(temp_rbuf.data[7] == 0)
                        SetSleepMode = SetSP;
                }
                if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='P')
                {
                    if(temp_rbuf.data[7] == 0)
                        SetSleepMode = SetST;
                }
                if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='T')
                {
                    if(temp_rbuf.data[7] == 0)
                    {
                        FlagJionNet = JoinNet;
                        osal_stop_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT);
                        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
                    }
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
        UART_XBEE_EN; 
        return (events ^ XBEE_REC_DATA_PROCESS_EVT) ;
    }  
    
    if( events & XBEE_MOTOO_CTL_EVT )           //����MOTOR����
    {
        LockCurrentStateType MotorCurrentState,LocalLockState;
        
        UART_XBEE_DIS;
        //LocalLockState = LockObjState;
        LocalLockState = lock;
        MotorCurrentState = GetCurrentMotorState();
        switch(LocalLockState)
        {
            case lock:
                MotorLock();
                if(MotorCurrentState == LocalLockState || MotorCurrentState == over_lock)
                {
                    MotorStop();
                    XBeeLockState(ParkLockSuccess);
                    osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
                    //osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
                    UART_XBEE_EN;
                    return ( events ^ XBEE_MOTOO_CTL_EVT );
                }
                break;
            case unlock:
                MotorUnlock();
                if(MotorCurrentState == LocalLockState)
                {
                    MotorStop();
                    XBeeLockState(ParkUnlockSuccess);
                    osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
                    //osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
                    UART_XBEE_EN;
                    return ( events ^ XBEE_MOTOO_CTL_EVT );
                }
                break;
            default:
                break;
        }
        //��ѯ����Ƿ��������������ʱ����λ��ֹͣ�ڵ�ǰλ��
        //ֹͣ���
        //�����ﵱǰλ��
        //����ʧ�ܱ���
        osal_start_timerEx( XBeeTaskID, XBEE_MOTOO_CTL_EVT, 1 );
        return (events ^ XBEE_MOTOO_CTL_EVT) ;
    }
    
    if(events & XBEE_KEEP_LOCK_STATE_EVT )      //������λ��
    {
        
    }
    
    if( events & XBEE_VBT_CHENCK_EVT )          //��ȡ��ǰ��ѹ
    {
        //static uint16 check_times=0;
        //����ѹʮ�Σ�ȡƽ��ֵ
        
        return (events ^ XBEE_VBT_CHENCK_EVT) ;
    }
    
    if( events & XBEE_TEST_EVT )                //����
    {   
#if 0
        LockCurrentStateType i;
        
        i = GetCurrentMotorState();
#endif
#if 1
        uint8 abc;
        
        MotorLock();
        //MotorUnlock();
            abc = GetCurrentMotorState();
            if(abc == unlock )
            if(abc == lock )
            {
                MotorStop();
                MotorStop();
            }
#endif      
#if 0
        MotorUnlock();
        MotorLock();
        MotorStop();
#endif  
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 1 );
        return (events ^ XBEE_TEST_EVT) ;
    }
    
    if( events & XBEE_IDLE_EVT )                //������������ʧ�ܺ���룬�ٴ�������Ҫ����
    { 
        return (events ^ XBEE_IDLE_EVT) ;
    }
    
    return events;
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
            UART_XBEE_DIS;
            osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT ); 
            break;
        default:
            break;
    }
    return;  
}



#endif