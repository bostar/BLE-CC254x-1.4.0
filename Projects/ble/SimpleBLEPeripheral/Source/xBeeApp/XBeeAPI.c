/*********************************************************
**描述    ：XBee API接口函数
**版本    ：V1.0
**日期    ：2015.07.08
*********************************************************/

#include "XBeeAPI.h"
#include "hal_types.h"
#include "npi.h"

void xbee_api_checksum(uint8 *begin,uint16 length)
{
  uint8 sum = 0;
  for(int loop = 0;loop < length;loop ++)
  {
    sum += begin[loop];
  }
  begin[length] = 0xff - sum;
}


void xbee_api_atcmd_set_led(uint8 state)
{
  pst_api_frame_head head = (pst_api_frame_head)xbee_wbuf;
  pst_api_atCmd atCmd;
  head->start_delimiter = 0x7e;
  head->len_msb = 0x00;
  head->len_lsb = 0x05;
  atCmd = (pst_api_atCmd)&head->fram_data;
  atCmd->frame_type = 0x08;
  atCmd->frame_id = 0x52;
  atCmd->atCmd[0] = 'P';
  atCmd->atCmd[1] = '1';
  atCmd->atPara = 5;
  xbee_api_checksum((uint8*)atCmd,5);
  NPI_WriteTransport(xbee_wbuf, 9);
}

XBee_APISendATComd(XBeeAPIFramType,)        //发送AT命令帧
{
  
}












