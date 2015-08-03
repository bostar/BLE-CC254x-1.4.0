#ifndef _XBee_At_Cmd_h
#define _XBee_At_Cmd_h

#include "hal_types.h"

typedef enum
{
  ModemStatus     =     0x8A,
  ATCmdRep        =     0x88,
  XBeeRevPacket   =     0x90,
  XBeeTransStatus =     0x8B,
  
}FrameTypeStateType;

typedef enum
{
  Disable     =     0,
  PinSleep    =     1,
  CycSleep    =     4,
  PinCyc      =     5,
}SleepType;

typedef enum
{
  RES       =       0x52,
  NO_RES    =       0x00,
}IsResp;        //应答模式

typedef enum
{
  Default       =  0,
  DisableACK    =  0x01,
  EnableAPS     =  0x20,
  ExtTimeout    =  0x40,
}SetOptions;

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
}XBeeApiATCmdType;  //AT指令帧

typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 adr[8];
  uint8 net_adr[2];
  uint8 readius;
  uint8 options;
}XBeeTransReqType;  //zb发送请求帧
  
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

typedef struct       //IO口API没命令帧
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
void XBeeSetIO(XBeeIOParam ioparam,IOStatus state);       //设置IO口状态
uint16 XBeeSendATCmd(int8* atcmd,uint8* pparam,uint16 len,IsResp IsRes);   //发送zt指令
uint16 XBeeSetPanID(uint8 *panID,IsResp IsRes);   //设置ID的值
uint16 XBeeReadPanID(IsResp IsRes);  //读取ID
uint16 XBeeSetChannel(IsResp IsRes); //设置信道
uint16 XbeeFR(IsResp IsRes);  //
uint16 XBeeReadAI(IsResp IsRes);
uint16 XBeeSendWR(IsResp IsRes);
uint16 XBeeReadMY(IsResp IsRes);
uint16 XBeeReadCH(IsResp IsRes);
uint16 XbeeSendAC(IsResp IsRes);      
uint16 XBeeSetNJ(uint8 time, IsResp IsRes);
uint16 XBeeSetLT(uint8 time,IsResp IsRes);
uint16 XBeeSetZS(IsResp IsRes);
uint16 XBeeReadSH();
uint16 XBeeReadSL();
uint16 XBeeSendSM(SleepType sleep,IsResp IsRes);
uint16 XBeeReadSM(void);
uint16 XBeeSetSP(uint16 num,IsResp IsRes);
uint16 XBeeSetSN(uint16 num,IsResp IsRes);
uint16 XBeeSetST(uint16 num,IsResp IsRes);
uint16 XBeeTransReq(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len, IsResp IsRes); //xbee发送数据请求
uint16 XBeeSendToCoor(uint8 *data,uint16 len,IsResp IsRes);  //向coordinator发送数据
uint16 XBeeUnicastTrans(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes);
uint16 XBeeBoardcastTrans(uint8 *data,uint16 len,IsResp IsRes);


#endif

