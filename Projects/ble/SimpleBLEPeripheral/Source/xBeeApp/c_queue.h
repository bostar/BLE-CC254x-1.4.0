#ifndef _C_QUEUE_H_
#define _C_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_types.h"

#define _REV_DATA_MAX   512

typedef struct
{
	uint32 front;
	uint32 rear;
	uint32 count;
	uint32 maxsize;
	uint8 elements[_REV_DATA_MAX];
}CircularQueueType;



void creat_circular_queue( CircularQueueType *queue );
int8 is_empty( CircularQueueType *queue);
int8 is_full( CircularQueueType *queue );
int8 in_queue( CircularQueueType *queue, uint8 value);
int8 out_queue( CircularQueueType *queue , uint8 *out_buf);
uint16 read_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n);
uint16 write_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n);
uint16 read_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n);
uint16 read_one_package_f_queue( CircularQueueType* p_cqueue , uint8* buf );





#endif



















