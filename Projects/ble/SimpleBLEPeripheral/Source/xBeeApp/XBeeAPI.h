#ifndef __XBee_API_H__
#define __XBEE_API_H__



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
  ena,
  
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





#endif

