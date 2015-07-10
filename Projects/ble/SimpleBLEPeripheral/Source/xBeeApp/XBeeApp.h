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

