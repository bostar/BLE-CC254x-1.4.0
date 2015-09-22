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
#include "hal_uart.h"

/*********************************************************************
 * CONSTANTS
 */


// Simple BLE Peripheral Task Events
#define XBEE_START_DEVICE_EVT                              0x0001
#define XBEE_JOIN_NET_EVT                                  0x0002
#define XBEE_KEEP_LOCK_STATE_EVT                           0x0004    
#define XBEE_SAVE_FLASH_EVT                                0x0008
#define XBEE_REC_DATA_PROCESS_EVT                          0x0010
#define XBEE_TEST_EVT                                      0x0020
#define XBEE_TEST2_EVT                                     0x0040
#define XBEE_IDLE_EVT                                      0x0080
#define XBEE_MOTOR_CTL_EVT                                 0x0100
#define XBEE_HMC5983_EVT                                   0x0200
#define XBEE_SCAN_ROUTE_PATH                               0x0400
#define XBEE_VBT_CHENCK_EVT                                0x0800
#define z5                         0x1000
#define z6                         0x2000
#define z8                         0x4000
#define z7                         0x8000
  
#define XBeeOpenBuzzer()   XBeeSetIO(IO_P2,High)
#define XBeeCloseBuzzer()  XBeeSetIO(IO_P2,Low)
#define XBeeOpenLED1()     XBeeSetIO(IO_P1,High)
#define XBeeCloseLED1()    XBeeSetIO(IO_P1,Low)
#define XBeeOpenLED2()     XBeeSetIO(IO_D4,High)
#define XBeeCloseLED2()    XBeeSetIO(IO_D4,Low)
#define XBeeOpenLED3()     XBeeSetIO(IO_D5,High)
#define XBeeCloseLED3()    XBeeSetIO(IO_D5,Low)

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
#define OFFSET        100
#define SEN_MOTOR       0.7
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
    sleepState,
    wakeState
}SleepONState;

typedef enum
{
    unlock          =   0,
    lock            =   1,
    lockTounlock    =   2,
    unlockTolock    =   3,
    over_lock       =   4,
    over_unlock     =   5,
    none            =0x88
}LockCurrentStateType;

typedef enum
{
    task1    =  1,
}TaskSendType;

typedef enum
{
    ReadHead    =   1,
    ReadLen     =   2,
    ReadData    =   3,
    ReadNone    =   4,  
    None        =   5
}ToReadUARTType;

typedef enum 
{
    ParkingUsed             =   0x01,   //车位被使用
    ParkingUnUsed           =   0x02,   //车位空
    ParkLockSuccess         =   0x03,   //锁定成功
    ParkLockFailed          =   0x04,   //锁定失败
    ParkUnlockSuccess       =   0x05,   //解锁成功
    ParkUnlockFailed        =   0x06,   //解锁失败
    ParkLockingOrUnlocking  =   0x07
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

typedef enum
{
    router  =   0x01,
    end_dev =   0x02,
    notype  =   0x03
}DeviceType;

typedef struct
{
    uint8 IEEEadr[8];
    uint8 netadr[2];
    uint8 DevType;
    uint8 NetState;
}XBeeInfoType;

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
   
typedef struct
{
    LockCurrentStateType  LockState;
    HMC5983DataType hmc5983Data;
    uint8 panID[8];
}FlashLockStateType;

/***************************************************************/
extern XBeeInfoType XBeeInfo;  //IEEE地址和当前的网络地址
extern XBeeUartRecDataDef XBeeUartRec; //串口接收缓存数据  
extern uint8 FlagJionNet;
extern __xdata uint8 XBeeTaskID;
extern uint8 UartCtl; 
extern uint8 SenFlag; 
extern uint8 XBeeUartEn;
extern LockCurrentStateType LockObjState;
extern uint8 SetSleepMode;
extern FlashLockStateType FlashLockState;
extern ParkingStateType parkingState;              //当前车位状态
extern uint8 ReadFlashFlag;
extern DeviceType DevType;
extern uint8 test_cnt;
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
void XBeeInit( uint8 task_id );
void ProcessSerial(XBeeUartRecDataDef temp_rbuf);
void DailyEvt(void);
float ReadMotorSen(void);
uint32 SleepAndJoinNet(void);
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


