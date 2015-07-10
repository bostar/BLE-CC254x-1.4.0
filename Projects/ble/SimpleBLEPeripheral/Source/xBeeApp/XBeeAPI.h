#ifndef __XBee_API_H__
#define __XBEE_API_H__



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
  ena,
  
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





#endif

