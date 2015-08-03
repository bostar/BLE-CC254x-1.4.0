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
void XBeeRourerJoinNet(void)
{
  uint8 panID[8],i;
  for(i=0;i<8;i++)
    panID[i] = 0;
  XBeeSetPanID(panID,NO_RES);   //����ID��ֵ
  XBeeSetChannel(NO_RES); //�����ŵ�
  XBeeSetZS(NO_RES);
  XbeeSendAC(NO_RES);
  XBeeSendWR(NO_RES);
  XBeeReadAI(RES);
}
/*******************************************
**brief �뿪����
********************************************/
void XBeeLeaveNet(void)
{
  uint8 panID[8],i;
  for(i=0;i<8;i++)
    panID[i] = 0xee;
  XBeeSetPanID(panID,NO_RES);   //����ID��ֵ
  XbeeSendAC(NO_RES);
  XBeeReadAI(RES);
  FlagJionNet = NetOK;
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
    data[4+i] = XBeeAdr.IEEEadr[i];
  for(i=0;i<2;i++)
    data[12+i] = XBeeAdr.netadr[i];
  return XBeeSendToCoor(data,14,NO_RES);  
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
  return XBeeSendToCoor(data,5,NO_RES);
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
  return XBeeSendToCoor(data,5,NO_RES);
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
  return XBeeSendToCoor(data,5,NO_RES);
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
void MotorForward(void)
{
   GPIO_XBEE_MOTOR1_TURN_LOW();
   GPIO_XBEE_MOTOR2_TURN_HIGH();
}
/**********************************************
**brief ��ﷴת
**********************************************/
void MotorReverse(void)
{
   GPIO_XBEE_MOTOR1_TURN_HIGH();
   GPIO_XBEE_MOTOR2_TURN_LOW();
}


/**************************************************
**brief xbee���߳�ʼ��
**************************************************/
void XBeeSleepInit(void)
{
   XBeeSendSM(PinSleep,NO_RES);
   //XBeeSetSP(0x64,NO_RES);
}

void XBeeSleep(void)
{
    
}

void XBeeWake(void)
{
    
}

void setLedBit(unsigned char bits)
{
   
}

  
#endif