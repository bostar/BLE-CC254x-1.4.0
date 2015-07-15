#ifndef __ZIGBEEAPP_H
#define __ZIGBEEAPP_H

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
#define ZIGBEE_START_DEVICE_EVT                              0x0001
#define ZIGBEE_RESET_ZM516X_EVT                              0x0002
#define ZIGBEE_READ_ZM516X_INFO_EVT                          0x0004
#define UART_RECEIVE_EVT                                     0x0008
#define BOARD_TEST_EVT                                       0x0010
#define READ_UART_BUFFER_EVT                                 0x0020
#define ZIGBEE_APPLY_NETWORK_EVT                             0x0040
#define ZIGBEE_WRITE_ZM516X_INFO_EVT                         0x0080
#define ZIGBEE_LINK_TEST_EVT                                 0x0100
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
    stateGpioSet = 0x0B,
    stateReset = 0x10,
    stateResetOver = 0x11,
    stateReadCfgAgain = 0x12,
    stateApplyNetwork = 0x13,
    stateMotorForward = 0x14,
    stateMotorReverse = 0x15,
    stateAckLinkTest = 0x16,
    stateTest = 0x99,
    stateNoWork = 0xff
}state_t;

/* States for CRC parser */
typedef enum {
    NPI_SERIAL_PACK_HEAD,
    NPI_SERIAL_PACK_CMD,
    NPI_SERIAL_PACK_LEN,
    NPI_SERIAL_PACK_DATA,
} npi_serial_parse_state_t;

typedef enum {
    ZLG_FLASH_SET,
    ZLG_RAM_SET,
    BASE_STATION_CFG,
    BASE_STATION_CTL,
    BASE_STATION_SEN,
    BASE_STATION_OTA,
    BASE_STATION_TST
}command_word_t;


typedef struct {
    unsigned char header[3];
    unsigned char cmd;
    unsigned char len;
}ptk_t;
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

#endif

