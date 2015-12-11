#include "zlg_protocol.h"
#include "npi.h"
#include <string.h>
#include "zlgAtCmd.h"
#include "OSAL.h"

#define  WBUF_LEN   20
static char wbuf[ WBUF_LEN ];

void applicateForNetwork( unsigned char *IEEEAddress )
{
    osal_memset( wbuf, 0x00, WBUF_LEN );
    
    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdCheckIn;//cmd

    osal_memcpy( &wbuf[4], (char const *)IEEEAddress, 8 );
    NPI_WriteTransport( (unsigned char *)wbuf , 12 ); 
}

void ackLinkTest( unsigned char *IEEEAddress )
{
    osal_memset( wbuf, 0x00, WBUF_LEN ); 
  
    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdAckLinkTest;//cmd    
    osal_memcpy( &wbuf[4], (char const *)IEEEAddress, 8 );
    
    NPI_WriteTransport( (unsigned char *)wbuf , 11 );
}

void dateRequset( unsigned char status )
{
    osal_memset( wbuf, 0x00, WBUF_LEN );
    
    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdDataRequest;//cmd  
    wbuf[4] = stDevInfo->devLoacalNetAddr[0];
    wbuf[5] = stDevInfo->devLoacalNetAddr[1];
    wbuf[6] = parkingState->lockState;
    wbuf[7] = parkingState->vehicleState;
    wbuf[8] = (unsigned char)( ( mag_xyz->x & 0xff00 ) >> 8);
    wbuf[9] = (unsigned char)( mag_xyz->x & 0x00ff );
    wbuf[10] = (unsigned char)( ( mag_xyz->y & 0xff00 ) >> 8);
    wbuf[11] = (unsigned char)( mag_xyz->y & 0x00ff );
    wbuf[12] = (unsigned char)( ( mag_xyz->z & 0xff00 ) >> 8);
    wbuf[13] = (unsigned char)( mag_xyz->z & 0x00ff );
    wbuf[14] = status;
    
    NPI_WriteTransport( (unsigned char *)wbuf , 15 );
}

void batteryRemainingReport( unsigned char voltage )
{    
    osal_memset( wbuf, 0x00, WBUF_LEN );
    
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = cmdBatteryRemainReport;
    wbuf[4] = voltage;
    wbuf[5] = stDevInfo->devLoacalNetAddr[0];
    wbuf[6] = stDevInfo->devLoacalNetAddr[1];
    
    NPI_WriteTransport( (unsigned char *)wbuf , 7 );
}

void motorStopAdcReport( unsigned short voltage )
{     
    osal_memset( wbuf, 0x00, WBUF_LEN );
    
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = 0x03;
    wbuf[4] = voltage >> 8;
    wbuf[5] = voltage;
    wbuf[6] = stDevInfo->devLoacalNetAddr[0];
    wbuf[7] = stDevInfo->devLoacalNetAddr[1];
    
    NPI_WriteTransport( (unsigned char *)wbuf , 8 );
}