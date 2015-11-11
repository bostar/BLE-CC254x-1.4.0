#include "XBeeBsp.h"
#include "XBeeAtCmd.h"
#include "hal_zlg.h"
#include "OnBoard.h"
#include "npi.h"
#include "XBeeApp.h"
#include "hal_board.h"
#include "hal_board_cfg.h"
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_drivers.h"
#include "osal.h"
#include "XBeeApp.h"
#include "XBeeProtocol.h"
#include "bcomdef.h"
#include "osal_snv.h"
#include "stdio.h"

#if defined _XBEE_APP_
/*******************************************
**brief 清除DMA
*******************************************/
void ClearDMA(void)
{
  uint8 i,data[1];
  i = 1;
  while(i)
    i = NPI_ReadTransport( data, 1);
}
/*******************************************
**brief 加入网络
*******************************************/
void XBeeJoinNet(void)
{
    uint8 panID[8],i;
    
    for(i=0;i<8;i++)
        panID[i] = XBeeInfo.panID[i];
    XBeeSetPanID(panID,NO_RES);             //设置ID的值
    XBeeSetChannel(SCAN_CHANNEL,NO_RES);    //设置信道
    //XBeeSetSD(3,NO_RES);
    //XBeeSetZS(1,NO_RES);
    XbeeRunAC(NO_RES);
    XBeeRunWR(NO_RES);
}
/*******************************************
**brief reset网络
********************************************/
void NetReset(uint8 param)
{
    uint8 nr_param[3];
    nr_param[0] = param;
    XBeeSetAT("NR",nr_param ,1, NO_RES);
}
/*******************************************
**brief 离开网络
********************************************/
void XBeeLeaveNet(void)
{
    uint8 param[1];
    param[0] = 4;
    int8 *cmd = "CB";
    XBeeSetAT(cmd, param, 1, RES);   
}
/*******************************************
**brief 申请加入停车网络
*******************************************/
uint16 XBeeReqJionPark(void)
{
  uint8 data[14],i;
	
  data[0]  =  'C';	
  data[1]  =  'F';
  data[2]  =  'G';
  data[3]  = 0X01;
  for(i=0;i<8;i++)
    data[4+i] = XBeeInfo.MacAdr[i];
  for(i=0;i<2;i++)
    data[12+i] = XBeeInfo.NetAdr[i];
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,14,RES);
#else
  return XBeeSendToCoor(data,14,RES);
#endif
}
/***************************************************
**brief 解锁/锁定OK终端应答
***************************************************/
uint16 XBeeEndDeviceLockRepOK(void)
{
  uint8 data[5];
  data[0]  =  'C';	
  data[0]  =  'T';
  data[1]  =  'L';
  data[3]  =  0x01;
  data[4]  =  0x01;
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
}
/***************************************************
**brief 解锁NG终端应答
***************************************************/
uint16 XBeeEndDeviceUnlockRepNG(void)
{
  uint8 data[5];
  data[0]  =  'C';	
  data[0]  =  'T';
  data[1]  =  'L';
  data[3]  =  0x01;
  data[4]  =  0x02;
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
}
/***************************************************
**brief 锁定NG终端应答
***************************************************/
uint16 XBeeEndDeviceLockRepNG(void)
{
  uint8 data[5];
  data[0]  =  'C';	
  data[0]  =  'T';
  data[1]  =  'L';
  data[3]  =  0x01;
  data[4]  =  0x03;
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,5,RES);
#else
  return XBeeSendToCoor(data,5,RES);
