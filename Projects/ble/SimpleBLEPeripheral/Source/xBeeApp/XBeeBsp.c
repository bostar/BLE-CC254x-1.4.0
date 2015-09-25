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
**brief ���DMA
*******************************************/
void ClearDMA(void)
{
  uint8 i,data[1];
  i = 1;
  while(i)
    i = NPI_ReadTransport( data, 1);
}
/*******************************************
**brief ��������
*******************************************/
void XBeeJoinNet(void)
{
    uint8 panID[8],i;
    
    for(i=0;i<8;i++)
        panID[i] = 0;
    XBeeSetPanID(panID,NO_RES);             //����ID��ֵ
    XBeeSetChannel(SCAN_CHANNEL,NO_RES);    //�����ŵ�
    XBeeSetSD(3,NO_RES);
    XbeeRunAC(NO_RES);
    XBeeRunWR(NO_RES);
}
/*******************************************
**brief reset����
********************************************/
void NetReset(uint8 param)
{
    uint8 nr_param[3];
    nr_param[0] = param;
    XBeeSetAT("NR",nr_param ,1, NO_RES);
}
/*******************************************
**brief �뿪����
********************************************/
void XBeeLeaveNet(void)
{
    uint8 param[1];
    param[0] = 4;
    int8 *cmd = "CB";
    XBeeSetAT(cmd, param, 1, RES);   
}
/*******************************************
**brief �������ͣ������
*******************************************/
uint16 XBeeReqJionPark(void)
{
  uint8 data[14],i;
	
  data[0]  =  'C';	
  data[1]  =  'F';
  data[2]  =  'G';
  data[3]  = 0X01;
  for(i=0;i<8;i++)
    data[4+i] = XBeeInfo.IEEEadr[i];
  for(i=0;i<2;i++)
    data[12+i] = XBeeInfo.netadr[i];
#if defined BY_MAC
  return XBeeSendToCoorByMac(data,14,RES);
#else
  return XBeeSendToCoor(data,14,RES);
#endif
}
/***************************************************
**brief ����/����OK�ն�Ӧ��
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
**brief ����NG�ն�Ӧ��
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
**brief ����NG�ն�Ӧ��
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
**brief ֹͣ���
**********************************************/
void MotorStop(void)
{
    GPIO_XBEE_MOTOR1_TURN_HIGH();
    GPIO_XBEE_MOTOR2_TURN_HIGH();
}
/**********************************************
**brief �����ת
**********************************************/
void MotorUnlock(void)
{
   GPIO_XBEE_MOTOR1_TURN_LOW();
   GPIO_XBEE_MOTOR2_TURN_HIGH();
}
/**********************************************
**brief ��ﷴת
**********************************************/
void MotorLock(void)
{
   GPIO_XBEE_MOTOR1_TURN_HIGH();
   GPIO_XBEE_MOTOR2_TURN_LOW();
}
/**************************************************
**brief pin�Ż���
**************************************************/
void XBeePinWake(void)
{   
    if(XBeeInfo.DevType == end_dev)
    {
        GPIO_XBEE_SLEEP_TURN_HIGH();
        Delay100us();	
        GPIO_XBEE_SLEEP_TURN_LOW();
        while(HalGpioGet(GPIO_XBEE_SLEEP_INDER) != 1);
    }
}
/**************************************************
**brief pin������
**************************************************/
void XBeePinSleep(void)
{
    if(DevType == end_dev)
        GPIO_XBEE_SLEEP_TURN_HIGH();
}
/**************************************************
**brief ģʽ5 ����
**************************************************/
void XBeeMode5Wake(void)
{
    GPIO_XBEE_SLEEP_TURN_LOW();
    //Delay100us();	
    GPIO_XBEE_SLEEP_TURN_HIGH();
    while(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==0);
}
/**************************************************
**brief ��õ�ǰ���״̬
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
**brief ��õ�ǰ���״̬ ������
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
**brief XBeeReset
*****************************************************/
void XBeeReset(void)
{   
    uint8 i;
    GPIO_XBEE_RESET_TURN_LOW();
    for(i=0;i<5;i++)
        HAL_GPIO_CHANGE_DELAY();
    GPIO_XBEE_RESET_TURN_HIGH();
    for(i=0;i<9;i++)
        HAL_GPIO_CHANGE_DELAY();
}
/*****************************************************
**brief ��ʼ��xbee����
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
**brief �������Ƿ�����
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