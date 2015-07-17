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
    //set_temporary_cast_mode(unicast); 
}

//void ackChangeNodeType(void)
//{
//    char wbuf[4];
//    wbuf[0] = 'C';
//    wbuf[1] = 'F';
//    wbuf[2] = 'G';
//    wbuf[3] = cmdAckChangeNodeType;//cmd
//    
//    NPI_WriteTransport( (unsigned char *)wbuf , 4 );
//}

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

void switchLockControl(LockSW_t cmd)
{
    char wbuf[5];
    wbuf[0] = 'C';
    wbuf[1] = 'T';
    wbuf[2] = 'L';
    wbuf[3] = 0x00;//cmd
    wbuf[4] = cmd;
    
    NPI_WriteTransport( (unsigned char *)wbuf , 5 );
}