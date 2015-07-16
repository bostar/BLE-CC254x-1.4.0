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

#if defined _XBEE_APP_

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

}
/************************************************************
**brief ��Ѱ�ڵ�
**
************************************************************/

/************************************************************
**brief �����ŵ�
**
************************************************************/

/************************************************************
**brief Զ����������
**
************************************************************/

/************************************************************
**brief Զ������Ӧ��
**
************************************************************/

/************************************************************
**brief ��������Ӧ��
**
************************************************************/

/************************************************************
**brief Զ����������
**
************************************************************/


static uint8 wbuf[255];
/*******************************************************************
**brief �򿪷�����
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
**brief �رշ�����
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





















