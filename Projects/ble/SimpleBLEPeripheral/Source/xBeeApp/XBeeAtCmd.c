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

/************************************************
**brief 发送API格式的AT命令帧
**param FramID  命令帧是否需要应答，参数在IsResp内取值
**      ATCmd   要发送的AT命令
**      status  最高位是否发送命令内容，1有命令参数，0查询该命令的值
                低7位为发送命令参数的长度
**      CmdData 指向发送命令参数的指针 数据长度在0-0x7f个字节之间
**reval none
************************************************/
void API_AtCmd(IsResp FramID,ATCommand InATCmd,uint8 status,uint8 *CmdData)     //发送AT命令帧，配置&读取XBee信息
{

}
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


static uint8 wbuf[255];
/*******************************************************************
**brief 打开蜂鸣器
**param none        
**reval none
*******************************************************************/
void XBeeOpenBuzzer()
{
  uint8 i;
  XBeeApiIOCmd *cmd = (XBeeApiIOCmd*)wbuf;
  cmd->start_delimiter  = 0x7E;
  cmd->len_msb          = 0x00;
  cmd->len_lsb          = 0x05;
  cmd->frame_type       = 0x08;
  cmd->frame_id         = 0x52;
  cmd->atCmd[0]         = 'P';
  cmd->atCmd[1]         = '1';
  cmd->param            = 0x05;
  i = cmd->frame_type + cmd->frame_id + cmd->atCmd[0] + cmd->atCmd[1] + cmd->param;
  cmd->checksum = 0xff- i;
  NPI_WriteTransport((uint8*)cmd,9);
}
/*******************************************************************
**brief 关闭蜂鸣器
**param none       
**reval none
*******************************************************************/
void XBeeCloseBuzzer()
{
  uint8 i;
  XBeeApiIOCmd *cmd = (XBeeApiIOCmd*)wbuf;
  cmd->start_delimiter  = 0x7E;
  cmd->len_msb          = 0x00;
  cmd->len_lsb          = 0x05;
  cmd->frame_type       = 0x08;
  cmd->frame_id         = 0x52;
  cmd->atCmd[0]         = 'P';
  cmd->atCmd[1]         = '1';
  cmd->param            = 0x04;
  i = cmd->frame_type + cmd->frame_id + cmd->atCmd[0] + cmd->atCmd[1] + cmd->param;
  cmd->checksum = 0xff- i;
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





















