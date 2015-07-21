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
#define XBEE_IO_TEST                                       0x0002
#define XBEE_MCU_UART_RECEIVE_EVT                          0x0004 //串口标志位占用
#define XBEE_CLOSE_LED                                     0x0008
#define XBEE_REC_DATA_PROCESS_EVT                          0x0010
#define XBEE_READ_UART_BUFFER_EVT                          0x0020
#define XBEE_APPLY_NETWORK_EVT                             0x0040
#define XBEE_WRITE_ZM516X_INFO_EVT                         0x0080
#define z1                         0x0100
#define z2                         0x0200
#define z3                         0x0400
#define z4                         0x0800
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

/*********************************************************************
 * MACROS
 */



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

/* States for CRC parser */
typedef enum {
  NPI_SERIAL_PACK_HEAD,
    NPI_SERIAL_PACK_CMD,
    NPI_SERIAL_PACK_LEN,
    NPI_SERIAL_PACK_DATA,
} npi_serial_parse_state_t;

typedef enum {
    XBee_FLASH_SET,
    XBee_RAM_SET,
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

typedef struct
{
  uint8 data[255];
  uint8 num;
}XBeeUartRecDataDef;
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


