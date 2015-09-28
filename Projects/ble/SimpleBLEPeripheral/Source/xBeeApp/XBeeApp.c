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
XBeeUartRecDataDef XBeeUartRec;             //串口接收缓存数据  
XBeeInfoType XBeeInfo;                
ToReadUARTType ToReadUART=ReadHead;         //读取串口状态
ToReadUARTType CtlToReadUART=ReadNone;      //控制读取串口状态
uint8 XBeeUartEn=0;                         //串口读取使能
uint8 LcokState;                            //锁状态标志
ParkingStateType parkingState;              //当前车位状态
uint8 SenFlag=0x88;                         //传感器初值
LockCurrentStateType LockObjState;
FlashLockStateType FlashLockState;
uint8 ReadFlashFlag;
DeviceType DevType=notype;
uint32 BuzzerTime=200;

void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //初始化UART 
    InitUart1();  //初始化串口1
    HalAdcSetReference ( HAL_ADC_REF_AVDD );
    RegisterForKeys( XBeeTaskID );
    parkingState.vehicleState = ParkingUnUsed;
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //触发事件
    //osal_set_event( XBeeTaskID, XBEE_SCAN_ROUTE_PATH );  
    //osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    //osal_set_event( XBeeTaskID, XBEE_TEST_EVT );
}

uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;    
    
    if ( events & XBEE_START_DEVICE_EVT )       //起始任务
    {
        SenFlag=0x88;
        //osal_set_event( XBeeTaskID, XBEE_BUZZER_CTL );
        osal_set_event( XBeeTaskID, XBEE_MOTOR_CTL_EVT );
        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        return (events ^ XBEE_START_DEVICE_EVT) ;
    }
    if( events & XBEE_JOIN_NET_EVT)             //加入网络、设置休眠
    {
        uint32 time_delay;
        if(XBeeInfo.InPark != 1)
        {
            time_delay = SleepModeAndJoinNet();
            osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, time_delay );
        }
        return (events ^ XBEE_JOIN_NET_EVT) ;
    }
    if(events & XBEE_HMC5983_EVT)               //处理传感器数据
    {
        ReportSenser();
        ReportStatePeriod();
        hmc5983Data.state = 1;
        osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
        return (events ^ XBEE_HMC5983_EVT) ;
    }
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //处理串口收到的xbee数据,处理完毕，清除XBeeUartRec.num
    {
        if(XBeeUartRec.num != 0)
            ProcessSerial(XBeeUartRec);
        XBeeUartRec.num=0;
        UART_XBEE_EN;
        return (events ^ XBEE_REC_DATA_PROCESS_EVT);
    }
    if( events & XBEE_MOTOR_CTL_EVT )           //控制MOTOR动作
    {
        static float sen_v_0=0,sen_v_1=0;
        static uint8 cnt=0,cnt_state=11;
        osal_stop_timerEx( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT );
        cnt++;
        if(cnt == 1)
            sen_v_0 = ReadMotorSen();
        else if(cnt == cnt_state)
            sen_v_1 = ReadMotorSen();
        if(sen_v_0 > SEN_MOTOR && sen_v_1 > SEN_MOTOR)
        {
            MotorStop();
            if(LockObjState == lock)
                XBeeLockState(ParkLockFailed);
            else if(LockObjState == unlock)
                XBeeLockState(ParkLockFailed);
            osal_start_timerEx( XBeeTaskID, XBEE_MOTOR_CTL_EVT, 5000 );
        }
        else
        {
            if(ControlMotor() == 1 || ControlMotor() == 2)
            {
                if(ControlMotor() == 1)
                    XBeeLockState(ParkLockSuccess);
                else
                    XBeeLockState(ParkUnlockSuccess);
                osal_set_event( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT );
            }
            else
                osal_start_timerEx( XBeeTaskID, XBEE_MOTOR_CTL_EVT, 10 );
        }
        if(cnt == cnt_state)
        {
            cnt = 0;
            sen_v_0 = sen_v_1 =0;
        }
        return (events ^ XBEE_MOTOR_CTL_EVT) ;
    }
    if(events & XBEE_KEEP_LOCK_STATE_EVT )      //保持锁位置
    {
        static float sen_v_0=0,sen_v_1=0;
        static uint8 cnt=0,cnt_state=9;
        cnt++;
        if(cnt == 1)
            sen_v_0 = ReadMotorSen();
        else if(cnt == cnt_state)
            sen_v_1 = ReadMotorSen();
        if(sen_v_0 > SEN_MOTOR && sen_v_1 > SEN_MOTOR)
        {
            MotorStop();
            osal_start_timerEx( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT, 5000 );
        }
        else
        {
            KeepLockState();
            osal_start_timerEx( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT, 10 );
        }
        if(cnt == cnt_state)
        {
            cnt = 0;
            sen_v_0 = sen_v_1 =0;
        }
        return (events ^ XBEE_KEEP_LOCK_STATE_EVT);
    }
    if( events & XBEE_VBT_CHENCK_EVT )          //读取当前电压
    {
        ReportVbat();       //每执行10次上报一次电压
        osal_start_timerEx( XBeeTaskID, XBEE_VBT_CHENCK_EVT, 1000 );
        return (events ^ XBEE_VBT_CHENCK_EVT) ;
    }
    if( events & XBEE_TEST_EVT )                //测试
    {   static uint8 asd=0;
        if(asd == 0)
        {
            asd++;
            XBeeSetNO(2,NO_RES);
            XBeeReadAT("ND");
        }
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 1000 );
        return (events ^ XBEE_TEST_EVT) ;
    }
    if( events & XBEE_BUZZER_CTL )
    {
        static  uint8 bzzer_fr=0;
        if(bzzer_fr == 0)
        {
            bzzer_fr = 1;
            XBeeOpenBuzzer();
        }
        else if(bzzer_fr == 1)
        {
            bzzer_fr = 0;
            XBeeCloseBuzzer();
        }
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, BuzzerTime );
        return (events ^ XBEE_BUZZER_CTL) ;
    }
    return events;
}
/**********************************************************
**brief process serial data
**********************************************************/
void ProcessSerial(XBeeUartRecDataDef temp_rbuf)
{
    switch(temp_rbuf.data[3])
    {
        case receive_packet:  //处理收到的RF包
            if(temp_rbuf.data[15]=='C' && temp_rbuf.data[16]=='F' && temp_rbuf.data[17]=='G')
                CFGProcess((uint8*)&XBeeUartRec.data[18]);
            else if(temp_rbuf.data[15]=='C' && temp_rbuf.data[16]=='T' && temp_rbuf.data[17]=='L')
                CTLProcess((uint8*)&temp_rbuf.data[18]);
            else if(temp_rbuf.data[15]=='S' && temp_rbuf.data[16]=='E' && temp_rbuf.data[17]=='N')
                SENProcess((uint8*)&temp_rbuf.data[18]);
            else if(temp_rbuf.data[15]=='O' && temp_rbuf.data[16]=='T' && temp_rbuf.data[17]=='A')
            {}
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
        case at_command_response:  //处理收到的AT指令返回值
            ProcessAT(temp_rbuf);
            break;
        case transmit_status:
            ProcessTransmitStatus(temp_rbuf);
            break;
        case modem_status:         //Zigbee模块状态
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
**brief 读取xbee发送到串口数据
**********************************************************/
static void npiCBack_uart( uint8 port, uint8 events )
{
    uint16 rev_data_temp;
    uint8 cnt;
    static uint8 checksum=0;
    uint16 numBytes=0,RecLen=0;
    static uint16 APICmdLen=0;

    if(XBeeUartEn == 1)  //默认值为0 使能
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
    osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
    osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
    osal_stop_timerEx( XBeeTaskID,XBEE_START_DEVICE_EVT);
    osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
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