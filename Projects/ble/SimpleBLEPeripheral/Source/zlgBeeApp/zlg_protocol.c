#include "zlg_protocol.h"
#include "npi.h"
#include <string.h>
#include "zlgAtCmd.h"
#include "OSAL.h"

void applicateForNetwork(unsigned char *IEEEAddress)
{
    char wbuf[12];

    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdCheckIn;//cmd

    osal_memcpy( &wbuf[4],(char const *)IEEEAddress,8 );
    NPI_WriteTransport( (unsigned char *)wbuf , 12 ); 
}

void ackLinkTest(unsigned char *IEEEAddress)
{
    char wbuf[11];
    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdAckLinkTest;//cmd    
    osal_memcpy( &wbuf[4],(char const *)IEEEAddress,8 );
    
    NPI_WriteTransport( (unsigned char *)wbuf , 11 );
}

void dateRequset(void)
{
    char wbuf[6];
    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdDataRequest;//cmd  
    wbuf[4] = stDevInfo->devLoacalNetAddr[0];
    wbuf[5] = stDevInfo->devLoacalNetAddr[1];
    
    NPI_WriteTransport( (unsigned char *)wbuf , 6 );
}

void eventReport(parkingEvent_t event)
{
    char wbuf[7];
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = cmdEventReport;
    wbuf[4] = event;
    wbuf[5] = stDevInfo->devLoacalNetAddr[0];
    wbuf[6] = stDevInfo->devLoacalNetAddr[1];
    
    NPI_WriteTransport( (unsigned char *)wbuf , 7 );
}

void batteryRemainingReport(unsigned char voltage)
{
    char wbuf[7];
    
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = cmdBatteryRemainReport;
    wbuf[4] = voltage;
    wbuf[5] = stDevInfo->devLoacalNetAddr[0];
    wbuf[6] = stDevInfo->devLoacalNetAddr[1];
    
    NPI_WriteTransport( (unsigned char *)wbuf , 7 );
}