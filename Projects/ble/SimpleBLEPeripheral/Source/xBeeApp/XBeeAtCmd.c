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
#include "XBeeBsp.h"
#include <stdio.h>

#if defined _XBEE_APP_
static uint8 wbuf[50];
/************************************************************
**brief xbee发送数据请求
**param adr 指向64位地址的指针
        net_adr 指向16位网络地址的指针
        options 配置字
        rf_data 待发送数据的指针
        len     待发送数据的字节数
        IsRes   是否应答
**reval 发送的字节数
************************************************************/
uint16 XBeeTransReq(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len, IsResp IsRes)
{
  uint8 cnt=0;
  uint16 reval;
  XBeeTransReqType *frame = (XBeeTransReqType*)wbuf;
  
  frame->start_delimiter = 0x7E;
  frame->len_msb         = (uint8)((14+len)>>8);
  frame->len_lsb         = (uint8)(14+len);
  frame->frame_type      = 0x10;
  frame->frame_id        = IsRes;
  for(cnt=0;cnt<8;cnt++)
    frame->adr[cnt] = *(adr + cnt);
  frame->net_adr[0]      = *(net_adr);
  frame->net_adr[1]      = *(net_adr+1);
  frame->readius         = 0;
  frame->options         = options;
  for(cnt=0;cnt<len;cnt++)
    *((uint8*)frame + 17 + cnt) = *(rf_data + cnt);
  *(((uint8*)frame)+17+len) = XBeeApiChecksum(((uint8*)frame)+3,14+len);
  XBeePinWake();
  reval = NPI_WriteTransport((uint8*)frame,18+len);
  XBeePinSleep();
  return reval;
}
/**************************************************
**brief api模式发送AT指令
**param atcmd  指向命令字符串的指针
**      pparam 指向参数的指针
**      len    参数的长度
**reval 
**************************************************/
uint16 XBeeSendATCmd(int8* atcmd,uint8* pparam,uint16 len,IsResp IsRes)
{
    uint8 i;
    uint16 reval;
    XBeeApiATCmdType *cmd = (XBeeApiATCmdType*)wbuf;
    cmd->start_delimiter  = 0x7E;
    cmd->len_msb          = (uint8)((4+len)>>8);
    cmd->len_lsb          = (uint8)(4+len);
    cmd->frame_type       = 0x08;
    cmd->frame_id         = IsRes;
    cmd->atCmd[0]         = *atcmd;
    cmd->atCmd[1]         = *(atcmd+1);
    for(i=0;i<len;i++)
        *(((uint8*)cmd)+7+i) = *(pparam+i);
    *(((uint8*)cmd)+7+len) = XBeeApiChecksum(((uint8*)cmd)+3,4+len); 
#if 0
    printf("send AT command:\n");
    for(i=0;i<8+len;i++)
        printf("0x%02x ",wbuf[i]);
    printf("\n");
#endif
    XBeePinWake();
    reval = NPI_WriteTransport((uint8*)cmd,8+len);
    XBeePinSleep();
    return reval;
}


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
  cmd->frame_id         = 0x00;
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
    default:
      break;  
  }
  cmd->param   = (state == Low)?4:5;
  //cmd->checksum = 0xff- i;
  cmd->checksum = XBeeApiChecksum(((uint8*)cmd)+3,5);
  XBeePinWake();
  NPI_WriteTransport((uint8*)cmd,9);
  XBeePinSleep();
}

/*********************************************************
**biref 设置ID的值
**********************************************************/
uint16 XBeeSetPanID(uint8 *panID,IsResp IsRes)
{
  int8 *cmd = "ID";
  return XBeeSendATCmd(cmd,panID,8,IsRes);
}
/*********************************************************
**biref 设置ZS的值
**********************************************************/
uint16 XBeeSetZS(uint8 data,IsResp IsRes)
{
  uint8 param[1];
  int8 *cmd = "ZS";
  *param = data;
  return XBeeSendATCmd(cmd,param,1,IsRes);
}

