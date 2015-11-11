/*********************************************************
**����    ��XBee API֡�ṹ
**�汾    ��V1.0
**����    ��2015.07.08
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
/************************************************************
**brief xbee������������
**param adr ָ��64λ��ַ��ָ��
        net_adr ָ��16λ�����ַ��ָ��
        options ������
        rf_data ���������ݵ�ָ��
        len     ���������ݵ��ֽ���
        IsRes   �Ƿ�Ӧ��
**reval ���͵��ֽ���
************************************************************/
uint16 XBeeTransReq(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len, IsResp IsRes)
{
  static uint8 wbuf[128],cnt=0;
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
**brief apiģʽ����ATָ��
**param atcmd  ָ�������ַ�����ָ��
**      pparam ָ�������ָ��
**      len    �����ĳ���
**reval 
**************************************************/
uint16 XBeeSendATCmd(int8* atcmd,uint8* pparam,uint16 len,IsResp IsRes)
{
    static uint8 wbuf[64];
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
#if defined _PRINTF
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
**brief ����IO��״̬
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
**biref ����ID��ֵ
**********************************************************/
uint16 XBeeSetPanID(uint8 *panID,IsResp IsRes)
{
  int8 *cmd = "ID";
  return XBeeSendATCmd(cmd,panID,8,IsRes);
}
/*********************************************************
**biref ����ZS��ֵ
**********************************************************/
uint16 XBeeSetZS(uint8 data,IsResp IsRes)
{
  uint8 param[1];
  int8 *cmd = "ZS";
  *param = data;
  return XBeeSendATCmd(cmd,param,1,IsRes);
}

/*********************************************************
**biref ���Ͷ�ȡIDֵ����
**********************************************************/
uint16 XBeeReadPanID(void)
{
  uint8 panID[1];
  int8 *cmd = "OP";
  return XBeeSendATCmd(cmd,panID,0,RES);
}
/*********************************************************
**biref ����AI����
**********************************************************/
uint16 XBeeReadAI(void)
{
  uint8 paramer[1];
  int8 *cmd = "AI";
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref ����SP���� 
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
**biref ����SN���� 
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
**biref ����ST���� 
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
**biref ����MY����
**********************************************************/
uint16 XBeeReadMY(void)
{
  uint8 paramer[1];
  int8 *cmd = "MY";
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*************************************************************
**brief  �����ŵ�
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
**brief  ����SD
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
**brief ����/��������ָʾ����˸ʱ��
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
**brief  ��ȡ�ŵ�
*************************************************************/
uint16 XBeeReadCH(void)
{
  uint8 paramer[8];
  int8 *cmd = "CH";
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*************************************************************
**brief ��λģ��
*************************************************************/
uint16 XbeeRunFR(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "FR";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief ʹ�ܸ�������
*************************************************************/
uint16 XbeeRunAC(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "AC";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref ����WR����,�������
**********************************************************/
uint16 XBeeRunWR(IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "WR";
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref ����NJ����
**********************************************************/
uint16 XBeeSetNJ(uint8 time, IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "NJ";
  paramer[0]=time;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref ����SH����
**********************************************************/
uint16 XBeeReadSH()
{
  uint8 paramer[1];
  int8 *cmd = "SH";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref ���Ͷ�ȡDL����
**********************************************************/
uint16 XBeeReadDL()
{
  uint8 paramer[1];
  int8 *cmd = "DL";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref ����SL����  ����λ��ַ
**********************************************************/
uint16 XBeeReadSL()
{
  uint8 paramer[1];
  int8 *cmd = "SL";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref ����NO
**********************************************************/
uint16 XBeeSetNO(uint8 num,IsResp IsRes)
{
    uint8 paramer[1];
    int8 *cmd = "NO";
    paramer[0]=num;
    return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref ����SM���� ��������
**********************************************************/
uint16 XBeeSetSM(SleepType sleep,IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "SM";
  paramer[0]=sleep;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref ����SP���� ����ʱ��
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
**biref ����SN���� ����ʱ�����
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
**biref ����ST���� ����ʱ��
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
**biref ���Ͷ�ȡSM���� �������ֵ�ǰģ������
**********************************************************/
uint16 XBeeReadSM(void)
{
  uint8 paramer[1];
  int8 *cmd = "SM";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/***********************************************************
**brief ����NT
***********************************************************/
uint16 XBeeSetNT(uint8 num,IsResp IsRes)
{
    uint8 paramer[1];
    int8 *cmd = "NT";
    paramer[0]=num;
    return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref ���Ͷ�ȡAT��������
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
**biref ��������AT��������
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
**biref ����AT��������
**********************************************************/
uint16 XBeeSetAT(int8 *at_cmd, uint8 *param, uint8 len, IsResp IsRes)
{
    int8 *cmd;
	cmd = at_cmd;
  	return XBeeSendATCmd(cmd,param,len,IsRes);	
}
/********************************************************
**brief ��������
********************************************************/
uint16 XBeeUnicastTrans(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes)
{
	return XBeeTransReq(adr,net_adr,options,rf_data,len,IsRes); 
}
/***********************************************************
**brief ��coordinator��������
**reval ���͵��ֽ���
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
**brief ��coordinator��������,����ȷ��ַ
**reval ���͵��ֽ���
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
**brief ���͹㲥
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
**brief ����Դ·������
**param mac_adr Ŀ��������ַ
		net_adr Ŀ�������ַ ���� 0xEEFF ��Ӧ�����ַ 0xEE 0xFF
		num Ŀ���뷢���߼�Ľڵ�����
		mid_adr ָ���м�ڵ������ַ��ָ�� 
			���� �ӷ�����A��Ŀ��E�Ľڵ�˳��Ϊ A B C D E
			�м�ڵ�������ַ	B	0xAABB
							C	0xCCDD
							D	0xEEFF
			�������������˳��Ϊ	EE FF CC DD AA BB  
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
**brief ��chencksum��
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





















