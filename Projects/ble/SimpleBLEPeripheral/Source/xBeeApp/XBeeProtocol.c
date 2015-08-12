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
void CFGProcess(uint8 *cmd)
{
    switch(*cmd)
    {
        case 0x00:   //允许限时加入网络
            XBeeSetNJ(*(cmd+1),NO_RES);
            break;
        case 0x02:   //入网响应
            if(*(cmd+1) == 0x01)   //允许入网
              FlagJionNet = NetOK;    //组网成功后，进入正常工作模式
            else    //禁止入网
            {
                XBeeLeaveNet();  
                FlagJionNet = NetOK;
                //zigbee进入休眠
            } 
            osal_stop_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT);
            break;
        case 0x03:  //恢复出厂设置
            XBeeLeaveNet();  
            FlagJionNet = NetOK;
            //zigbee进入休眠
            break;
        default:
            break;
  }
}
/*****************************************************
**
*****************************************************/
void CTLProcess(uint8 *cmd)
{
  switch(*cmd)
  {
    case 0:
        if(*(cmd+1) == 0)  //解锁
        {
            MotorForward();
            LockTargetState = unlock;
        }
        if(*(cmd+1) == 1)  //上锁
        {
            MotorReverse();
            LockTargetState = unlock;
        }
        osal_set_event( XBeeTaskID, XBEE_MOTOO_CTL_EVT );
        osal_stop_timerEx( XBeeTaskID,XBEE_HMC5983_EVT);
        osal_stop_timerEx( XBeeTaskID,XBEE_VBT_CHENCK_EVT);
        break;
    default:
        break;
  }
}
/**************************************************
**brief 处理SEN指令
**************************************************/
void SENProcess(uint8 *cmd)
{
    switch(*cmd)
    {
        case 0x00:
            SenFlag = 0x88;
            osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT );
            break;
        default:
            break;
    }
}
void OTAProcess(uint8 *cmd)
{}
void TSTProcess(uint8 *cmd)
{}

/*********************************************************
**brief 发送锁状态函数
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
**brief 发送车进场状态
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






















#endif