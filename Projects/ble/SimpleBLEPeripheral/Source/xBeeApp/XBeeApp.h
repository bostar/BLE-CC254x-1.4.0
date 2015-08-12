#ifndef __XBEEAPP_H
#define __XBEEAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "Hal_types.h"
/*********************************************************************
 * CONSTANTS
 */


// Simple BLE Peripheral Task Events
#define XBEE_START_DEVICE_EVT                              0x0001
#define XBEE_JOIN_NET_EVT                                  0x0002
#define XBEE_CTL_MCU_UART_READ_EVT                         0x0004    
#define XBEE_PARAM_INIT_EVT                                0x0008
#define XBEE_REC_DATA_PROCESS_EVT                          0x0010
#define XBEE_TEST_EVT                                      0x0020
#define XBEE_SLEEP_EVT                                     0x0040
#define XBEE_IDLE_EVT                                      0x0080
#define XBEE_MOTOO_CTL_EVT                                 0x0100
#define XBEE_HMC5983_EVT                                   0x0200
#define XBEE_WAKEUP_EVT                                    0x0400
#define XBEE_VBT_CHENCK_EVT                                0x0800
#define z5                         0x1000
#define z6                         0x2000
#define z8                         0x4000
#define z7                         0x8000
  
#define XBeeOpenBuzzer   XBeeSetIO(IO_P2,High)
#define XBeeCloseBuzzer  XBeeSetIO(IO_P2,Low)
#define XBeeOpenLED1     XBeeSetIO(IO_P1,High)
#define XBeeCloseLED1    XBeeSetIO(IO_P1,Low)
#define XBeeOpenLED2     XBeeSetIO(IO_D4,High)
#define XBeeCloseLED2    XBeeSetIO(IO_D4,Low)
#define XBeeOpenLED3     XBeeSetIO(IO_D5,High)
#define XBeeCloseLED3    XBeeSetIO(IO_D5,Low)
#define XBeeOpenLED4     XBeeSetIO(IO_D3,High)
#define XBeeCloseLED4    XBeeSetIO(IO_D3,Low)
#define XBeeOpenRDDILED  XBeeSetIO(IO_P2,High)
#define XBeeCloseRSSILED XBeeSetIO(IO_P2,Low)

#define XBEE_JN       13
#define XBEE_NOT_JN   15
#define UNKNOW_NET    0
#define NO_NET        1
#define IN_NET        2
#define IN_PARK_NET   3
#define SUCCESS_TRANS 1
#define FAIL_TRANS    2
#define UART_XBEE_EN   XBeeUartEn = 0
#define UART_XBEE_DIS  XBeeUartEn = 1
/*********************************************************************
 * MACROS
 */

typedef enum
{
	ReqJion   	=	 0x01,
	AllowJion	=	 0x02,
	ReFactory	=	 0x03
}CREprotocolType;

typedef enum
{
    ROUTOR     =      1,
    ENDER      =      2,
}DeviceTypeDef;

typedef enum 
{
    sleepState,
    wakeState
}SleepONState;

typedef enum
{
    lock            =   0,
    unlock          =   1,
    lockTounlock    =   2,
    unlockTolock    =   3,
    none            =0x88
}LockCurrentStateType;

typedef enum
{
    task1    =  1,
}TaskSendType;

typedef enum
{
    InNone      =     0,
    JoinNet     =     1,
    GetSH       =     2,
    GetSL       =     3,
    GetMY       =     4,
    JoinPark    =     5,
    NetOK       =     6,
}FlagJionNetType;

typedef enum
{
    ReadHead    =    1,
    ReadLen     =    2,
    ReadData    =    3,
    ReadNone    =    4,
}ToReadUARTType;

typedef enum 
{
    ParkingUsed             =   0x00,   //车位被使用
    ParkingUnUsed           =   0x01,   //车位空
    ParkLockSuccess         =   0x02,   //锁定成功
    ParkLockFailed          =   0x03,   //锁定失败
    ParkUnlockSuccess       =   0x04,   //解锁成功
    ParkUnlockFailed        =   0x05,   //解锁失败
    ParkLockingOrUnlocking  =   0x06,   
} parkingEventType;

typedef enum{
    stateInit = 0x00,
    stateStart = 0x01,
    stateReadCfg = 0x02,
    stateWriteCfg = 0x03,
    stateRestore = 0x04,
    stateInitXBee = 0x05,
    stateInitXBeeAgain = 0x06,
    stateBeepOff = 0x07,
    stateBeepOn = 0x08,
    stateRecevie = 0x09,
    stateLedTest = 0x0A,
    stateGpioSet = 0x0B,
    stateReset = 0x10,
    stateResetOver = 0x11,
    stateReadCfgAgain = 0x12,
    stateApplyNetwork = 0x13,
    stateTest = 0x99,
    stateNoWork = 0xff
}state_t;

typedef struct
{
    uint8 IEEEadr[8];
    uint8 netadr[2];
}XBeeAdrType;

typedef struct 
{
    parkingEventType vehicleState;
    parkingEventType lockState;
}ParkingStateType;

typedef struct
{
    uint8 data[256];
    uint8 num;
}XBeeUartRecDataDef;
/***************************************************************/
extern XBeeAdrType XBeeAdr;  //IEEE地址和当前的网络地址
extern XBeeUartRecDataDef XBeeUartRec; //串口接收缓存数据  
extern __xdata FlagJionNetType FlagJionNet;
extern __xdata uint8 XBeeTaskID;
extern uint8 UartCtl; 
extern uint8 SenFlag; 
extern uint8 XBeeUartEn;
extern LockCurrentStateType LockTargetState;
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
void XBeeInit( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
uint16 XBeeProcessEvent( uint8 task_id, uint16 events );

static void npiCBack_uart( uint8 port, uint8 events );
static unsigned char referenceCmdLength(unsigned char * const command,unsigned char cmd);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif


