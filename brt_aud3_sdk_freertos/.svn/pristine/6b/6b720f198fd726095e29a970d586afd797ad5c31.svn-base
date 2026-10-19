#ifndef BTAPP_RING_BUFFER_H__
#define BTAPP_RING_BUFFER_H__

#include "btdatatype.h"
#include "sysdep.h"

typedef struct tag_ring_buffer {
	char *rcvbuf;
	int rcvmax;
	volatile int rcvstart;
	volatile int rcvend;
	SYSTEM_LOCK system_lock;
} ring_buffer_t;

ring_buffer_t *ring_buffer_alloc(int buf_len);
void ring_buffer_free_ex(ring_buffer_t *buf);
void ring_buffer_push(ring_buffer_t *buf, UINT8 *p_data, UINT32 len);
unsigned long ring_buffer_pull(ring_buffer_t *buf, char *pt, unsigned long len);

#endif // BTAPP_RING_BUFFER_H__
