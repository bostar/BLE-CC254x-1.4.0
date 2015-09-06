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
        case 0x00:      //设置限时加入网络时限
            XBeeSetNJ(*(cmd+1),NO_RES);
            break;
        case 0x02:      //入网响应
            if(*(cmd+1) == 0x01)   //允许入网
            {
              FlagJionNet = NetOK;
              osal_stop_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT);
              osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT ); 
              return;
            }
            else if(*(cmd+1) == 0x00)   //禁止入网
            {
                XBeeLeaveNet();  
                FlagJionNet = JoinNet;
                osal_stop_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT);
                osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 5000 );
                return;
            }
            break;
        case 0x03:  //恢复出厂设置
            XBeeLeaveNet();  
            FlagJionNet = NetOK;
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
            LockObjState = unlock;
        else if(*(cmd+1) == 1)  //上锁
            LockObjState = lock;
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
**
*********************************************************/
void ProcessAT(XBeeUartRecDataDef temp_rbuf)
{
    if(temp_rbuf.data[5]=='N' && temp_rbuf.data[6]=='J')
    {}
    else if(temp_rbuf.data[5]=='A' && temp_rbuf.data[6]=='I')
    {
        if(temp_rbuf.data[7]==0 && temp_rbuf.data[8]==0)
        {
            FlagJionNet = GetSH;   
            XBeeSetSM(PinCyc,NO_RES);
            osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='H')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
                XBeeAdr.IEEEadr[cnt] = temp_rbuf.data[8+cnt];
            FlagJionNet = GetSL;
            osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
        }
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='L')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<4;cnt++)
                XBeeAdr.IEEEadr[4+cnt] = temp_rbuf.data[8+cnt];
            FlagJionNet = GetMY;
            osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
        }                 
    }
    else if(temp_rbuf.data[5]=='M' && temp_rbuf.data[6]=='Y')
    {
        if(temp_rbuf.data[7]==0)
        {
            uint8 cnt;
            for(cnt=0;cnt<2;cnt++)
                XBeeAdr.netadr[cnt] = temp_rbuf.data[8+cnt];
            FlagJionNet = JoinPark;
            osal_stop_timerEx( XBeeTaskID,XBEE_JOIN_NET_EVT);
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );      
        }
    } 
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='M')
    {
        //if(temp_rbuf.data[7] == 0)
        SetSleepMode = SetSP;
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='P')
    {
        if(temp_rbuf.data[7] == 0)
        SetSleepMode = SetST;
    }
    else if(temp_rbuf.data[5]=='S' && temp_rbuf.data[6]=='T')
    {
        if(temp_rbuf.data[7] == 0)
        {
            FlagJionNet = JoinNet;
            osal_stop_timerEx( XBeeTaskID, XBEE_START_DEVICE_EVT);
            osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        }
    }
    else if(temp_rbuf.data[5]=='M' && temp_rbuf.data[6]=='P')
    {}
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