#ifndef _XBee_At_Cmd_h
#define _XBee_At_Cmd_h

#include "hal_types.h"

typedef enum
{
  ATComd        =       0x08,           //����ATָ��
  ATQueueComd   =       0x09,          //AT���в���ֵ
  ZBTransReq    =       0x10,          //ZB��������
  ZBAdrFram     =       0x11,           //��ʾѰַ��ZB����֡         
  RenotATComdReq=       0x17,           //Զ��AT��������
  CreateSourceRoute=    0x21,           //����Դ·��
  ATComdResponse=       0x88,           //AT����Ӧ��
  RFModemStatus =       0x8A,           //RFģ��״̬
  ZBTransStats  =       0x8B,           //ZB���ͽ��״̬��־
  ZBReceivePacket=      0x90,           //ZB���յ�RF���ݰ���ͨ��UARת��
  ZBExpRxInd    =       0x91,           //�յ���Ϣ����Ϣ���͵�UART
  ZBIODataRxInd =       0X92,           //�յ�IO״̬���ݺ��͵�����
  XBReadInd     =       0x94,           //�յ����������ݺ��͵�����
  NodeIdenInd   =       0x95,           //�ڵ�ʶ���ʶ
  RemoteComdResp=       0x97,           //Զ������Ӧ��
  OnAirUpdataFirmware=  0xA0,           //���������̼�
  RouteRecordInd=       0xA1,           //·�ɼ�¼ָʾ��       
  MTORouteReqInd=       0xA3,           //���һ·�������ʾ��
}TypeXBeeAPIFram;


typedef enum
{
  Response      =       0x52,
  NoResponse    =       0x00,
}IsResp;        //Ӧ��ģʽ

typedef enum
{
  D0    =       0x4430,         //DIO0
}ATCommand;     //AT���

typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
}XBeeApiATCmdType; 

  
typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
  uint8 atPara;
//  uint8 CheckSum;
}TypeAPICmd;    //API͸������֡

typedef struct
{
  uint8 a;     //������ ������
}TypeAPIEsCmd;  //APIת������֡

typedef struct       //IO��APIû����֡
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
  uint8 param;
  uint8 checksum;
}XBeeApiIOCmd;
typedef enum    
{
  IO_P0    =       1,
  IO_P1    =       2,
  IO_P2    =       3,
  IO_P3    =       4,
  IO_D0    =       5,
  IO_D1    =       6,
  IO_D2    =       7,
  IO_D3    =       8,
  IO_D4    =       9,
  IO_D5    =      10,
  IO_D6    =      11,
  IO_D7    =      12,
  IO_D8    =      13 
}XBeeIOParam;

typedef enum
{
  High  =  1,
  Low   =  2
}IOStatus;



uint8 XBeeApiChecksum(uint8 *begin,uint16 length);
void XBeeSetIO(XBeeIOParam ioparam,IOStatus state);       //����IO��״̬
int XBeeSendATCmd(int8* atcmd,uint8* pparam,uint8 len);   //����ztָ��
int XBeeSetPanID(void);   //����ID��ֵ
int XBeeReadPanID(void);  //��ȡID
int XBeeSetChannel(void); //�����ŵ�
int xbeeFR(void);  //
int XBeeReadAI(void);
int XBeeSendWR(void);
int XBeeSendMY(void);
int XBeeSetLT(uint8 time);
int XBeeReadCH(void);
int XbeeSendAC(void);                                    //����AI����







#endif

