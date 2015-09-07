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
#include "hal_xbee.h"
#include <math.h>
#include <stdlib.h>
#include "bcomdef.h"
#include "osal_snv.h"

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
FlashLockStateType FlashLockState;


void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //��ʼ��UART 
    InitUart1();  //��ʼ������1
    osal_snv_init();
    RegisterForKeys( XBeeTaskID );
    parkingState.vehicleState = ParkingUnUsed;
                    XBeeCloseBuzzer();
                XBeeCloseLED1();
                XBeeCloseLED2();
#if 0
    if(osal_snv_read( BLE_NVID_USER_CFG_STATRT,1, &FlashLockState) == SUCCESS)
        MotorInit(FlashLockState.LockCurrentState);
    else
        MotorInit(unlock);
#endif
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //�����¼�
    //osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    //osal_set_event( XBeeTaskID, XBEE_TEST_EVT );
}

uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;    
    
    if ( events & XBEE_START_DEVICE_EVT )       //��ʼ����,����xbee���߲���
    {
        SetXBeeSleepMode(); //��������ģʽ
        osal_start_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT, 100 );
        return (events ^ XBEE_START_DEVICE_EVT) ;
    }
    if( events & XBEE_JOIN_NET_EVT)             //��������
    {
        JionParkNet();
        osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 2000 );
        return (events ^ XBEE_JOIN_NET_EVT) ;
    }
    if(events & XBEE_HMC5983_EVT)               //������������
    {
        ReportSenser();
        ReportStatePeriod();
        osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
        return (events ^ XBEE_HMC5983_EVT) ;
    }
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //�������յ���xbee����,������ϣ����XBeeUartRec.num
    { 
        //uint16 CmdState;
        
        static XBeeUartRecDataDef temp_rbuf;
        
        temp_rbuf = XBeeUartRec;
        if(temp_rbuf.num==0)
        {
            UART_XBEE_EN; 
            return ( events ^ XBEE_REC_DATA_PROCESS_EVT );
        }
        ProcessSerial(temp_rbuf);
        XBeeUartRec.num=0;
        UART_XBEE_EN; 
        return (events ^ XBEE_REC_DATA_PROCESS_EVT) ;
    }  
    if( events & XBEE_MOTOO_CTL_EVT )           //����MOTOR����
    {
        UART_XBEE_DIS;
        ControlMotor();
        osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
        osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
        osal_set_event( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT );
        //��ѯ����Ƿ��������������ʱ����λ��ֹͣ�ڵ�ǰλ��
        //ֹͣ���
        //�����ﵱǰλ��
        //����ʧ�ܱ���
        UART_XBEE_EN;
        osal_start_timerEx( XBeeTaskID, XBEE_MOTOO_CTL_EVT, 1 );
        return (events ^ XBEE_MOTOO_CTL_EVT) ;
    }
    if(events & XBEE_KEEP_LOCK_STATE_EVT )      //������λ��
    {
        KeepLockState();
        //����������
        osal_start_timerEx( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT, 10 );
    }
    if( events & XBEE_VBT_CHENCK_EVT )          //��ȡ��ǰ��ѹ
    {
        //static uint16 check_times=0;
        //����ѹʮ�Σ�ȡƽ��ֵ
        
        return (events ^ XBEE_VBT_CHENCK_EVT) ;
    }
    if( events & XBEE_TEST_EVT )                //����
    {   
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 100 );
        return (events ^ XBEE_TEST_EVT) ;
    }
    if( events & XBEE_IDLE_EVT )                //������������ʧ�ܺ���룬�ٴ�������Ҫ����
    { 
        return (events ^ XBEE_IDLE_EVT) ;
    }
    return events;
}
/**********************************************************
**brief process serial data
**********************************************************/
void ProcessSerial(XBeeUartRecDataDef temp_rbuf)
{
    uint8 FrameTypeState;
    FrameTypeState = temp_rbuf.data[3];
    switch(FrameTypeState)
    {
        case receive_packet:  //�����յ���RF��
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
        case at_command_response:  //�����յ���ATָ���ֵ
            ProcessAT(temp_rbuf);
            break;
        case modem_status:         //Zigbeeģ��״̬
            ProcessModeStatus(temp_rbuf);
            break;
        case 0x8B:        //�����ͷ���ֵ     
            break;
        case mto_route_request_indcator:
            if(temp_rbuf.data[12]==0 && temp_rbuf.data[13]==0)
            {
            }
            break;
        default:
            break;
    }  
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