#ifndef __XBee_API_H__
#define __XBEE_API_H__

#include "hal_types.h"

#define XBEE_LED1       0
#define XBEE_LED2       1
#define XBEE_LED3       2
#define XBEE_LED4       3

#ifndef LED_ON
#define LED_ON          1
#endif
#ifndef LED_OFF
#define LED_OFF         !LED_ON
#endif

#ifndef SPEAKER_ON
#define SPEAKER_ON      1
#endif
#ifndef SPEAKER_OFF
#define SPEAKER_OFF     !SPEAKER_ON
#endif


typedef enum
{
  Disable       =       0,
  RSSIPWM       =       1,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}P0Mode;

typedef enum
{
  DigInputM     =       0,
  DigInputUnm   =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}P1P2Mode;

typedef enum
{
  StarButton    =       1,
  AnalogInput   =       2,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D0Mode;

typedef enum
{
  Disable       =       0,
  AnalogInput   =       2,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D1D2D3Mode;

typedef enum
{
  Disable       =       0,
  AnalogInput   =       2,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D2Mode;

typedef enum
{
  Disable       =       0,
  AnalogInput   =       2,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D3Mode;

typedef enum
{
  Disable       =       0,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D4Mode;

typedef enum
{
  Disable       =       0,
  LED           =       1,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D5Mode;

typedef enum
{
  Disable       =       0,
  DigInput      =       3,
  DigOutLow     =       4,
  DigOutHigh    =       5,
}D8Mode;


/*typedef enum
{
  enDisable,
  enSpecial,
  enAnalog,
  enDigitalInput,
  enDigitalOutPutLow,
  enDigitalOutPutHigh,
}*/
/*
typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 *fram_data;
  //uint8 checksum;
}st_api_frame_head,*pst_api_frame_head;

typedef struct
{
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
  uint8 atPara;
}st_api_atCmd,*pst_api_atCmd;

typedef struct
{
  uint8 frame_type;
  uint8 frame_id;
  uint8 dest_long_addr[8];
  uint8 dest_short_addr[2];
  uint8 radius;
  uint8 options;
  uint8 *data;
}st_api_trasmit_data,*pst_api_trasmit_data;

void xbee_api_checksum(uint8 *begin,uint16 length);
void xbee_api_atcmd_set_led(uint8,uint8);
void xbee_api_atcmd_speaker_opt(uint8 state);
void xbee_api_transmit_data_request(uint8 *dest_addr,uint8 *data,uint16 len);
*/

/*
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
}XBeeAPIFramType;

typedef enum            
{
  
  
}XBeeATComdType;        //ATָ� ����Ҫ���AT����

typedef enum
{
  Disable       =       0,
  RSSI_PWM      =       1,
  InputMonitored=       3,
  OutputLow     =       4,
  OutPutHigh    =       5,
  
}ATPramaterRange;       //AT�������



//void XBee_APISendATComd(XBeeAPIFramType,)        //����AT���

*/



#endif

