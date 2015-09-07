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
__xdata XBeeUartRecDataDef XBeeUartRec;     //串口接收缓存数据  
__xdata FlagJionNetType FlagJionNet;        //加入网络标志
__xdata uint8 FlagPowerON=0;                //启动标志
//static uint8 FlagXBeeTrans=0;             //xbee数据发送状态
__xdata uint8 SendTimes;                    //命令发送次数
__xdata XBeeAdrType XBeeAdr;                //IEEE地址和当前的网络地址
TaskSendType TaskSend;                      //数据发送次数
ToReadUARTType ToReadUART=ReadHead;         //读取串口状态
ToReadUARTType CtlToReadUART=ReadNone;      //控制读取串口状态
uint8 XBeeUartEn=0;                            //串口读取使能
uint8 LcokState;                            //锁状态标志
DeviceTypeDef DeviceType;                   //当前设备类型
ParkingStateType parkingState;              //当前车位状态
uint8 XBeeSOW;                              //xbee休眠标志
uint8 SenFlag=0x88;                              //传感器初值标志
uint8 test123;
LockCurrentStateType LockObjState;
SetSleepModeType SetSleepMode;                       //
FlashLockStateType FlashLockState;


void XBeeInit( uint8 task_id )
{
    XBeeTaskID = task_id;
    NPI_InitTransport(npiCBack_uart);         //初始化UART 
    InitUart1();  //初始化串口1
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
    osal_set_event( XBeeTaskID, XBEE_START_DEVICE_EVT );  //触发事件
    //osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
    //osal_set_event( XBeeTaskID, XBEE_TEST_EVT );
}

uint16 XBeeProcessEvent( uint8 task_id, uint16 events )
{
    VOID  task_id;    
    
    if ( events & XBEE_START_DEVICE_EVT )       //起始任务,设置xbee休眠参数
    {
        SetXBeeSleepMode(); //设置休眠模式
        osal_start_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT, 100 );
        return (events ^ XBEE_START_DEVICE_EVT) ;
    }
    if( events & XBEE_JOIN_NET_EVT)             //加入网络
    {
        JionParkNet();
        osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 2000 );
        return (events ^ XBEE_JOIN_NET_EVT) ;
    }
    if(events & XBEE_HMC5983_EVT)               //处理传感器数据
    {
        ReportSenser();
        ReportStatePeriod();
        osal_start_timerEx( XBeeTaskID , XBEE_HMC5983_EVT,1000);
        return (events ^ XBEE_HMC5983_EVT) ;
    }
    if( events & XBEE_REC_DATA_PROCESS_EVT )    //处理串口收到的xbee数据,处理完毕，清除XBeeUartRec.num
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
    if( events & XBEE_MOTOO_CTL_EVT )           //控制MOTOR动作
    {
        UART_XBEE_DIS;
        ControlMotor();
        osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
        osal_set_event( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
        osal_set_event( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT );
        //轮询马达是否阻塞，马达阻塞时，归位或停止在当前位置
        //停止马达
        //检查马达当前位置
        //发送失败报告
        UART_XBEE_EN;
        osal_start_timerEx( XBeeTaskID, XBEE_MOTOO_CTL_EVT, 1 );
        return (events ^ XBEE_MOTOO_CTL_EVT) ;
    }
    if(events & XBEE_KEEP_LOCK_STATE_EVT )      //保持锁位置
    {
        KeepLockState();
        //监测马达阻塞
        osal_start_timerEx( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT, 10 );
    }
    if( events & XBEE_VBT_CHENCK_EVT )          //读取当前电压
    {
        //static uint16 check_times=0;
        //检测电压十次，取平均值
        
        return (events ^ XBEE_VBT_CHENCK_EVT) ;
    }
    if( events & XBEE_TEST_EVT )                //测试
    {   
        osal_start_timerEx( XBeeTaskID, XBEE_TEST_EVT, 100 );
        return (events ^ XBEE_TEST_EVT) ;
    }
    if( events & XBEE_IDLE_EVT )                //空闲任务，入网失败后进入，再次入网需要重启
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
        case receive_packet:  //处理收到的RF包
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
        case at_command_response:  //处理收到的AT指令返回值
            ProcessAT(temp_rbuf);
            break;
        case modem_status:         //Zigbee模块状态
            ProcessModeStatus(temp_rbuf);
            break;
        case 0x8B:        //处理发送返回值     
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



#endif