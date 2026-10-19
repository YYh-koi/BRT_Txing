#include "global.h"
#include "tl_buffer.h"

// TODO merge this file into ring_buffer.c ?

#if defined(CONFIG_BT_DEBUG)
#define TL_RX_BUF_SIZE	1024
#define TL_TX_BUF_SIZE	1024
#else
#define TL_RX_BUF_SIZE	1024
#define TL_TX_BUF_SIZE	8
#endif

static char s_tl_sndbuf[TL_TX_BUF_SIZE];
static char s_tl_rcvbuf[TL_RX_BUF_SIZE];

typedef struct tag_TUartContext {
	char *sndbuf;
	int sndmax;
	char *rcvbuf;
	int rcvmax;
	volatile int sndstart;
	volatile int sndend;
	volatile int rcvstart;
	volatile int rcvend;
	volatile int sndflag;
	SYSTEM_LOCK system_lock;
} TUartContext, *PTUartContext;

static TUartContext gPortContext;

void tl_buffer_init()
{
	memset(&gPortContext, 0, sizeof(gPortContext));
	gPortContext.rcvmax = TL_RX_BUF_SIZE;
	gPortContext.sndmax = TL_TX_BUF_SIZE;
	gPortContext.sndbuf = s_tl_sndbuf;
	gPortContext.rcvbuf = s_tl_rcvbuf;
	gPortContext.rcvstart = gPortContext.rcvend = 0;
	gPortContext.system_lock = CREATE_CRITICAL(NULL);
}

unsigned long tl_buffer_read(char *pt, unsigned long len)
{
	unsigned long i = 0;
	ENTER_CRITICAL(gPortContext.system_lock);

	while (i < len) {
		if (gPortContext.rcvstart == gPortContext.rcvend)
			break;

		pt[i++] = gPortContext.rcvbuf[gPortContext.rcvstart++];
		gPortContext.rcvstart %= gPortContext.rcvmax;
	}

	LEAVE_CRITICAL(gPortContext.system_lock);
	return i;
}

void tl_buffer_write(UINT8 *p_data, UINT32 len)
{
	ENTER_CRITICAL(gPortContext.system_lock);

	while (len-- > 0) {
		gPortContext.rcvbuf[gPortContext.rcvend ++] = *p_data++;
		gPortContext.rcvend %= gPortContext.rcvmax;
	}

	LEAVE_CRITICAL(gPortContext.system_lock);
}

