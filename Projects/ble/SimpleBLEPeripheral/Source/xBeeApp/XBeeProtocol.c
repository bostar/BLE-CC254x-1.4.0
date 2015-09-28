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
#include "osal_snv.h"
#include "bcomdef.h"

#if defined _XBEE_APP_

static uint8 my_msg=0;
/*****************************************************
**
*****************************************************/
uint16 CFGProcess(uint8 *rf_data)
{
    switch(*rf_data)
    {
        case 0x00:      //设置限时加入网络时限,end device禁止该功能
            if(XBeeInfo.DevType == router)
                XBeeSetNJ(*(rf_data+1),NO_RES);
            break;
        case 0x02:      //入网响应
            if(*(rf_data+1) == 0x01)   //允许入网
            {
                XBeeInfo.NetState =9;
                XBeeCloseLED1();
                XBeeCloseLED2();
                XBeeSetSP(100,RES);
                XBeeSetST(100,RES);
                XBeeSendAT("AC");
                XBeeSendAT("WR");
                XBeeInfo.InPark = 1;
                DailyEvt();
            }
            else if(*(rf_data+1) == 0x00)   //禁止入网
                HAL_SYSTEM_RESET();
            break;
        case 0x03:  //恢复出厂设置
            HAL_SYSTEM_RESET();
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
    if(temp_rbuf.data[5]=='A' && temp_rbuf.data[6]=='I')
    {
        if(temp_rbuf.data[7]==0 && temp_rbuf.data[8]==0)
            XBeeInfo.XBeeAI = 1;
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='H')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
                XBeeInfo.MacAdr[cnt] = temp_rbuf.data[8+cnt];
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='L')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
                XBeeInfo.MacAdr[4+cnt] = temp_rbuf.data[8+cnt];    
        }
    }
    else if(temp_rbuf.data[5]=='M' && temp_rbuf.data[6]=='Y')
    {
        if(temp_rbuf.data[7]==0 && XBeeInfo.InPark != 1)
        {
            uint8 cnt;
            for(cnt=0;cnt<2;cnt++)
                XBeeInfo.NetAdr[cnt] = temp_rbuf.data[8+cnt];
        }
        else if(temp_rbuf.data[7]==0 && XBeeInfo.InPark == 1)
        {
            if(XBeeInfo.NetAdr[0] != temp_rbuf.data[8] || XBeeInfo.NetAdr[1] != temp_rbuf.data[9])
                HAL_SYSTEM_RESET();
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='M')
    {
        if(temp_rbuf.data[7] == 0)
        {
            if(temp_rbuf.data[8] == 0)
                XBeeInfo.DevType = router;
            else if(temp_rbuf.data[8] == 4)
                XBeeInfo.DevType = end_dev;
        }
    }
    else if(temp_rbuf.data[5]=='C' && temp_rbuf.data[6]=='H')
        XBeeInfo.channel = temp_rbuf.data[8];
}
/*********************************************************
**brief 
*********************************************************/
uint16 ProcessTransmitStatus(XBeeUartRecDataDef temp_rbuf)
{    
    static uint8 time_out_cnt=0;
    
    if(temp_rbuf.data[8] != 0)
        time_out_cnt++;
    else
        time_out_cnt = 0;
    if(time_out_cnt >= 5)
        HAL_SYSTEM_RESET();
    return 0;
}
/*********************************************************
**brief 
*********************************************************/
void ProcessAR(XBeeUartRecDataDef temp_rbuf)
{
    uint8 i;
    if(temp_rbuf.data[12] == 0 && temp_rbuf.data[13] == 0)
    {
        for(i=0;i<8;i++)
            CoorMAC[i] = temp_rbuf.data[4+i];
        if(my_msg == 1)
        {   
            my_msg++;
            osal_set_event( XBeeTaskID,XBEE_JOIN_NET_EVT);
        }
    }
    return;
}
/*********************************************************
**brief mode status process
*********************************************************/
void ProcessModeStatus(XBeeUartRecDataDef temp_rbuf)
{
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
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
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
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
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
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
}
/**********************************************************
**brief 设置休眠，入网
**********************************************************/
uint32 SleepModeAndJoinNet(void)
{
    uint32 reval=10;   //单位ms
    static uint8 JoinState = SetRE;
    switch(JoinState)
    {
        case SetRE:
            XBeeReset();
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                XBeeSendAT("RE");
                JoinState = SetCB;
                reval = 500;
            }
            break;
        case SetCB:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                XBeeLeaveNet();
                JoinState = GetSM;
                reval = 1000;
            }
            break;
        case GetSM:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                XBeeCloseBuzzer();
                XBeeReadAT("SM");
                JoinState = SetSM;
                reval = 500;
            }
            break;
        case SetSM:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                if(XBeeInfo.DevType == end_dev)
                {
                    XBeeSetSM(PinCyc,NO_RES);
                    JoinState = JoinNet;
                }
                else if(XBeeInfo.DevType == router)
                    JoinState = JoinNet;
                else
                {
                    XBeeReadAT("SM");
                    JoinState = SetSM;
                }
                reval = 500;
            }
            break;
        case JoinNet:
            XBeeJoinNet();
            XBeeReadAT("AI");
            JoinState = GetAI;
            reval = 2000;
            break;
        case GetAI:
            if(XBeeInfo.XBeeAI == 1)
                JoinState = GetSH;
            else
            {
                XBeeReadAT("AI");
                reval = 500;
            }
            break;
        case GetSH:
            XBeeReadAT("SH");
            JoinState = GetSL;
            reval = 100;
            break;
        case GetSL:
            XBeeReadAT("SL");
            JoinState = GetMY;
            reval = 100;
            break;
        case GetMY:
            XBeeReadAT("MY");
            JoinState = JoinPark;
            reval = 100;
            break;
        case JoinPark:
            if(XBeeInfo.InPark != 1)
            {
                XBeeReqJionPark();
                reval = 1500;
            }
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
    if( abs(temp_hmc5983DataStandard.x - temp_hmc5983Data.x) > SEN_THR \
        || abs(temp_hmc5983DataStandard.y - temp_hmc5983Data.y) > SEN_THR \
        || abs(temp_hmc5983DataStandard.z - temp_hmc5983Data.z) > SEN_THR)  
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
uint16 ReportLockState(void)
{
    if(GetCurrentMotorState() == lock)
        XBeeLockState(ParkLockSuccess);
    else if(GetCurrentMotorState() == unlock )
        XBeeLockState(ParkUnlockSuccess);   
    else if(LockObjState == lock)
        XBeeLockState(ParkLockFailed);
    else if(LockObjState == unlock)
        XBeeLockState(ParkLockFailed);
    XBeeReadAT("MY");
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
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
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