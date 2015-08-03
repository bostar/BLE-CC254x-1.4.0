#include "XBeeProtocol.h"
#include "npi.h"
#include <string.h>
#include "XBeeAtCmd.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "XBeeApp.h"
#include "XBeeBsp.h"
#include "XBeeApp.h"


#if defined _XBEE_APP_
/*****************************************************
**
*****************************************************/
void CFGProcess(uint8 cmd)
{
  switch(cmd)
  {
    case 0x00:   //������ʱ��������
      XBeeSetNJ(XBeeUartRec.data[19],NO_RES);
    break;
    case 0x02:   //������Ӧ
      if(*(&cmd+1) == 0x01)  //��������
        FlagJionNet = NetOK;  //�����ɹ��󣬽�����������ģʽ
      else   //��ֹ����
      {
        XBeeLeaveNet();  
        FlagJionNet = NetOK;
        osal_set_event( XBeeTaskID, XBEE_IDLE_EVT ); //�������ģʽ
        UartCtl = 1;   //�رմ���
        //zigbee��������
      } 
      break;
    case 0x03:
      XBeeLeaveNet();
      XBeeLeaveNet();  
      FlagJionNet = NetOK;
      osal_set_event( XBeeTaskID, XBEE_IDLE_EVT ); //�������ģʽ
      UartCtl = 1;   //�رմ���
      //zigbee��������
      break;
    default:
    break;
  }
}
/*****************************************************
**
*****************************************************/
void CTLProcess(uint8 cmd)
{
  switch(cmd)
  {
    case 0:
      if(XBeeUartRec.data[19] == 0)  //����
      {
        MotorForward();
      }
      if(XBeeUartRec.data[19] == 1)  //����
      {
        MotorReverse();
      }
      break;
    default:
      break;
  }
}
void SENProcess(uint8 cmd)
{}
void OTAProcess(uint8 cmd)
{}
void TSTProcess(uint8 cmd)
{}

/*********************************************************
**brief ������״̬����
*********************************************************/
uint16 XBeeLockState(LockStateType LockState)
{
  uint8 data[5];
  
  data[0]   =   'C';
  data[1]   =   'T';
  data[2]   =   'L';
  data[3]   =  0x01;
  data[4]   =  LockState;
  
  return XBeeSendToCoor(data,5,NO_RES);
}
/*********************************************************
**brief ���ͳ�����״̬
*********************************************************/
uint16 XBeeCarState(CarStateType CarState)
{
  uint8 data[5];
  
  data[0]   =   'S';
  data[1]   =   'E';
  data[2]   =   'N';
  data[3]   =  0x01;
  data[4]   =  CarState;
  
  return XBeeSendToCoor(data,5,NO_RES);
}
/*********************************************************
**brief ���͵�ص���
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

























#endif