#include "XBeeProtocol.h"
#include "npi.h"
#include <string.h>
#include "XBeeAtCmd.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "XBeeApp.h"
#include "XBeeBsp.h"
#include "XBeeApp.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "hal_adc.h"
#if defined _XBEE_APP_
/*****************************************************
**
*****************************************************/
uint16 CFGProcess(uint8 *rf_data)
{
    switch(*rf_data)
    {
        case 0x00:      //设置限时加入网络时限,end device禁止该功能
            if(DevType == router)
                XBeeSetNJ(*(rf_data+1),NO_RES);
            break;
        case 0x02:      //入网响应
            if(*(rf_data+1) == 0x01)   //允许入网
            {
                FlagJionNet = NetOK;
                XBeeCloseLED1();
                XBeeCloseLED2();
                XBeeSetSP(0x64,RES);
                XBeeSetST(0x64,RES);
                XBeeSendAT("AC");
                XBeeSendAT("WR");
                DailyEvt();
                XBeeReadAT("OP");
            }
            else if(*(rf_data+1) == 0x00)   //禁止入网
            {
                XBeeLeaveNet();
                FlagJionNet = JoinNet;
                osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 5000 );
            }
            break;
        case 0x03:  //恢复出厂设置
            XBeeLeaveNet();  
            FlagJionNet = NetOK;
            break;
        default:
            break;
    }
    return 0;
}
/*****************************************************
**
*****************************************************/
void CTLProcess(uint8 *rf_data)
{
    switch(*rf_data)
    {
        case 0:
            if(*(rf_data+1) == 0)  //解锁
                LockObjState = unlock;
            else if(*(rf_data+1) == 1)  //上锁
                LockObjState = lock;
            osal_set_event( XBeeTaskID, XBEE_SAVE_FLASH_EVT );
            osal_set_event( XBeeTaskID, XBEE_MOTOR_CTL_EVT );
            break;
        default:
            break;
    }
}
/**************************************************
**brief 处理SEN指令
**************************************************/
void SENProcess(uint8 *rf_data)
{
    switch(*rf_data)
    {
        case 0x00:
            SenFlag = 0x88;
            break;
        default:
            break;
    }
}
void OTAProcess(uint8 *rf_data)
{}
/*********************************************************
**
*********************************************************/
void ProcessAT(XBeeUartRecDataDef temp_rbuf)
{
    uint8 i;
    if(temp_rbuf.data[5]=='A' && temp_rbuf.data[6]=='I')
    {   
        if(temp_rbuf.data[7]==0 && temp_rbuf.data[8]==0)
        {
            FlagJionNet = GetMY;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='H')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
                XBeeInfo.IEEEadr[cnt] = temp_rbuf.data[8+cnt];
            SetSleepMode = GetSL;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='L')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
                XBeeInfo.IEEEadr[4+cnt] = temp_rbuf.data[8+cnt];
            SetSleepMode = SendND;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
        }                 
    }
    else if(temp_rbuf.data[5]=='N' && temp_rbuf.data[6]=='D')
    {
        uint8 mac_adr[8];
        static uint8 read_nd = 0;
        connectarr(temp_rbuf.data+10,temp_rbuf.data+14,mac_adr);
        if(arrncmp(mac_adr,XBeeInfo.IEEEadr,8) == 0 && read_nd == 0)
        {
            read_nd++;
            XBeeInfo.DevType = temp_rbuf.data[22];
            SetSleepMode = SetSleep;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        }
    }
    else if(temp_rbuf.data[5]=='M' && temp_rbuf.data[6]=='Y')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<2;cnt++)
                XBeeInfo.netadr[cnt] = temp_rbuf.data[8+cnt];
            FlagJionNet = JoinPark;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='M')
    {
        if(temp_rbuf.data[7] == 0 || XBeeInfo.DevType == router)
        {
            FlagJionNet = SetOK;
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT ); 
        }
    }
    else if(temp_rbuf.data[5]=='O' && temp_rbuf.data[6]=='P')
    {
        if(temp_rbuf.data[7] == 0)
        {
            for(i=0;i<8;i++)
            {
                FlashLockState.panID[i] = temp_rbuf.data[8+i];
            }
            osal_set_event( XBeeTaskID, XBEE_SAVE_FLASH_EVT );
        }
        else
            XBeeReadAT("OP");
    }
}
/*********************************************************
**brief mode status process
*********************************************************/
void ProcessModeStatus(XBeeUartRecDataDef temp_rbuf)
{
    if(temp_rbuf.data[4] == 0x03)
    {
        FlagJionNet = JoinNet;
        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        osal_stop_timerEx( XBeeTaskID, XBEE_KEEP_LOCK_STATE_EVT );
        osal_stop_timerEx( XBeeTaskID, XBEE_HMC5983_EVT );
        osal_stop_timerEx( XBeeTaskID, XBEE_VBT_CHENCK_EVT );
    }
}
/*********************************************************
**brief 发送锁状态函数
*********************************************************/
uint16 XBeeLockState(parkingEventType LockState)
{
  uint8 data[5];
  
  data[0]   =   'S';
  data[1]   =   'E';
  data[2]   =   'N';
  data[3]   =  0x01;
  data[4]   =  LockState;
  
  return XBeeSendToCoor(data,5,NO_RES);
}
/*********************************************************
**brief 发送车位状态
*********************************************************/
uint16 XBeeParkState(parkingEventType ParkState)
{
  uint8 data[5];
  
  data[0]   =   'S';
  data[1]   =   'E';
  data[2]   =   'N';
  data[3]   =  0x01;
  data[4]   =  ParkState;
  
  return XBeeSendToCoor(data,5,RES);
  //return XBeeSendToCoor(data,5,NO_RES);
}
/*********************************************************
**brief 发送电池电量
*********************************************************/
uint16 XBeeBatPower(uint8 PowerVal)
{
  uint8 data[5];
  data[0]   =   'S';
  data[1]   =   'E';
  data[2]   =   'N';
  data[3]   =  0x02;
  data[4]   =  PowerVal;
  return XBeeSendToCoor(data,5,NO_RES);
}
/**************************************************
**brief set xbee sleep mode
**************************************************/
uint8 SetXBeeSleepMode(void)
{
    uint8 reval=0;
    static uint8 ND_s=0;
    switch(SetSleepMode)
    {   
        case GetSH:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                XBeeSendAT("RE");
                XBeeSetST(0xfff0,NO_RES);
                XBeeCloseBuzzer();
                XBeeSetNT(0x20,NO_RES);
                XBeeSetNO(2,NO_RES);
                XBeeReadSH();   
            }
            break;
        case GetSL:
            XBeeReadSL();
            break;
        case SendND:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1 /*&& ND_s == 0*/)  //high--wake  low--sleep
            {
                XBeeReadAT("ND");
                ND_s++;
            }
            break;
        case SetSleep:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                if(XBeeInfo.DevType == end_dev)
                    XBeeSetSM(PinCyc,NO_RES);
                reval = 1;
            }
            break;
        default:
            break;
        }
    return reval;
}
/**********************************************************
**brief jion park net
**********************************************************/
uint8 JionParkNet(void)
{
    static uint8 join_net=0;
    uint8 reval=1;
    if(join_net == 0)
        XBeeRourerJoinNet();
    join_net++;
    switch(FlagJionNet)
    {
        case JoinNet:
            XBeeReadAT("AI");
            reval = 20;
            break;
        case GetMY:
            XBeeReadAT("MY");
            reval = 1;
            break;
        case JoinPark:
            XBeeReqJionPark();
            reval = 40;
            break;  
        default:
            break;
    }
    return reval;
}
/**********************************************************
**brief report senser data
**********************************************************/
uint16 ReportSenser(void)
{
    HMC5983DataType temp_hmc5983Data,temp_hmc5983DataStandard;
 
    temp_hmc5983Data = hmc5983Data;
    temp_hmc5983DataStandard = hmc5983DataStandard;
    if(temp_hmc5983Data.state!=0x88)
    {
        Uart1_Send_Byte("get",osal_strlen("get"));
        return 0;
    }
    if( abs(temp_hmc5983DataStandard.x - temp_hmc5983Data.x) > OFFSET \
        || abs(temp_hmc5983DataStandard.y - temp_hmc5983Data.y) > OFFSET \
        || abs(temp_hmc5983DataStandard.z - temp_hmc5983Data.z) > OFFSET)  
    {   
        if(parkingState.vehicleState == ParkingUnUsed)
        {  
            parkingState.vehicleState = ParkingUsed;
            return XBeeParkState(ParkingUsed);               
        }
    } 
    else if(parkingState.vehicleState == ParkingUsed)
    {
        parkingState.vehicleState = ParkingUnUsed;
        return XBeeParkState(ParkingUnUsed);  
    }
    return 0;
}
/**********************************************************
**brief report park state periodly
**********************************************************/
uint16 ReportStatePeriod(void)
{
    static uint8 cnt=0;
    HMC5983DataType temp_hmc5983Data,temp_hmc5983DataStandard;
 
    temp_hmc5983Data = hmc5983Data;
    temp_hmc5983DataStandard = hmc5983DataStandard;
    cnt++;
    if(cnt > 9)
    {
        cnt = 0;
        if( abs(temp_hmc5983DataStandard.x - temp_hmc5983Data.x) > OFFSET \
            || abs(temp_hmc5983DataStandard.y - temp_hmc5983Data.y) > OFFSET \
            || abs(temp_hmc5983DataStandard.z - temp_hmc5983Data.z) > OFFSET)                  
            XBeeParkState(ParkingUsed);
        else
            XBeeParkState(ParkingUnUsed);
        if(GetCurrentMotorState() == lock)
            XBeeLockState(ParkLockSuccess);
        else if(GetCurrentMotorState() == unlock )
            XBeeLockState(ParkUnlockSuccess);   
        else if(LockObjState == lock)
            XBeeLockState(ParkLockFailed);
        else if(LockObjState == unlock)
            XBeeLockState(ParkLockFailed);  
    }
    return 0;
}
/**********************************************************
**brief report Vbat to gateway
**********************************************************/
void ReportVbat(void)
{
    volatile static uint8 v_cnt=0;
    volatile static float vbat=0;
    //检测电压十次，取平均值
    float vbt_v=0;
    int16 vbt=0;
    vbt = HalAdcRead (HAL_ADC_CHANNEL_1, HAL_ADC_RESOLUTION_8);
    vbt_v = 3.482 * (float)vbt / 0x7f;
    v_cnt++;
    vbat += vbt_v;
    if(v_cnt == 10)
    {
        v_cnt = 0;
        vbat = vbat/(float)10;
        //发送报告
        vbat = 0;
    }
}
/**********************************************************
**brief 向网关发送字符串
**********************************************************/
uint16 SendString(uint8 in ,uint8 len )
{
  uint8 i;
  uint8 data[10];
  for(i=0;i<len;i++)
    data[i] = in;
  return XBeeSendToCoor(data,len,NO_RES);
}
/*************************************************************
**brief	比较数组是否相等
*************************************************************/
int8 arrncmp(uint8 *arr1,uint8 *arr2,uint8 n)
{
	uint8 i;
	for(i=0;i<n;i++)
	{
		if(*(arr1+i) != *(arr2+i))
			return 1;
	}
	return 0;
}
/**************************************************************
**broef 合并数组
**************************************************************/
void connectarr(uint8 *arr1,uint8 *arr2,uint8 *arr)
{
    uint8 i;
    for(i=0;i<4;i++)
    {
        arr[i]   = *(arr1+i);
        arr[i+4] = *(arr2+i);
    }
    return;
}




















#endif