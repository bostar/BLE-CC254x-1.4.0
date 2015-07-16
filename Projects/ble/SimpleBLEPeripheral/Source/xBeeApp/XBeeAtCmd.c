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

/************************************************
**brief ����API��ʽ��AT����֡
**param FramID  ����֡�Ƿ���ҪӦ�𣬲�����IsResp��ȡֵ
**      ATCmd   Ҫ���͵�AT����
**      status  ���λ�Ƿ����������ݣ�1�����������0��ѯ�������ֵ
                ��7λΪ������������ĳ���
**      CmdData ָ�������������ָ�� ���ݳ�����0-0x7f���ֽ�֮��
**reval none
************************************************/
void API_AtCmd(IsResp FramID,ATCommand InATCmd,uint8 status,uint8 *CmdData)     //����AT����֡������&��ȡXBee��Ϣ
{
  uint8 CmdDataLen=0,APICmdLen=0;
  uint16 DataLen=0;
//  uint8 *XBeeWBuff;
  TypeAPICmd *APICmd;
  if((status & 0x80)==0x80)
    CmdDataLen = status & 0x7f;
  else
    CmdDataLen = 0;
  APICmdLen = CmdDataLen+8;

  APICmd = (TypeAPICmd*)malloc(APICmdLen * sizeof(uint8));
  APICmd->start_delimiter = 0x7e;
  DataLen = CmdDataLen + 4;
  APICmd->len_msb = (uint8)(DataLen >> 8);
  APICmd->len_lsb = (uint8)DataLen;
  APICmd->frame_type=ATComd;
  APICmd->frame_id= FramID;
  APICmd->atCmd[0]= (uint8)(InATCmd >> 8);
  APICmd->atCmd[1]= (uint8)InATCmd;
  if(CmdDataLen==0)
    xbee_api_checksum((uint8*)APICmd,APICmdLen);
  else
  {
    for(uint8 i=0;i<CmdDataLen;i++)
      *((&APICmd->atPara)+i) = CmdData[i];
    xbee_api_checksum((uint8*)APICmd,APICmdLen);
  }
  NPI_WriteTransport((uint8*)APICmd,APICmdLen);
  free(APICmd);
}
  

void xbee_api_checksum(uint8 *begin,uint16 length)
{
  uint8 sum = 0;
  for(int loop = 0;loop < length-1;loop ++)
  {
    sum += begin[loop];
  }
  begin[length-1] = 0xff - sum;
}





























