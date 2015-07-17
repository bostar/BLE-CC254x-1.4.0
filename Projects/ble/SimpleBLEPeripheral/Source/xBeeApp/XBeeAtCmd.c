/*********************************************************
**描述    ：XBee API帧结构
**版本    ：V1.0
**日期    ：2015.07.08
*********************************************************/


#include "XBeeAtCmd.h"
#include "npi.h"
#include "OSAL.h"
#include "string.h"
#include <stdlib.h>
#include "OnBoard.h"

#if defined _XBEE_APP_

/************************************************************
**brief 搜寻节点
**
************************************************************/

/************************************************************
**brief 设置信道
**
************************************************************/

/************************************************************
**brief 远程命令请求
**
************************************************************/

/************************************************************
**brief 远程命令应答
**
************************************************************/

/************************************************************
**brief 本地命令应答
**
************************************************************/

/************************************************************
**brief 远程命令请求
**
************************************************************/

/***********************************************************
**brief 设置IO口状态
**param
**reval
***********************************************************/
void XBeeSetIO(XBeeIOParam ioparam,IOStatus state)
{
  uint8 wbuf[9];
  XBeeApiIOCmd *cmd = (XBeeApiIOCmd*)wbuf;
  cmd->start_delimiter  = 0x7E;
  cmd->len_msb          = 0x00;
  cmd->len_lsb          = 0x05;
  cmd->frame_type       = 0x08;
  cmd->frame_id         = 0x52;
  switch(ioparam)
  {
    case IO_P0:
      cmd->atCmd[0]         = 'P';
      cmd->atCmd[1]         = '0';
      break;
    case IO_P1:
      cmd->atCmd[0]         = 'P';
      cmd->atCmd[1]         = '1';
      break;
    case IO_P2:
      cmd->atCmd[0]         = 'P';
      cmd->atCmd[1]         = '2';
      break;
    case IO_P3:
      cmd->atCmd[0]         = 'P';
      cmd->atCmd[1]         = '3';
      break;
    case IO_D0:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '0';
      break;
    case IO_D1:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '1';
      break;
    case IO_D2:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '2';
      break;
    case IO_D3:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '3';
      break;
    case IO_D4:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '4';
      break;
    case IO_D5:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '5';
      break;
    case IO_D6:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '6';
      break;
    case IO_D7:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '7';
      break;
    case IO_D8:
      cmd->atCmd[0]         = 'D';
      cmd->atCmd[1]         = '8';
      break;
  }
  
  cmd->param   = (state == Low)?4:5;
  //cmd->checksum = 0xff- i;
  cmd->checksum = XBeeApiChecksum(((uint8*)cmd)+3,5);
  NPI_WriteTransport((uint8*)cmd,9);
}






uint8 XBeeApiChecksum(uint8 *begin,uint16 length)
{
  uint8 sum = 0,i = 0;
  for(i = 0;i < length;i++)
  {
    sum += begin[i];
  }
  i = 0xff - sum;
  return i;
}







#endif





















