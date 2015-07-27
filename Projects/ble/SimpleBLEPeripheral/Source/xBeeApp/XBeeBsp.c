#include "XBeeBsp.h"
#include "XBeeAtCmd.h"
#include "hal_zlg.h"
#include "OnBoard.h"
#include "npi.h"
#include "XBeeApp.h"

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
  for(i=0;i<8;i++)
    panID[i] = 0;
  XBeeSetPanID(panID,NO_RES);   //设置ID的值
  XBeeSetChannel(NO_RES); //设置信道
  XBeeSetZS(NO_RES);
  XbeeSendAC(NO_RES);
  XBeeSendWR(NO_RES);
  XBeeReadAI(RES);
}
/*******************************************
**brief 离开网络
********************************************/
void XBeeLeaveNet(void)
{
  uint8 panID[8],i;
  for(i=0;i<8;i++)
    panID[i] = 0xee;
  XBeeSetPanID(panID,NO_RES);   //设置ID的值
  XbeeSendAC(NO_RES);
  XBeeReadAI(RES);
  FlagJionNet = NetOK;
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
  //return XBeeUnicastTrans(data1,data1+8,Default,data,14,RES);  
}



void initXBeeBsp(void)
{
   
}

void setBeepOn(void)
{
    
}

void setBeepOff(void)
{
    
}

void setLedBit(unsigned char bits)
{
   
}

  
#endif