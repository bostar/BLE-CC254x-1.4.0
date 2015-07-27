#ifndef __XBSP_H__
#define __XBSP_H__

#include "XBeeApp.h"

#define XBee_GPIO_DIR        0x7C
#define BEEPON         (unsigned char)(0x01 << 2)
#define BEEPOFF        (unsigned char)(0x00)
#include "hal_types.h"

extern FlagJionNetType FlagJionNet;


void ClearDMA(void);
void initXBeeBsp(void);
void setBeepOn(void);
void setBeepOff(void);
void setLedBit(unsigned char bits);
uint16 XBeeUartRead(uint8* buff);
void XBeeRourerJoinNet(void);  //加入网络
void XBeeLeaveNet(void);       //离开网络
uint16 XBeeReqJionPark(void);  // 加入停车场
#endif