/*********************************************************
**biref 发送读取ID值命令
**********************************************************/
uint16 XBeeReadPanID(void)
{
  uint8 panID[1];
  int8 *cmd = "OP";
  return XBeeSendATCmd(cmd,panID,0,RES);
}
/*********************************************************
**biref 发送AI命令
**********************************************************/
uint16 XBeeReadAI(void)
{
  uint8 paramer[1];
  int8 *cmd = "AI";
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送SP命令 
**********************************************************/
uint16 XBeeSetSP(uint16 num,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SP";
  paramer[0] =(uint8)(num>>8);
  paramer[1] = (uint8)num;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送SN命令 
**********************************************************/
uint16 XBeeSetSN(uint16 num,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SN";
  paramer[0] =(uint8)(num>>8);
  paramer[1] = (uint8)num;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送ST命令 
**********************************************************/
uint16 XBeeSetST(uint16 num,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "ST";
  paramer[0] =(uint8)(num>>8);
  paramer[1] = (uint8)num;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
uint16 XBeeSetAR(uint8 num,IsResp IsRes)
{
    uint8 paramer[1];
    int8 *cmd = "AR";
    paramer[0] = num;
    return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送MY命令
**********************************************************/
uint16 XBeeReadMY(void)
{
  uint8 paramer[1];
  int8 *cmd = "MY";
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*************************************************************
**brief  设置信道
*************************************************************/
uint16 XBeeSetChannel(uint16 channel,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SC";
  *paramer     = (uint8)(channel >> 8);
  *(paramer+1) = (uint8)(channel);
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*************************************************************
**brief  设置SD
**param  time  0-7
*************************************************************/
uint16 XBeeSetSD(uint8 time,IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "SD";
  *paramer     = time;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*************************************************************
**brief 连接/创建网络指示灯闪烁时间
**param time   it should be 0x0A-0xFF or 0x00
*************************************************************/
uint16 XBeeSetLT(uint8 time,IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "LT";
  *paramer = time;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*************************************************************
**brief  读取信道
*************************************************************/
uint16 XBeeReadCH(void)
{
  uint8 paramer[8];
  int8 *cmd = "CH";
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*************************************************************
**brief 复位模块
*************************************************************/
uint16 XbeeRunFR(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "FR";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief 使能更改内容
*************************************************************/
uint16 XbeeRunAC(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "AC";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref 发送WR命令,保存更改
**********************************************************/
uint16 XBeeRunWR(IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "WR";
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref 发送NJ命令
**********************************************************/
uint16 XBeeSetNJ(uint8 time, IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "NJ";
  paramer[0]=time;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送SH命令
**********************************************************/
uint16 XBeeReadSH()
{
  uint8 paramer[1];
  int8 *cmd = "SH";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送读取DL命令
**********************************************************/
uint16 XBeeReadDL()
{
  uint8 paramer[1];
  int8 *cmd = "DL";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送SL命令  低四位地址
**********************************************************/
uint16 XBeeReadSL()
{
  uint8 paramer[1];
  int8 *cmd = "SL";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 设置NO
**********************************************************/
uint16 XBeeSetNO(uint8 num,IsResp IsRes)
{
    uint8 paramer[1];
    int8 *cmd = "NO";
    paramer[0]=num;
    return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送SM命令 休眠设置
**********************************************************/
uint16 XBeeSetSM(SleepType sleep,IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "SM";
  paramer[0]=sleep;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送SP命令 休眠时间
**********************************************************/
uint16 XBeeSendSP(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SP";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送SN命令 休眠时间个数
**********************************************************/
uint16 XBeeSendSN(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SN";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送ST命令 唤醒时间
**********************************************************/
uint16 XBeeSendST(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "ST";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送读取SM命令 用以区分当前模块类型
**********************************************************/
uint16 XBeeReadSM(void)
{
  uint8 paramer[1];
  int8 *cmd = "SM";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/***********************************************************
**brief 发送NT
***********************************************************/
uint16 XBeeSetNT(uint8 num,IsResp IsRes)
{
    uint8 paramer[1];
    int8 *cmd = "NT";
    paramer[0]=num;
    return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送读取AT参数命令
**********************************************************/
uint16 XBeeReadAT(int8 *at_cmd)
{
	uint8 paramer[1];
  	int8 *cmd;
	cmd = at_cmd;
  	paramer[0]=0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);	
}
/*********************************************************
**biref 发送设置AT参数命令
**********************************************************/
uint16 XBeeSendAT(int8 *at_cmd)
{
    uint8 paramer[1];
  	int8 *cmd;
	cmd = at_cmd;
  	paramer[0]=0;
  	return XBeeSendATCmd(cmd,paramer,0,NO_RES);	
}
/*********************************************************
**biref 设置AT参数命令
**********************************************************/
uint16 XBeeSetAT(int8 *at_cmd, uint8 *param, uint8 len, IsResp IsRes)
{
    int8 *cmd;
	cmd = at_cmd;
  	return XBeeSendATCmd(cmd,param,len,IsRes);	
}
/********************************************************
**brief 单播发送
********************************************************/
uint16 XBeeUnicastTrans(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes)
{
	return XBeeTransReq(adr,net_adr,options,rf_data,len,IsRes); 
}
/***********************************************************
**brief 向coordinator发送数据
**reval 发送的字节数
***********************************************************/
uint16 XBeeSendToCoor(uint8 *data,uint16 len,IsResp IsRes)
{
    uint8 adr[8],net_adr[2],cnt;
    for(cnt=0;cnt<8;cnt++)
        adr[cnt] = 0;
    net_adr[0] = 0xFF;
    net_adr[1] = 0xFE;
    return XBeeTransReq(adr, net_adr, Default, data, len, IsRes);
}
/***********************************************************
**brief 向coordinator发送数据,带明确地址
**reval 发送的字节数
***********************************************************/
uint16 XBeeSendToCoorByMac(uint8 *data,uint16 len,IsResp IsRes)
{
    uint8 adr[8],net_adr[2],cnt;
    for(cnt=0;cnt<8;cnt++)
        adr[cnt] = CoorMAC[cnt];
    net_adr[0] = 0;
    net_adr[1] = 0;
    return XBeeTransReq(adr, net_adr, Default, data, len, IsRes);
}
/********************************************************
**brief 发送广播
********************************************************/
uint16 XBeeBoardcastTrans(uint8 *data,uint16 len,IsResp IsRes)
{
	uint8 adr[8],net_adr[2],cnt;
	for(cnt=0;cnt<8;cnt++)
    	adr[cnt] = 0;
	adr[6]     = 0xFF;
	adr[7]     = 0xFF;
	net_adr[0] = 0xff;
	net_adr[1] = 0xfe;
	return XBeeTransReq(adr,net_adr,ExtTimeout,data,len,IsRes);
}
/*********************************************************
**brief 发送源路由请求
**param mac_adr 目标的物理地址
		net_adr 目标网络地址 比如 0xEEFF 对应网络地址 0xEE 0xFF
		num 目标与发送者间的节点数量
		mid_adr 指向中间节点网络地址的指针 
			比如 从发送者A到目标E的节点顺序为 A B C D E
			中间节点的网络地址	B	0xAABB
							C	0xCCDD
							D	0xEEFF
			输入参数的排列顺序为	EE FF CC DD AA BB  
*********************************************************/
/*
int16 XBeeCreatSourceRout(CoorRoutePathType CoorRoutePath)
{
	static uint8 wbuf_temp[128],wbuf_len=0,i=0;
	uint16 lenth=0;
	
	wbuf_len = 18 + CoorRoutePath.num_mid_adr * 2;
	lenth = wbuf_len - 4;
	*(wbuf_temp) = 0x7E;		
	*(wbuf_temp + 1) = (uint8)(lenth >> 8);
	*(wbuf_temp + 2) = (uint8)lenth;
	*(wbuf_temp + 3) = 0x21;
	*(wbuf_temp + 4) = 0;
	for(i=0;i<8;i++)
		*(wbuf_temp + i +5) = CoorRoutePath.mac_adr[i];
	*(wbuf_temp + 13) = CoorRoutePath.net_adr[0];
	*(wbuf_temp + 14) = CoorRoutePath.net_adr[1];
	*(wbuf_temp + 15) = 0;
	*(wbuf_temp + 16) = CoorRoutePath.num_mid_adr;  	
	for(i=0;i<CoorRoutePath.num_mid_adr*2;i++)
		 *(wbuf_temp + 17 + i) = CoorRoutePath.num_mid_adr[i];
	*(wbuf_temp + wbuf_len-1) = 0;
	for(i=3;i<wbuf_len-1;i++)
		*(wbuf_temp + wbuf_len-1) += *(wbuf_temp + i);
	*(wbuf_temp + wbuf_len-1) = 0xff - *(wbuf_temp + wbuf_len-1);
    XBeePinWake();
	return WriteComPort(wbuf_temp,wbuf_len);
}
*/
/************************************************************
**brief 求chencksum和
************************************************************/
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





















