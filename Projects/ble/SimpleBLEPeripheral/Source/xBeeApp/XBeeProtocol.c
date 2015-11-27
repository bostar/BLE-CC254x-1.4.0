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
static uint8 JoinState = SetCB;
/*****************************************************
**
*****************************************************/
uint16 CFGProcess(uint8 *rf_data)
{
    switch(*rf_data)
    {
        case 0x02:      //入网响应
            if(*(rf_data+1) == 0x01)   //允许入网
            {
                XBeeInfo.NetState =9;
                XBeeCloseLED1();
                XBeeCloseLED2();
                XBeeSetSP(100,NO_RES);
                XBeeSetST(100,NO_RES);
                XBeeSendAT("AC");
                XBeeInfo.InPark = 1;
                //XBeeReadAT("OP");
                CreatXBeeMsg(XBEE_HMC5983_EVT,ACTIVATE);
                CreatXBeeMsg(XBEE_VBT_CHENCK_EVT,ACTIVATE);
                CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
                CreatXBeeMsg(XBEE_JOIN_NET_EVT,INACTIVATE);
                CreatXBeeMsg(XBEE_CLOSE_BUZZER_EVT,INACTIVATE);
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
                LockState.FinalState = unlock;
            else if(*(rf_data+1) == 1)  //上锁
                LockState.FinalState = lock;
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
            //UartStop();
            SenFlag = 0x88;
            //UartStart();
            break;
        default:
            break;
    }
}
/**************************************************
**brief process OTA
**************************************************/
void OTAProcess(uint8 *rf_data)
{}
/**************************************************
**brief process TST
**************************************************/
uint16 TSTProcess(uint8 *rf_data)
{
    int8 data[4];
    data[0] = 'T';
    data[1] = 'E';
    data[2] = 'S';
    data[3] = 'T';
    if(rf_data[18] == 'T' && rf_data[18] == 'E' && rf_data[18] == 'S' && rf_data[18] == 'T')
        return XBeeSendToCoor((uint8*)data,4,RES);
    return 0;
}
/*********************************************************
**
*********************************************************/
void ProcessJoinRes(uint8 *temp_rbuf)
{
    if(temp_rbuf[5]=='C' && temp_rbuf[6]=='B' && temp_rbuf[7]== 0)
    {
        JoinState = GetSM;
        CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
    }
    else if(temp_rbuf[5]=='S' && temp_rbuf[6]=='M' && temp_rbuf[7] == 0)
    {
        if(temp_rbuf[2] == 0x06)
        {
            if(temp_rbuf[8] == 0)
                XBeeInfo.DevType = router;
            else if(temp_rbuf[8] == 4)
                XBeeInfo.DevType = end_dev;
            JoinState = SetSM;
            CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
        }
        else if(temp_rbuf[2] == 0x05)
        {
            JoinState = SetCh;
            CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
        }
        
    }
    else if(temp_rbuf[5]=='S' && temp_rbuf[6]=='C' && temp_rbuf[7]== 0)
    {
        JoinState = SendAC;
        CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
    }
    else if(temp_rbuf[5]=='A' && temp_rbuf[6]=='C' && temp_rbuf[7]== 0)
    {
        JoinState = GetAI;
        CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
    }
    else if(temp_rbuf[5]=='A' && temp_rbuf[6]=='I' && temp_rbuf[7]==0 && temp_rbuf[8]==0)
    {
        JoinState = JoinPark;
        CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
    }
}
/*********************************************************
**brief 
*********************************************************/
uint16 ProcessTransmitStatus(uint8 *temp_rbuf)
{    
    static uint8 time_out_cnt=0;
    
    if(temp_rbuf[8] != 0)
        time_out_cnt++;
    else
        time_out_cnt = 0;
    if(time_out_cnt >= 5)
        HAL_SYSTEM_RESET();
    return 0;
}
/*********************************************************
**brief mode status process
*********************************************************/
void ProcessModeStatus(uint8 *temp_rbuf)
{
    if(temp_rbuf[4] == 3 && XBeeInfo.InPark == 1)
    {
        //HAL_SYSTEM_RESET();
        XBeeInfo.ParentLost = 1;
        XBeeInfo.InPark = 0;
        //XBeeInfo.XBeeAI = 0;
        CreatXBeeMsg(XBEE_JOIN_NET_EVT,ACTIVATE);
        CreatXBeeMsg(XBEE_HMC5983_EVT,INACTIVATE);
        CreatXBeeMsg(XBEE_REPORT_EVT,INACTIVATE);
    }
    else if(temp_rbuf[4] == 0)
    {}
    else if(temp_rbuf[4] == 2)
    {}
}
/*********************************************************
**brief 发送车位状态
*********************************************************/
uint16 XBeeReport(eventInfoType eventInfo)
{
  uint8 data[10];
  
  data[0]   =   'S';
  data[1]   =   'E';
  data[2]   =   'N';
  data[3]   =  0x01;
  data[4]   =  eventInfo.senerEn;
  data[5]   =  eventInfo.senerEvt;
  data[6]   =  eventInfo.lockEn;
  data[7]   =  eventInfo.lockEvt;
  data[8]   =  eventInfo.batEn;
  data[9]   =  eventInfo.batEvt;
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,10,RES);
#else
  return XBeeSendToCoor(data,10,RES);
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

    if(XBeeInfo.ParentLost == 1)
    {
        XBeeInfo.ParentLost = 0;
        JoinState = GetAI;
    }
    switch(JoinState)
    {
        case SetCB:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                clear_queue(&serialBuf);
                XBeeLeaveNet();
                reval = 5000;
            }
            break;
        case GetSM:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                clear_queue(&serialBuf);
                XBeeCloseBuzzer();
                XBeeReadAT("SM");
                reval = 500;
            }
            break;
        case SetSM:
            if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
            {
                clear_queue(&serialBuf);
                if(XBeeInfo.DevType == end_dev)
                {
                    XBeeSetSM(PinCyc,RES);
                    reval = 500;
                }
                else if(XBeeInfo.DevType == router)
                {
                    JoinState = SetCh;
                    reval = 10;
                }
            }
            break;
        case SetCh:
            clear_queue(&serialBuf);
            XBeeSetChannel(SCAN_CHANNEL,RES);
            reval = 500;
            break;
        case SendAC:
            clear_queue(&serialBuf);
            XbeeRunAC(RES);
            reval = 500;
            break;
        case GetAI:
            clear_queue(&serialBuf);
            XBeeReadAT("AI");
            reval = 500;
            break;
        case JoinPark:
            XBeeReqJionPark();
            reval = 20000;
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
 
    //UartStop();
    temp_hmc5983Data = hmc5983Data;
    temp_hmc5983DataStandard = hmc5983DataStandard;
    //UartStart();
    if(temp_hmc5983Data.state != 0x88)
    {
        Uart1_Send_Byte("get",osal_strlen("get"));
        return 0;
    }
    if( abs(temp_hmc5983DataStandard.x - temp_hmc5983Data.x) > SEN_THR \
        || abs(temp_hmc5983DataStandard.y - temp_hmc5983Data.y) > SEN_THR \
        || abs(temp_hmc5983DataStandard.z - temp_hmc5983Data.z) > SEN_THR)  
    {
        eventInfo.senerEn = 1;
        if(eventInfo.senerEvt == ParkingUnUsed)
        {
            eventInfo.senerEvt = ParkingUsed;
            CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
            //return XBeeReport(eventInfo);
            return 0;
        }
        else
            eventInfo.senerEvt = ParkingUsed;
    }
    else
    {
        eventInfo.senerEn = 1;
        if(eventInfo.senerEvt == ParkingUsed)
        {
            eventInfo.senerEvt = ParkingUnUsed;
            CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
            //return XBeeReport(eventInfo);
            return 0;
        }
        else
            eventInfo.senerEvt = ParkingUnUsed;
    }
    return 0;
}
/**********************************************************
**brief report park state periodly
**********************************************************/
uint16 ReportLockState(void)
{
    if(GetCurrentMotorState() == lock)
    {
        eventInfo.lockEn = 1;
        eventInfo.lockEvt = ParkLockSuccess;
        CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
        //XBeeReport(eventInfo);
    }
    else if(GetCurrentMotorState() == unlock )
    {
        eventInfo.lockEn = 1;
        eventInfo.lockEvt = ParkUnlockSuccess;
        CreatXBeeMsg(XBEE_REPORT_EVT,ACTIVATE);
        //XBeeReport(eventInfo);
    }
    return 0;
}
/**********************************************************
**brief report Vbat to gateway
**********************************************************/
float ReportVbat(void)
{
    static uint16 v_cnt=0;
    static float vbat=0;
    float reval=0,vbt_v=0;
    int16 vbt=0;
    
    vbt = HalAdcRead (HAL_ADC_CHANNEL_1, HAL_ADC_RESOLUTION_8);
    vbt_v = 3.2 * (float)vbt / 0x7f;
    v_cnt++;
    vbat += vbt_v;
    if(v_cnt == 10)
    {
        vbat = vbat/(float)v_cnt;  //检测电压十次，取平均值
        reval = vbat * 6;
        v_cnt = 0;
        vbat = 0;
    }
    return reval;
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