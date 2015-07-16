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
  ATComd        =       0x08,           //发送AT指令
  ATQueueComd   =       0x09,          //AT队列参数值
  ZBTransReq    =       0x10,          //ZB发送请求
  ZBAdrFram     =       0x11,           //显示寻址的ZB命令帧         
  RenotATComdReq=       0x17,           //远程AT命令请求
  CreateSourceRoute=    0x21,           //创建源路由
  ATComdResponse=       0x88,           //AT命令应答
  RFModemStatus =       0x8A,           //RF模块状态
  ZBTransStats  =       0x8B,           //ZB发送结果状态标志
  ZBReceivePacket=      0x90,           //ZB接收到RF数据包后，通过UAR转发
  ZBExpRxInd    =       0x91,           //收到信息后，信息发送到UART
  ZBIODataRxInd =       0X92,           //收到IO状态数据后发送到串口
  XBReadInd     =       0x94,           //收到传感器数据后发送到串口
  NodeIdenInd   =       0x95,           //节点识别标识
  RemoteComdResp=       0x97,           //远程命令应答
  OnAirUpdataFirmware=  0xA0,           //无线升级固件
  RouteRecordInd=       0xA1,           //路由记录指示器       
  MTORouteReqInd=       0xA3,           //多对一路由请求标示符
}XBeeAPIFramType;

typedef enum            
{
  
  
}XBeeATComdType;        //AT指令集 看需要添加AT命令

typedef enum
{
  Disable       =       0,
  RSSI_PWM      =       1,
  InputMonitored=       3,
  OutputLow     =       4,
  OutPutHigh    =       5,
  
}ATPramaterRange;       //AT命令参数



//void XBee_APISendATComd(XBeeAPIFramType,)        //发送AT命令集

*/



#endif

