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
#include <stdio.h>
#include "hal_adc.h"
#include <string.h>
#include "hal_adc.h"
#include "XBeeProtocol.h"

//#define __TEST


#if defined _XBEE_APP_



uint8 XBeeTaskID;                           // Task ID for internal task/event processing       
XBeeUartRecDataDef XBeeUartRec;             //���ڽ��ջ�������  
XBeeInfoType XBeeInfo;                
ToReadUARTType ToReadUART=ReadHead;         //��ȡ����״̬
ToReadUARTType CtlToReadUART=ReadNone;      //���ƶ�ȡ����״̬
uint8 XBeeUartEn=0;                         //���ڶ�ȡʹ��
uint8 LcokState;                            //��״̬��־
ParkingStateType parkingState;              //��ǰ��λ״̬
uint8 SenFlag=0x88;                         //��������ֵ
LockCurrentStateType LockObjState;
FlashLockStateType FlashLockState;
uint8 ReadFlashFlag;
uint32 BuzzerTime=200;

void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //��ʼ��UART 
    InitUart1();  //��ʼ������1
    HalAdcSetReference ( HAL_ADC_REF_AVDD );
    RegisterForKeys( XBeeTaskID );
    //osal_set_event( XBeeTaskID, XBEE_MOTOR_CTL_EVT );
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //�����¼�
    //osal_set_event( XBeeTaskID, XBEE_SCAN_ROUTE_PATH );  
    //osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    //osal_set_event( XBeeTaskID, XBEE_TEST_EVT );
}
uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;    
    uint8 i;
    uint32 reval;
    if ( events & XBEE_START_DEVICE_EVT )       //start event
    {
        for(i=0;i<8;i++)
            XBeeInfo.panID[i] = 0;
        SenFlag=0x88;
        parkingState.vehicleState = ParkingUnUsed;
        LockObjState = unlock;
        //XBeeReset();
        osal_set_event( XBeeTaskID, XBEE_MOTOR_CTL_EVT );   
        osal_set_event( XBeeTaskID, XBEE_CLOSE_BUZZER_EVT );
        osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 500 );
        //osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        return (events ^ XBEE_START_DEVICE_EVT) ;
    }
    if( events & XBEE_JOIN_NET_EVT)             //join the park net
    {
        uint32 time_delay;
        if(XBeeInfo.InPark != 1)
        {
            time_delay = SleepModeAndJoinNet();
            osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, time_delay );
        }
        return (events ^ XBEE_JOIN_NET_EVT) ;
    }
    if( events & XBEE_MOTOR_CTL_EVT )           //control motor
    {
        if(CheckMotor() == 0) //when 0 the motor works properly
        {
            ControlMotor();
            reval = 10;
        }
        else
        {
            MotorStop();
            if(LockObjState == lock)
                XBeeLockState(ParkLockFailed);
            else if(LockObjState == unlock)
                XBeeLockState(ParkLockFailed);
            reval = 4000;
        }
        osal_start_timerEx( XBeeTaskID, XBEE_MOTOR_CTL_EVT, reval );
        return (events ^ XBEE_MOTOR_CTL_EVT);
    }
    if(events & XBEE_HMC5983_EVT)               //process senser data
    {
        ReportSenser();
        hmc5983Data.state = 1;
        osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
        return (events ^ XBEE_HMC5983_EVT) ;
    }
    if(events & XBEE_REPORT_EVT)               //report event
    {
        ReportLockState();
        osal_start_timerEx( XBeeTaskID , XBEE_REPORT_EVT,5000);
        return (events ^ XBEE_REPORT_EVT) ;
    }
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //process the data by xbee send
    {
        if(XBeeUartRec.num != 0)
            ProcessSerial(XBeeUartRec);
        XBeeUartRec.num=0;
        UART_XBEE_EN;
        return (events ^ XBEE_REC_DATA_PROCESS_EVT);
    }
    if( events & XBEE_VBT_CHENCK_EVT )          //report the battery voltage
    {
        //ReportVbat();
        osal_start_timerEx( XBeeTaskID, XBEE_VBT_CHENCK_EVT, 1000 );
        return (events ^ XBEE_VBT_CHENCK_EVT) ;
    }
    if(events & XBEE_CLOSE_BUZZER_EVT)
    {
        if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
        {
            XBeeCloseBuzzer();
        }
        osal_start_timerEx( XBeeTaskID, XBEE_CLOSE_BUZZER_EVT, 500 );
        return (events ^ XBEE_CLOSE_BUZZER_EVT) ;
    }
    if( events & XBEE_TEST_EVT )                //test event
    {   
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 1000 );
        return (events ^ XBEE_TEST_EVT) ;
    }
    return events;
}
/**********************************************************
**brief check motor current,if current too large,stop motor
**********************************************************/
uint8 CheckMotor(void)
{
    static volatile float sen_v_0=0,sen_v_1=0;
    static volatile uint8 cnt=0;
    static uint8 cnt_state=9;
    uint8 reval=0;

    if(cnt > cnt_state)
    {
        cnt = 0;
        sen_v_0 = sen_v_1 =0;
    }
    if(cnt == 0)
        sen_v_0 = ReadMotorSen();
    else if(cnt == cnt_state)
        sen_v_1 = ReadMotorSen();
    cnt++;
    if(sen_v_0 > SEN_MOTOR && sen_v_1 > SEN_MOTOR)
    {
        reval = 1;
    }
    return reval;
}
/**********************************************************
**brief process serial data
**********************************************************/
void ProcessSerial(XBeeUartRecDataDef temp_rbuf)
{
    switch(temp_rbuf.data[3])
    {
        case receive_packet:  //�����յ���RF��
            if(temp_rbuf.data[15]=='C' && temp_rbuf.data[16]=='F' && temp_rbuf.data[17]=='G')
                CFGProcess((uint8*)&XBeeUartRec.data[18]);
            else if(temp_rbuf.data[15]=='C' && temp_rbuf.data[16]=='T' && temp_rbuf.data[17]=='L')
                CTLProcess((uint8*)&temp_rbuf.data[18]);
            else if(temp_rbuf.data[15]=='S' && temp_rbuf.data[16]=='E' && temp_rbuf.data[17]=='N')
                SENProcess((uint8*)&temp_rbuf.data[18]);
            else if(temp_rbuf.data[15]=='O' && temp_rbuf.data[16]=='T' && temp_rbuf.data[17]=='A')
            {}
            else if(temp_rbuf.data[15]=='T' && temp_rbuf.data[16]=='S' && temp_rbuf.data[17]=='T')
                TSTProcess(temp_rbuf);
            break;
        case explicit_rx_indeicator:
            if(temp_rbuf.data[21]=='C' && temp_rbuf.data[22]=='F' && temp_rbuf.data[23]=='G')
                CFGProcess((uint8*)&XBeeUartRec.data[24]);
            else if(temp_rbuf.data[21]=='C' && temp_rbuf.data[22]=='T' && temp_rbuf.data[23]=='L')
                CTLProcess((uint8*)&temp_rbuf.data[24]);
            else if(temp_rbuf.data[21]=='S' && temp_rbuf.data[22]=='E' && temp_rbuf.data[23]=='N')
                SENProcess((uint8*)&temp_rbuf.data[24]);
            else if(temp_rbuf.data[21]=='O' && temp_rbuf.data[22]=='T' && temp_rbuf.data[23]=='A')
            {}
            break;
        case at_command_response:  //�����յ���ATָ���ֵ
            ProcessAT(temp_rbuf);
            break;
        case transmit_status:
            //ProcessTransmitStatus(temp_rbuf);
            break;
        case modem_status:         //Zigbeeģ��״̬
            ProcessModeStatus(temp_rbuf);
            break;
        case mto_route_request_indcator:
            break;
        case route_record_indicator:
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
/********************************************************
**brief daily event
********************************************************/
void DailyEvt(void)
{
#if !defined _TEST_LARGE_MODES
    osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
    osal_set_event( XBeeTaskID, XBEE_REPORT_EVT );
#endif 
    osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
    osal_stop_timerEx( XBeeTaskID, XBEE_CLOSE_BUZZER_EVT );
}
/************************************************************
**brief read motor_sen 
************************************************************/
float ReadMotorSen(void)
{
    int16 sen=0;
    
    sen = HalAdcRead (HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_8);
    return (3.482 * (float)sen / 0x7f);    
}









#endif