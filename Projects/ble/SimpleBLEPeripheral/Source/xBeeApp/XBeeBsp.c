#include "XBeeBsp.h"
#include "XBeeAtCmd.h"
#include "hal_zlg.h"
#include "OnBoard.h"
#include "npi.h"

#if defined _XBEE_APP_
/*******************************************
**brief ��������
*******************************************/
void XBeeRourerJoinNet(void)
{
  XBeeSetPanID(NO_RES);   //����ID��ֵ
  XBeeSetChannel(NO_RES); //�����ŵ�
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