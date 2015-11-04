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
#include "c_queue.h"

//#define __TEST


#if defined _XBEE_APP_



uint8 XBeeTaskID;                           // Task ID for internal task/event processing        
XBeeInfoType XBeeInfo;                
ToReadUARTType ToReadUART=ReadHead;         //读取串口状态
ToReadUARTType CtlToReadUART=ReadNone;      //控制读取串口状态
uint8 LcokState;                            //锁状态标志
ParkingStateType parkingState;              //当前车位状态
uint8 SenFlag=0x88;                         //传感器初值
LockCurrentStateType LockObjState;
FlashLockStateType FlashLockState;
uint8 ReadFlashFlag;
CircularQueueType serialBuf;

void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //初始化UART 
    InitUart1();  //初始化串口1
    HalAdcSetReference ( HAL_ADC_REF_AVDD );
    RegisterForKeys( XBeeTaskID );
    creat_circular_queue( &serialBuf);
    //osal_set_event( XBeeTaskID, XBEE_MOTOR_CTL_EVT );
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //触发事件
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
        osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT );
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
        uint8 buff[128];
        uint16 bufLen=0;
        bufLen = read_one_package_f_queue(&serialBuf , buff);
        if(bufLen)
        {
            ProcessSerial(buff);
        }
        osal_start_timerEx( XBeeTaskID , XBEE_REC_DATA_PROCESS_EVT,3);
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
void ProcessSerial(uint8 *temp_rbuf)
{
    switch(temp_rbuf[3])
    {
        case receive_packet:  //处理收到的RF包
            if(temp_rbuf[15]=='C' && temp_rbuf[16]=='F' && temp_rbuf[17]=='G')
                CFGProcess(temp_rbuf+18);
            else if(temp_rbuf[15]=='C' && temp_rbuf[16]=='T' && temp_rbuf[17]=='L')
                CTLProcess(temp_rbuf+18);
            else if(temp_rbuf[15]=='S' && temp_rbuf[16]=='E' && temp_rbuf[17]=='N')
                SENProcess(temp_rbuf+18);
            else if(temp_rbuf[15]=='O' && temp_rbuf[16]=='T' && temp_rbuf[17]=='A')
            {}
            else if(temp_rbuf[15]=='T' && temp_rbuf[16]=='S' && temp_rbuf[17]=='T')
            {}
            break;
        case explicit_rx_indeicator:
            if(temp_rbuf[21]=='C' && temp_rbuf[22]=='F' && temp_rbuf[23]=='G')
                CFGProcess(temp_rbuf+24);
            else if(temp_rbuf[21]=='C' && temp_rbuf[22]=='T' && temp_rbuf[23]=='L')
                CTLProcess(temp_rbuf+24);
            else if(temp_rbuf[21]=='S' && temp_rbuf[22]=='E' && temp_rbuf[23]=='N')
                SENProcess(temp_rbuf+24);
            else if(temp_rbuf[21]=='O' && temp_rbuf[22]=='T' && temp_rbuf[23]=='A')
            {}
            break;
        case at_command_response:  //处理收到的AT指令返回值
            ProcessAT(temp_rbuf);
            break;
        case transmit_status:
            //ProcessTransmitStatus(temp_rbuf);
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
    static uint8 checksum=0;
    uint16 numBytes=0,RecLen=0;
    static uint16 APICmdLen=0;
    static XBeeUartRecDataDef XBeeUartRec;             //串口接收缓存数据 
    
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
            APICmdLen = 0;
            APICmdLen |= (uint16)*(XBeeUartRec.data+2);
            APICmdLen |= (uint16)*(XBeeUartRec.data+1) << 8;
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
            checksum = XBeeApiChecksum(XBeeUartRec.data+3 , APICmdLen);
            if(*(XBeeUartRec.data + APICmdLen+3) != checksum)
                return;
            ToReadUART = ReadHead; 
            write_cqueue(&serialBuf , XBeeUartRec.data , XBeeUartRec.num);
            XBeeUartRec.num = 0;
            //osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT ); 
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