#endif
}
/**********************************************
**brief 停止马达
**********************************************/
void MotorStop(void)
{
    GPIO_XBEE_MOTOR1_TURN_HIGH();
    GPIO_XBEE_MOTOR2_TURN_HIGH();
}
/**********************************************
**brief 马达正转
**********************************************/
void MotorUnlock(void)
{
   GPIO_XBEE_MOTOR1_TURN_LOW();
   GPIO_XBEE_MOTOR2_TURN_HIGH();
}
/**********************************************
**brief 马达反转
**********************************************/
void MotorLock(void)
{
   GPIO_XBEE_MOTOR1_TURN_HIGH();
   GPIO_XBEE_MOTOR2_TURN_LOW();
}
/**************************************************
**brief pin脚唤醒
**************************************************/
void XBeePinWake(void)
{   
    uint32 cnt=0;
    if(XBeeInfo.DevType == end_dev && HalGpioGet(GPIO_XBEE_SLEEP_INDER) != 1)
    {
        GPIO_XBEE_SLEEP_TURN_LOW();
        while(HalGpioGet(GPIO_XBEE_CTS) == 1 && cnt < 72000)
        {
            cnt++;
        }
    }
}
/**************************************************
**brief pin脚休眠
**************************************************/
void XBeePinSleep(void)
{
    if(XBeeInfo.DevType == end_dev)
        GPIO_XBEE_SLEEP_TURN_HIGH();
}
/**************************************************
**brief 模式5 唤醒
**************************************************/
void XBeeMode5Wake(void)
{
    GPIO_XBEE_SLEEP_TURN_LOW();
    //Delay100us();	
    GPIO_XBEE_SLEEP_TURN_HIGH();
    while(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==0);
}
/**************************************************
**brief 获得当前马达状态
**************************************************/
LockCurrentStateType GetCurrentMotorState(void)
{
    uint8 key1,key2,key3;
    LockCurrentStateType state=none;
    
    key1 = HalGpioGet(GPIO_XBEE_KEY1);
    key2 = HalGpioGet(GPIO_XBEE_KEY2);
    key3 = HalGpioGet(GPIO_XBEE_KEY3);

    if(key1 == 1 && key2 == 1 && key3 == 1 )
        state = lock;
    else if(key1 == 1 && key2 == 0 && key3 == 0 )
        state = unlock;  
    else if(key1 == 0 && key2 == 1 && key3 == 1 )
        state = over_lock;
    
    return state;  
}
/**************************************************
**brief 获得当前马达状态 、测试
**************************************************/
uint8 GetCurrentMotorStateTest(void)
{
    uint8 valve=0;
    
    if(HalGpioGet(GPIO_XBEE_KEY1) == 1)
        valve |= 0x01;
    if(HalGpioGet(GPIO_XBEE_KEY2) == 1)
        valve |= 0x02;
    if(HalGpioGet(GPIO_XBEE_KEY3) == 1)
        valve |= 0x04;
    return valve;
}
/***************************************************
**brief conrtol motor
***************************************************/
uint8 ControlMotor(LockCurrentStateType state)
{
    LockCurrentStateType MotorCurrentState;
    uint8 reval=0;
   
    MotorCurrentState = GetCurrentMotorState();
    switch(state)
    {
        case lock:
            if(MotorCurrentState == state)
            {
                MotorStop();
                reval = 1;
            }
            else if(MotorCurrentState == over_lock)
            {
                MotorUnlock();
                reval = 0;
            }
            else
            {
                MotorLock();
                reval = 0;
            }
            break;
        case unlock:
            if(MotorCurrentState == state)
            {
                MotorStop();
                reval = 2;
            }
            else
            {
                MotorUnlock();
                reval = 0;
            }
            break;
        default:
            break;
    }
    return reval;
}
/*****************************************************
**brief XBeeReset
*****************************************************/
void XBeeReset(void)
{   
    GPIO_XBEE_RESET_TURN_LOW();
    HAL_GPIO_CHANGE_DELAY();
    GPIO_XBEE_RESET_TURN_HIGH();
    HAL_GPIO_CHANGE_DELAY();
}
/*****************************************************
**brief 初始化xbee参数
*****************************************************/
void XBeeDevInit(void)
{
    uint8 i;
    XBeeSendAT("RE");
//    for(i=0;i<5;i++)
        HAL_GPIO_CHANGE_DELAY();
    XBeeLeaveNet();
    for(i=0;i<7;i++)
        HAL_GPIO_CHANGE_DELAY();
}
/*****************************************************
**brief 监测马达是否阻塞
*****************************************************/
uint8 CheckADC(void)
{
    uint8 reval=0;
    
    return reval;
}
/*****************************************************
**brief 关中断
*****************************************************/
void UartStop(void)
{
    IEN0 &= 0xf7; // 关总中断
}
/*****************************************************
**brief 开中断
*****************************************************/
void UartStart(void)
{
    IEN0 |= 0x08; // 开总中断
}

void Delay1ms(void)		//@33.000MHz
{
	unsigned char i, j;

	i = 33;
	j = 22;
	do
	{
		while (--j);
	} while (--i);
}
void Delay100us(void)		//@33.000MHz
{
	unsigned char i, j;

	i = 4;
	j = 50;
	do
	{
		while (--j);
	} while (--i);
}

void setLedBit(unsigned char bits)
{
   
}

  
#endif