#include "c_queue.h"
#include "XBeeAtCmd.h"


//serial receive data buffer define******************************************************************
void creat_circular_queue( CircularQueueType *queue )  
{
    queue -> front = 0;
    queue -> rear = 0; 
    queue -> count = 0;
    queue -> maxsize = _REV_DATA_MAX - 1;
}
int8 is_empty( CircularQueueType *queue)    
{
    if(queue->count == 0)  
        return 0;
    else  
        return -1;
}  
int8 is_full( CircularQueueType *queue )  
{  
    if( queue->count == _REV_DATA_MAX )
        return 0;
    else
        return -1;  
} 
int8 in_queue( CircularQueueType *queue, uint8 value)  
{  
    if ( queue -> count == _REV_DATA_MAX )
    {  
		queue -> elements[queue ->rear] = value;  
		queue -> rear = (queue -> rear + 1) % _REV_DATA_MAX;
		queue -> front = queue -> rear;
		return -1;
    }  
    else  
    {  
		queue -> elements[queue ->rear] = value;  
		queue -> rear = (queue -> rear + 1) % _REV_DATA_MAX;       	
       	queue -> count++;
		return 0;         
	}	    
}  
int8 out_queue(CircularQueueType *queue , uint8 *out_buf)  
{
    if ( queue -> count == 0 )
		return -1;
    else  
    {  
		*out_buf = queue -> elements[queue -> front];
        queue -> front = (queue -> front + 1) % _REV_DATA_MAX;  
        queue -> count -= 1;
        return 0;
    }  
}
/**********************************************************************
**brief	write n byte into queue 
**param	p_cqueue	pointer of the circular queue
		buf			the write data buffer
		n			the number of byte to be writen
**reval	the number of byte writen into queue
**********************************************************************/
uint16 write_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n)
{
	uint16 i=0,reval=0;

	for(i=0;i<n;i++)
	{
		in_queue( p_cqueue, *(buf + i));
		reval++;
	}
	return reval;
}
/**********************************************************************
**brief	read n byte from queue 
**param	p_cqueue	pointer of the circular queue
		buf			the read data buffer
		n			the number of byte to be read
**reval	the number of byte read from queue
**********************************************************************/
uint16 read_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n)
{
	uint8 i=0,reval=0;
	int8 state;

	for(i=0;i<n;i++)
	{
		state = out_queue( p_cqueue , buf+i);
		if(state == 0)
		{
			reval++;
		}
		else
			break;
	}
	return reval;
}
/*****************************************************************************
**brief	analysis package from a queue
**param	
**reval	
*****************************************************************************/
uint16 read_one_package_f_queue( CircularQueueType* p_cqueue , uint8* UartRevBuf )
{
	int16 len;
	uint16 DataLen=0;
	uint8 checksum,reval=0;
	
	len = read_cqueue(p_cqueue , UartRevBuf , 1);
    if(len == 1 && UartRevBuf[0] == 0x7E)
    {
        len = read_cqueue(p_cqueue , UartRevBuf+1 , 2);
        if(len == 2)
        {
            DataLen = 0;
            DataLen |= UartRevBuf[2];
            DataLen |= (uint16)UartRevBuf[1]<<8;
            len = read_cqueue(p_cqueue , UartRevBuf+3 , DataLen+1);
            if(len == DataLen+1)
            {
                checksum = XBeeApiChecksum(UartRevBuf+3,DataLen); //校验数据
                if(checksum == UartRevBuf[DataLen+3])
                    reval = DataLen+4;
            }
        }
    }
    return reval;
}
/*****************************************************************************
**brief	clear queue
**param	
**reval	
*****************************************************************************/
void clear_queue(CircularQueueType* p_cqueue)
{
    p_cqueue->count = 0;
    p_cqueue->front = 0;
    p_cqueue->rear  = 0;
}




















