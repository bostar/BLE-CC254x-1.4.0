#ifndef _ZIGBEE_H__
#define _ZIGBEE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */


// Simple BLE Peripheral Task Events
#define ZIGBEE_START_DEVICE_EVT                              0x0001
#define ZIGBEE_PERIODIC_EVT                                  0x0002
#define ZIGBEE_READ_ZM516X_INFO_EVT                          0x0004
#define UART_RECEIVE_EVT                                     0x0008
#define BOARD_TEST_EVT                                       0x0010
#define READ_UART_BUFFER_EVT                                 0x0020
/*********************************************************************
 * MACROS
 */
typedef enum{
    stateInit = 0x00,
    stateStart = 0x01,
    stateReadCfg = 0x02,
    stateWriteCfg = 0x03,
    stateRestore = 0x04,
    stateInitZlg = 0x05,
    stateInitZlgAgain = 0x06,
    stateBeepOff = 0x07,
    stateBeepOn = 0x08,
    stateRecevie = 0x09,
    stateLedTest = 0x0A,
    stateReset = 0x10,
    stateResetOver = 0x11,
    stateReadCfgAgain = 0x12,
    stateApplyNetwork = 0x13,
    stateTest = 0x99,
    stateNoWork = 0xff
}state_t;
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void Zigbee_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 Zigbee_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEBLEPERIPHERAL_H */