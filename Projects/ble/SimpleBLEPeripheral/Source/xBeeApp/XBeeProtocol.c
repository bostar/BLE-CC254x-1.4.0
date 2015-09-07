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

#if defined _XBEE_APP_
/*****************************************************
**
*****************************************************/
void CFGProcess(uint8 *cmd)
{
    switch(*cmd)
    {
        case 0x00:      //������ʱ��������ʱ��
            XBeeSetNJ(*(cmd+1),NO_RES);
            break;
        case 0x02:      //������Ӧ
            if(*(cmd+1) == 0x01)   //��������
            {
              FlagJionNet = NetOK;
              osal_stop_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT);
              osal_set_event( XBeeTaskID, XBEE_HMC5983_EVT ); 
              return;
            }
            else if(*(cmd+1) == 0x00)   //��ֹ����
            {
                XBeeLeaveNet();  
                FlagJionNet = JoinNet;
                osal_stop_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT);
                osal_start_timerEx( XBeeTaskID, XBEE_JOIN_NET_EVT, 5000 );
                return;
            }
            break;
        case 0x03:  //�ָ���������
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
        if(*(cmd+1) == 0)  //����
            LockObjState = unlock;
        else if(*(cmd+1) == 1)  //����
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
**brief ����SENָ��
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
**brief mode status process
*********************************************************/
void ProcessModeStatus(XBeeUartRecDataDef temp_rbuf)
{
    if(temp_rbuf.data[4] == 0x03)
    {
        FlagJionNet = JoinNet;
        osal_set_event( XBeeTaskID, XBEE_JOIN_NET_EVT );
        osal_stop_timerEx( XBeeTaskID,XBEE_HMC5983_EVT);
        osal_stop_timerEx( XBeeTaskID,XBEE_VBT_CHENCK_EVT);
    }
}
/*********************************************************
**brief ������״̬����
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
**brief ���ͳ�λ״̬
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
/**************************************************
**brief set xbee sleep mode
**************************************************/
void SetXBeeSleepMode(void)
{
    if(HalGpioGet(GPIO_XBEE_SLEEP_INDER)==1)  //high--wake  low--sleep
    {
        switch(SetSleepMode)
        {
            case SetMode:
                //XBeeSetSM(PinCyc,RES);
                XBeeSetSM(Disable,RES);
                break;
            case SetSP:
                XBeeSetSP(100,RES);
                break;
            case SetST:
                XBeeSetST(100,RES);
                break;
            case SetSN:
                break;
            default:
                break;
        }
    }
}
/**********************************************************
**brief jion park net
**********************************************************/
void JionParkNet(void)
{
    static uint8 XBeeReqJionParkIdx=0;
    switch(FlagJionNet)
    {
        case JoinNet:
            XBeeRourerJoinNet();
            break;
        case GetSH:
            XBeeReadSH();
            break;
        case GetSL:
            XBeeReadSL();
            break;
        case GetMY:
            XBeeReadMY();
            break;
        case JoinPark:
            XBeeReqJionParkIdx++;
            if(XBeeReqJionParkIdx == 10)
                FlagJionNet = JoinNet;
            XBeeReqJionPark();
            break;  
        default:
            break;
    }
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
    hmc5983Data.state = 1;
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
    if(temp_hmc5983Data.state!=0x88)
        return 0;
    cnt++;
    if(cnt > 2)
    {
        cnt = 0;
        if( abs(temp_hmc5983DataStandard.x - temp_hmc5983Data.x) > OFFSET \
            || abs(temp_hmc5983DataStandard.y - temp_hmc5983Data.y) > OFFSET \
            || abs(temp_hmc5983DataStandard.z - temp_hmc5983Data.z) > OFFSET)                  
            return XBeeParkState(ParkingUsed);
        else
            return XBeeParkState(ParkingUnUsed);
    }
    return 0;
}

/**********************************************************
**brief �����ط����ַ���
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