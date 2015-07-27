#include "XBeeProtocol.h"
#include "npi.h"
#include <string.h>
#include "XBeeAtCmd.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "XBeeApp.h"
#include "XBeeBsp.h"

#if defined _XBEE_APP_

void CFGProcess(uint8 cmd)
{
  switch(cmd)
  {
    case 0x00:
      XBeeSetNJ(XBeeUartRec.data[19],NO_RES);
    break;
    case 0x02:
      if(*(&cmd+1) == 0x01)
        FlagJionNet = NetOK;
      else
        XBeeLeaveNet();
        
      break;
    case 0x03:
      break;
    default:
    break;
  }
}
void CTLProcess(uint8 cmd)
{}
void SENProcess(uint8 cmd)
{}
void OTAProcess(uint8 cmd)
{}
void TSTProcess(uint8 cmd)
{}




#endif