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
    char wbuf[4];
    wbuf[0] = 'C';
    wbuf[1] = 'F';
    wbuf[2] = 'G';
    wbuf[3] = cmdDateRequest;//cmd  
    
    NPI_WriteTransport( (unsigned char *)wbuf , 4 );
}

void ackSwitchLockControl(lockRS_t cmd)
{
    char wbuf[5];
    wbuf[0] = 'C';
    wbuf[1] = 'T';
    wbuf[2] = 'L';
    wbuf[3] = 0x01;//cmd
    wbuf[4] = cmd;
    
    NPI_WriteTransport( (unsigned char *)wbuf , 5 );
}

void eventReport(carState_t carState)
{
    char wbuf[5];
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = cmdEventReport;
    wbuf[4] = carState;
    
    NPI_WriteTransport( (unsigned char *)wbuf , 5 );
}

void batteryRemainingReport(unsigned char voltage)
{
    char wbuf[5];
    
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = cmdBatteryRemainReport;
    wbuf[4] = voltage;
    
    NPI_WriteTransport( (unsigned char *)wbuf , 5 );
}