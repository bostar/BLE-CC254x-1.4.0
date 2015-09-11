#ifndef __XBEE_PROTOCOL_H__
#define __XBEE_PROTOCOL_H__

#include "hal_types.h"
#include "XBeeApp.h"








uint16 CFGProcess(uint8 *rf_data);  //CFG������
void CTLProcess(uint8 *rf_data);
void SENProcess(uint8 *rf_data);
void OTAProcess(uint8 *rf_data);
void TSTProcess(uint8 *rf_data);
uint16 XBeeLockState(parkingEventType LockState);
uint16 XBeeParkState(parkingEventType CarState);
uint16 SendString(uint8 in ,uint8 len );
void ProcessAT(XBeeUartRecDataDef temp_rbuf);
void ProcessModeStatus(XBeeUartRecDataDef temp_rbuf);
uint8 SetXBeeSleepMode(void);
void JionParkNet(void);
uint16 ReportSenser(void);
uint16 ReportStatePeriod(void);

#endif

