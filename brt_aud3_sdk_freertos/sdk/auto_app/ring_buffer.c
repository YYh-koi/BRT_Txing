#include "btdatatype.h"
#include "ring_buffer.h"
#include "sysdep.h"
#include <string.h>

ring_buffer_t *ring_buffer_alloc(int buf_len)
{
	ring_buffer_t *buf = NEW(sizeof(ring_buffer_t));

	memset(buf, 0, sizeof(ring_buffer_t));
	buf->rcvmax = buf_len;
	buf->rcvbuf = NEW(buf_len);
	buf->rcvstart = buf->rcvend = 0;
	buf->system_lock = CREATE_CRITICAL(NULL);

	return buf;
}

void ring_buffer_free_ex(ring_buffer_t *buf)
{
	if (buf == NULL) {
		return;
	}

	if (buf->rcvbuf) {
		FREE_CRITICAL(buf->system_lock);
		FREE(buf->rcvbuf);
	}

	FREE(buf);
}

void ring_buffer_push(ring_buffer_t *buf, UINT8 *p_data, UINT32 len)
{
	if (buf == NULL || p_data == NULL || len == 0) {
		return;
	}

	ENTER_CRITICAL(buf->system_lock);

	while(len-- > 0) {
		buf->rcvbuf[buf->rcvend ++] = *p_data++;
		buf->rcvend %= buf->rcvmax;
	}

	LEAVE_CRITICAL(buf->system_lock);
}

unsigned long ring_buffer_pull(ring_buffer_t *buf, char *pt, unsigned long len)
{
	unsigned long i = 0;

	if (buf == NULL || pt == NULL || len == 0) {
		return 0;
	}

	ENTER_CRITICAL(buf->system_lock);
	while(i < len) {
		if(buf->rcvstart == buf->rcvend)
			break;
		pt[i++] = buf->rcvbuf[buf->rcvstart++];
		buf->rcvstart %= buf->rcvmax;
	}
	LEAVE_CRITICAL(buf->system_lock);
	return i;
}

