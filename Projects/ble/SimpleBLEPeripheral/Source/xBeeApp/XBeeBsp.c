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
void XBeeRourerJoinNet(void)
{
    uint8 panID[8],i;
    static uint16 cnt=0;
    if(ReadFlashFlag == SUCCESS && cnt<100)
    {
        cnt++;
        for(i=0;i<8;i++)
        {
            panID[i] = FlashLockState.panID[i];
        }
    }
    else
    {
        for(i=0;i<8;i++)
            panID[i] = 0;
    }
    XBeeSetPanID(panID,NO_RES);   //设置ID的值
    XBeeSetChannel(0x0014,NO_RES); //设置信道
    XBeeSetSD(6,NO_RES);
    XbeeRunAC(NO_RES);
    XBeeSetZS(1,NO_RES);
    XbeeRunAC(NO_RES);
    XBeeRunWR(NO_RES);
    XBeeReadAT("AI");
}
/*******************************************
**brief 离开网络
********************************************/
void XBeeLeaveNet(void)
{
  uint8 panID[8],i;
  for(i=0;i<8;i++)
    panID[i] = 0x88;
  XBeeSetPanID(panID,NO_RES);   //设置ID的值
//  XbeeSendAC(NO_RES);
//  XBeeReadAI();
//  FlagJionNet = NetOK;
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
    data[4+i] = XBeeAdr.IEEEadr[i];
  for(i=0;i<2;i++)
    data[12+i] = XBeeAdr.netadr[i];
  return XBeeSendToCoor(data,14,NO_RES);  
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
  return XBeeSendToCoor(data,5,NO_RES);
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
  return XBeeSendToCoor(data,5,NO_RES);
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
  return XBeeSendToCoor(data,5,NO_RES);
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
**brief xbee休眠初始化，设置为mode5
**************************************************/
uint16 XBeeSleepMode5(void)
{
   return XBeeSetSM(PinSleep,RES);
   //XBeeSetSP(0x64,NO_RES);
}
/*************************************************
**brief 设置为pinsleep
*************************************************/
uint16 XBeeSleepMode1(void)
{
    return XBeeSetSM(PinSleep,NO_RES);
}
/**************************************************
**brief pin脚唤醒
**************************************************/
void XBeePinWake(void)
{
    GPIO_XBEE_SLEEP_TURN_LOW();
}
/**************************************************
**brief pin脚休眠
**************************************************/
void XBeePinSleep(void)
{
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
/****************************************************
**brief keep current locker state
****************************************************/
void KeepLockState(void)
{
    LockCurrentStateType MotorCurrentState;
    MotorCurrentState = GetCurrentMotorState();
    if(MotorCurrentState != LockObjState )
    {
        switch(LockObjState)
        {
            case lock:
                if(MotorCurrentState == over_lock)
                    MotorUnlock();
                else
                    MotorLock();
                if(MotorCurrentState == LockObjState)
                    MotorStop();
                break;
            case unlock:
                MotorUnlock();
                if(MotorCurrentState == LockObjState)
                    MotorStop();
                break;
            default:
                break;
        }
    }
    else
        MotorStop();
    return; 
}
/***************************************************
**brief conrtol motor
***************************************************/
uint8 ControlMotor(void)
{
    LockCurrentStateType MotorCurrentState;
    uint8 reval=0;
   
    MotorCurrentState = GetCurrentMotorState();
    switch(LockObjState)
    {
        case lock:
            MotorLock();
            if(MotorCurrentState == LockObjState || MotorCurrentState == over_lock)
            {
                MotorStop();
                reval = 1;
            }
            break;
        case unlock:
            MotorUnlock();
            if(MotorCurrentState == LockObjState)
            {
                MotorStop();
                reval = 2;
            }
            break;
        default:
            break;
    }
    return reval;
}
/*****************************************************
**brief 监测马达是否阻塞
*****************************************************/
uint8 CheckADC(void)
{
    uint8 reval=0;
    
    return reval;
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