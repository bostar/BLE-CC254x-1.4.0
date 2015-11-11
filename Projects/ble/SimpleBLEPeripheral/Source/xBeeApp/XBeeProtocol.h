#ifndef __XBEE_PROTOCOL_H__
#define __XBEE_PROTOCOL_H__

#include "hal_types.h"
#include "XBeeApp.h"

typedef enum
{
    InNone      =   0x00,
    JoinNet     =   0x01,
    GetSH       =   0x02,
    GetSL       =   0x03,
    GetMY       =   0x04,
    JoinPark    =   0x05,
    NetOK       =   0x06,
    SetSleep    =   0x07,
    SetSP       =   0x08,
    SetST       =   0x09,
    SetSO       =   0x10,
    SetSN       =   0x11,
    SetOK       =   0x12,
    SendND      =   0x13,
    GetAI       =   0X14,
    SetInit     =   0x15,
    SetRE       =   0x16,
    SetCB       =   0x17,
    SetSM       =   0x18,
    GetSM       =   0x19
}_type__;

uint16 CFGProcess(uint8 *rf_data);  //CFG´¦Àíº¯Êý
void CTLProcess(uint8 *rf_data);
void SENProcess(uint8 *rf_data);
void OTAProcess(uint8 *rf_data);
uint16 TSTProcess(uint8 *rf_data);

uint16 ProcessTransmitStatus(uint8 *temp_rbuf);
void ProcessAT(uint8 *temp_rbuf);
void ProcessModeStatus(uint8 *temp_rbuf);

uint16 XBeeLockState(parkingEventType LockState);
uint16 XBeeParkState(parkingEventType CarState);
uint16 SendString(uint8 in ,uint8 len );
uint8 SetXBeeSleepMode(void);
uint8 JionParkNet(void);
uint16 ReportSenser(void);
uint16 ReportLockState(void);
float ReportVbat(void);

int8 arrncmp(uint8 *arr1,uint8 *arr2,uint8 n);
void connectarr(uint8 *arr1,uint8 *arr2,uint8 *arr);
uint32 SleepModeAndJoinNet(void);
#endif

