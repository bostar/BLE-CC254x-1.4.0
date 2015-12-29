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
LcokStateType LockState;                    //锁状态标志
uint8 SenFlag=0x88;                         //传感器初值
LockCurrentStateType LockObjState;
//FlashLockStateType FlashLockState;
CircularQueueType serialBuf;
eventInfoType eventInfo;

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
#ifdef _WD
    osal_set_event( XBeeTaskID, FEED_DOG );
    wd_init();
#endif
}
uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;
    uint32 reval;
    if( events & FEED_DOG)
    {
#ifdef _WD
        feed_dog();
#endif
        osal_start_timerEx( XBeeTaskID ,FEED_DOG ,50 );
        return (events ^ FEED_DOG);
    }
    if (events & SYS_EVENT_MSG)
    {
        XBeeMsgType *pMsg;
        while((pMsg = (XBeeMsgType*)osal_msg_receive(XBeeTaskID)) != NULL)
        {
            ProcessXBeeMsg(pMsg);
            osal_msg_deallocate((uint8*)pMsg);
            //pMsg = NULL;
        }
        return (events ^ SYS_EVENT_MSG) ;
    }
    if ( events & XBEE_START_DEVICE_EVT )       //start event
    {
        //for(i=0;i<8;i++)
            //XBeeInfo.panID[i] = 0;
        SenFlag=0x88;
        LockState.FinalState = unlock;
        eventInfo.batEn		= 'n';
        eventInfo.lockEn 	= 'n';
        eventInfo.senerEn 	= 'n';
        osal_set_event( XBeeTaskID, XBEE_MOTOR_CTL_EVT );
        osal_set_event( XBeeTaskID, XBEE_CLOSE_BUZZER_EVT );
        osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT );
        osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 500 );
        //osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        return (events ^ XBEE_START_DEVICE_EVT) ;
    }
    if( events & XBEE_MOTOR_CTL_EVT )           //control motor
    {
        if(CheckMotor() == 0) //when 0 the motor works properly
        {
            uint8 state;
            state = ControlMotor(LockState.FinalState);
            if(state == 1)
            {
                eventInfo.lockEn = 'y';
                if(eventInfo.lockEvt == ParkUnlockSuccess)
                {
                    eventInfo.lockEvt = ParkLockSuccess;
                    if(XBeeInfo.InPark == 'y')
                        CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
                    //XBeeReport(eventInfo);
                }
                else
                    eventInfo.lockEvt = ParkLockSuccess;
            }
            else if(state == 2)
            {
                eventInfo.lockEn = 'y';
                if(eventInfo.lockEvt == ParkLockSuccess)
                {
                    eventInfo.lockEvt = ParkUnlockSuccess;
                    if(XBeeInfo.InPark == 'y')
                        CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
                    //XBeeReport(eventInfo);
                }
                else
                    eventInfo.lockEvt = ParkUnlockSuccess;
            }
            reval = 30;
        }
        else
        {
            MotorStop();
            if(LockState.FinalState == lock)
            {
                eventInfo.lockEn = 'y';
                eventInfo.lockEvt = ParkLockFailed;
                CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
                //XBeeReport(eventInfo);
            }
            else if(LockState.FinalState == unlock)
            {
                eventInfo.lockEn = 'y';
                eventInfo.lockEvt = ParkUnlockFailed;
                CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
                //XBeeReport(eventInfo);
            }
            reval = 2000;
        }
        osal_start_timerEx( XBeeTaskID, XBEE_MOTOR_CTL_EVT, reval );
        return (events ^ XBEE_MOTOR_CTL_EVT);
    }
    if(events & XBEE_HMC5983_EVT)               //process senser data
    {
        ReadSenser();
        osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
        return (events ^ XBEE_HMC5983_EVT) ;
    }
    if(events & XBEE_REPORT_EVT)               //report event
    {
        XBeeReport(eventInfo);
        eventInfo.batEn		= 'n';
        eventInfo.lockEn	= 'n';
        eventInfo.senerEn	= 'n';
        osal_start_timerEx( XBeeTaskID , XBEE_REPORT_EVT,5000);
        return (events ^ XBEE_REPORT_EVT) ;
    }
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //process the data by xbee send
    {
        static uint8 buff[128];
        uint16 bufLen=0;
        bufLen = read_one_package_f_queue(&serialBuf , buff);
        if(bufLen)
        {
            ProcessSerial(buff);
        }
        osal_start_timerEx( XBeeTaskID , XBEE_REC_DATA_PROCESS_EVT,10);
        return (events ^ XBEE_REC_DATA_PROCESS_EVT);
    }
    if( events & XBEE_VBT_CHENCK_EVT )          //report the battery voltage
    {
        float vbat;
        vbat = ReportVbat();
        if(vbat > 0)
            eventInfo.batEvt = (uint8)(vbat*10);
        osal_start_timerEx( XBeeTaskID, XBEE_VBT_CHENCK_EVT, 1000 );
        return (events ^ XBEE_VBT_CHENCK_EVT) ;
    }
    if( events & XBEE_JOIN_NET_EVT)             //join the park net
    {
        uint32 time_delay;
        if(XBeeInfo.InPark != 'y')
        {
            time_delay = SleepModeAndJoinNet();
            osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, time_delay );
        }
        return (events ^ XBEE_JOIN_NET_EVT) ;
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
**brief process XBeeTaskID SYS_EVENT_MSG
**********************************************************/
void ProcessXBeeMsg(XBeeMsgType *pMsg)
{
    switch(pMsg->event)
    {
        case XBEE_MOTOR_CTL_EVT:
            if(pMsg->operation == MOTOR_REVERSE)
            {
                if(LockState.FinalState == unlock)
                    LockState.FinalState = lock;
                else
                    LockState.FinalState = unlock;
            }
            else if(pMsg->operation == MOTOR_LOCK)
            {
                LockState.FinalState = lock;
            }
            else if(pMsg->operation == MOTOR_UNLOCK)
            {
                LockState.FinalState = unlock;
            }
            break;
        case XBEE_JOIN_NET_EVT:
            if(pMsg->operation == ACTIVATE)
                osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
            else if(pMsg->operation == INACTIVATE)
                osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
            break;
        case XBEE_REC_DATA_PROCESS_EVT:
            if(pMsg->operation == ACTIVATE)
                osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT );
            else if(pMsg->operation == INACTIVATE)
                osal_stop_timerEx( XBeeTaskID,XBEE_REC_DATA_PROCESS_EVT);
            break;
        case XBEE_HMC5983_EVT:
            if(pMsg->operation == ACTIVATE)
                osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
            else if(pMsg->operation == INACTIVATE)
                osal_stop_timerEx( XBeeTaskID,XBEE_HMC5983_EVT);
            break;
        case XBEE_VBT_CHENCK_EVT:
            if(pMsg->operation == ACTIVATE)
                osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
            else if(pMsg->operation == INACTIVATE)
                osal_stop_timerEx( XBeeTaskID,XBEE_VBT_CHENCK_EVT);
            break;
        case XBEE_REPORT_EVT:
            if(pMsg->operation == ACTIVATE)
                osal_set_event( XBeeTaskID, XBEE_REPORT_EVT );
            else if(pMsg->operation == INACTIVATE)
                osal_stop_timerEx( XBeeTaskID,XBEE_REPORT_EVT);
            break;
        case XBEE_CLOSE_BUZZER_EVT:
            if(pMsg->operation == ACTIVATE)
                osal_set_event( XBeeTaskID, XBEE_CLOSE_BUZZER_EVT );
            else if(pMsg->operation == INACTIVATE)
                osal_stop_timerEx( XBeeTaskID,XBEE_CLOSE_BUZZER_EVT);
            break;
        default:
            break;
    }
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
            if(XBeeInfo.InPark != 'y')
                ProcessJoinRes(temp_rbuf);
            break;
        case transmit_status:
            ProcessTransmitStatus(temp_rbuf);
            break;
        case modem_status:         //Zigbee模块状态
            ProcessModeStatus(temp_rbuf);
            break;
        default:
            break;
    }
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
/************************************************************
**brief
************************************************************/
uint8 CreatXBeeMsg(uint16 event,uint8 state)
{
    XBeeMsgType *pMsg;
    pMsg = (XBeeMsgType*)osal_msg_allocate(sizeof(XBeeMsgType));
    pMsg->event = event;
    pMsg->operation = state;
    return osal_msg_send(XBeeTaskID , (uint8*)pMsg );
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
/**********************************************************
**brief 读取xbee发送到串口数据
**********************************************************/
static void npiCBack_uart( uint8 port, uint8 events )
{
    uint8 checksum=0;
    static uint16 APICmdLen=0;
    static uint8 XBeeUartRec[128];
    static ToReadUARTType ToReadUART=ReadHead;         //读取串口状态
    
    if(NPI_RxBufLen()>0 && ToReadUART==ReadHead && NPI_ReadTransport( XBeeUartRec, 1)==1)
    {
        if(*XBeeUartRec == 0x7E)
            ToReadUART = ReadLen;
    }
    if(NPI_RxBufLen()>=2 && ToReadUART==ReadLen && NPI_ReadTransport((XBeeUartRec+1), 2)==2)
    {
        APICmdLen = 0;
        APICmdLen |= (uint16)*(XBeeUartRec+2);
        APICmdLen |= (uint16)*(XBeeUartRec+1) << 8;
        ToReadUART = ReadData;
    }
    if(NPI_RxBufLen()>=APICmdLen+1 && ToReadUART==ReadData \
        && NPI_ReadTransport((XBeeUartRec+3),APICmdLen+1)==APICmdLen+1)
    {
        checksum = XBeeApiChecksum(XBeeUartRec+3 , APICmdLen);
        if(*(XBeeUartRec + APICmdLen+3) == checksum)
        {
            ToReadUART = ReadHead;
            write_cqueue(&serialBuf , XBeeUartRec , APICmdLen+4);
            //osal_set_event( XBeeTaskID, XBEE_REC_DATA_PROCESS_EVT );
            CreatXBeeMsg(XBEE_REC_DATA_PROCESS_EVT,ACTIVATE);
        }
    }
    return;
}











#endif