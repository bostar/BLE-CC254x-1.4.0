#ifndef _XBee_At_Cmd_h
#define _XBee_At_Cmd_h

#include "hal_types.h"

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
}TypeXBeeAPIFram;


typedef enum
{
  Response      =       0x52,
  NoResponse    =       0x00,
}IsResp;        //应答模式

typedef enum
{
  D0    =       0x4430,         //DIO0
}ATCommand;     //AT命令集

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
}TypeAPICmd;    //API透传命令帧

typedef struct
{
  uint8 a;     //无意义 调试用
}TypeAPIEsCmd;  //API转义命令帧


extern void API_AtCmd(IsResp FramID,ATCommand InATCmd,uint8 status,uint8 *CmdData);     //发送AT命令帧，配置&读取XBee信息
extern void xbee_api_checksum(uint8 *begin,uint16 length);










#endif

