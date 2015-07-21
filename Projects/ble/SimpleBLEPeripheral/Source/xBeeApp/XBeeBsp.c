#include "XBeeBsp.h"
#include "XBeeAtCmd.h"
#include "hal_zlg.h"
#include "OnBoard.h"
#include "npi.h"

#if defined _XBEE_APP_

/*****************************************
**brief 接收xbee发出的串口数据
**param buff 指向接收数据的指针
**reval 数据的长度
*****************************************/
uint16 XBeeUartRead(uint8* buff)
{
  uint16 NPIState,cnt=1,DataLen;
  uint8 checksum;
  while(cnt>0)
  {
    NPIState = NPI_ReadTransport( buff, 1);
    if(NPIState==1 && *buff==0x7E)
      cnt = 11;
    if(cnt==10 && NPIState==0)
      return 0;
    cnt++;
  }
  NPIState = NPI_ReadTransport( buff+1, 2);
  DataLen = ((uint16)*(buff+1))<<8 + (uint16)*(buff+2);
  NPIState = NPI_ReadTransport( buff+3, DataLen+1);
  return DataLen + 4;
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