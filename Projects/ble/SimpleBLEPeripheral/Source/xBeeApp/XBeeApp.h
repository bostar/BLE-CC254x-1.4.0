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


// Simple BLE Peripheral Task Events      SYS_EVENT_MSG 0x8000 
#define XBEE_START_DEVICE_EVT                              0x0001
#define XBEE_JOIN_NET_EVT                                  0x0002
#define XBEE_REPORT_EVT                                    0x0004    
#define XBEE_READ_SENSER_EVT                               0x0008
#define XBEE_REC_DATA_PROCESS_EVT                          0x0010
#define XBEE_TEST_EVT                                      0x0020
#define XBEE_CLOSE_BUZZER_EVT                              0x0040
#define z4                                      0x0080
#define XBEE_MOTOR_CTL_EVT                                 0x0100
#define XBEE_HMC5983_EVT                                   0x0200
#define z5                               0x0400
#define XBEE_VBT_CHENCK_EVT                                0x0800
#define z8                                    0x1000
#define z6                         0x2000
#define z9                         0x4000
  
#define XBeeOpenBuzzer()   XBeeSetIO(IO_P2,High)
#define XBeeCloseBuzzer()  XBeeSetIO(IO_P2,Low)
#define XBeeOpenLED1()     XBeeSetIO(IO_P1,High)
#define XBeeCloseLED1()    XBeeSetIO(IO_P1,Low)
#define XBeeOpenLED2()     XBeeSetIO(IO_D4,High)
#define XBeeCloseLED2()    XBeeSetIO(IO_D4,Low)
#define XBeeOpenLED3()     XBeeSetIO(IO_D5,High)
#define XBeeCloseLED3()    XBeeSetIO(IO_D5,Low)

#define SEN_THR         50
#define SEN_MOTOR       0.7
/*******************XBeeProcessEvent msg operation*******************/
#define MOTOR_LOCK      (uint8)1
#define MOTOR_UNLOCK    (uint8)2
#define MOTOR_REVERSE   (uint8)3
#define ACTIVATE        (uint8)4
#define INACTIVATE      (uint8)4
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
    stateInit               =   0x00,
    stateStart              =   0x01,
    stateReadCfg            =   0x02,
    stateWriteCfg           =   0x03,
    stateRestore            =   0x04,
    stateInitXBee           =   0x05,
    stateInitXBeeAgain      =   0x06,
    stateBeepOff            =   0x07,
    stateBeepOn             =   0x08,
    stateRecevie            =   0x09,
    stateLedTest            =   0x0A,
    stateGpioSet            =   0x0B,
    stateReset              =   0x10,
    stateResetOver          =   0x11,
    stateReadCfgAgain       =   0x12,
    stateApplyNetwork       =   0x13,
    stateTest               =   0x99,
    stateNoWork             =   0xff
}state_t;

typedef enum
{
    router  =   0x01,
    end_dev =   0x02,
    notype  =   0x03
}DeviceType;

typedef struct
{
    uint8 MacAdr[8];
    uint8 NetAdr[2];
    uint8 DevType;
    uint8 NetState;
    uint8 channel;
    uint8 XBeeAI;
    uint8 InPark;
    uint8 panID[8];
    uint8 FlagReadFlash;
    uint8 ParentLost;
    uint8 GetSM;
    uint8 Test;
}XBeeInfoType;

typedef struct 
{
    parkingEventType vehicleState;
    parkingEventType lockState;
}ParkingStateType;

typedef struct
{
    LockCurrentStateType  LockState;
    HMC5983DataType hmc5983Data;
    uint8 panID[8];
}FlashLockStateType;

typedef struct
{
    LockCurrentStateType FinalState;
    LockCurrentStateType CurState;
    LockCurrentStateType MidState;
}LcokStateType;

typedef struct
{
    uint16 event;
    uint8 operation;
}XBeeMsgType;


/***************************************************************/
extern XBeeInfoType XBeeInfo;  
extern uint8 XBeeTaskID;
extern uint8 UartCtl; 
extern uint8 SenFlag; 
extern LockCurrentStateType LockObjState;
extern LcokStateType LockState;
extern uint8 SetSleepMode;
extern FlashLockStateType FlashLockState;
extern ParkingStateType parkingState;              //当前车位状态
extern uint8 test_cnt;
extern uint8 CoorMAC[8];
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
void XBeeInit( uint8 task_id );

void ProcessSerial(uint8 *temp_rbuf);
void ProcessXBeeMsg(XBeeMsgType *pMsg);

uint8 CreatXBeeMsg(uint16 event,uint8 state);
float ReadMotorSen(void);
uint8 CheckMotor(void);


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


