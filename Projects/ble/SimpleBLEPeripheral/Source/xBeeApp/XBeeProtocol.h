#ifndef __XBEE_PROTOCOL_H__
#define __XBEE_PROTOCOL_H__

#include "hal_types.h"
#include "XBeeApp.h"

typedef enum
{
  Success     =    0x01,
  UnlockFail    =    0x02,
  LockFail   =    0x03,
}LockStateType;






void CFGProcess(uint8 *cmd);  //CFG´¦Àíº¯Êý
void CTLProcess(uint8 *cmd);
void SENProcess(uint8 *cmd);
void OTAProcess(uint8 *cmd);
void TSTProcess(uint8 *cmd);
uint16 XBeeLockState(LockStateType LockState);
uint16 XBeeParkState(parkingEventType CarState);
uint16 SendString(uint8 in ,uint8 len );
#endif

