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
  static uint8 wbuf[256],cnt=0;
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
  XBeeMode5Wake();
  return NPI_WriteTransport((uint8*)frame,18+len);
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
  uint8 wbuf[128],i;
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
  XBeeMode5Wake();
  return NPI_WriteTransport((uint8*)cmd,8+len);
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
  XBeeMode5Wake();
  NPI_WriteTransport((uint8*)cmd,9);
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
uint16 XBeeSetZS(IsResp IsRes)
{
  uint8 panID[8];
  int8 *cmd = "ZS";
  *panID = 1;
  return XBeeSendATCmd(cmd,panID,1,IsRes);
}

/*********************************************************
**biref ���Ͷ�ȡIDֵ����
**********************************************************/
uint16 XBeeReadPanID(IsResp IsRes)
{
  uint8 panID[1];
  int8 *cmd = "OP";
  return XBeeSendATCmd(cmd,panID,0,IsRes);
}
/*********************************************************
**biref ����AI����
**********************************************************/
uint16 XBeeReadAI(IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "AI";
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
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
/*********************************************************
**biref ����MY����
**********************************************************/
uint16 XBeeReadMY(IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "MY";
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief  �����ŵ�
*************************************************************/
uint16 XBeeSetChannel(IsResp IsRes)
{
  uint8 paramer[8],i=1;
  int8 *cmd = "SC";
  for(i=0;i<8;i++)
     *(paramer+i) = 0x0B;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
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
uint16 XBeeReadCH(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "CH";
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief ��λģ��
*************************************************************/
uint16 XbeeFR(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "FR";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief ʹ�ܸ�������
*************************************************************/
uint16 XbeeSendAC(IsResp IsRes)
{
  uint8 paramer[8];
  int8 *cmd = "AC";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref ����WR����,�������
**********************************************************/
uint16 XBeeSendWR(IsResp IsRes)
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
**biref ����SM���� ��������
**********************************************************/
uint16 XBeeSendSM(SleepType sleep,IsResp IsRes)
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
/**********************************************************
**brief ��ȡ�Ĵ���ֵATָ��
**********************************************************/
uint16 XBeeReadRegCmd(int8 *atcmd)
{
    uint8 paramer[1];
    int8 cmd[2];
    *cmd = *atcmd;
    *(cmd+1) = *(atcmd+1);
    paramer[0]=0;
    return XBeeSendATCmd(cmd,paramer,0,RES);
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

/********************************************************
**brief ��������
********************************************************/
uint16 XBeeUnicastTrans(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes)
{
	return XBeeTransReq(adr,net_adr,options,rf_data,len,IsRes); 
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





















