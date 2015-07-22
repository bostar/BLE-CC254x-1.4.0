#include "XBeeBsp.h"
#include "XBeeAtCmd.h"
#include "hal_zlg.h"
#include "OnBoard.h"
#include "npi.h"

#if defined _XBEE_APP_
/*******************************************
**brief 加入网络
*******************************************/
void XBeeRourerJoinNet(void)
{
  XBeeSetPanID(NO_RES);   //设置ID的值
  XBeeSetChannel(NO_RES); //设置信道
  XBeeSetZS(NO_RES);
  XbeeSendAC(NO_RES);
  XBeeSendWR(NO_RES);
  XBeeReadAI(RES);
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