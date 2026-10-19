/* FreeRTOS includes. */
#ifndef OS_API_H
#define OS_API_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "semaphore.h"
#include "event_groups.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdio.h>
#include "sys.h"
#include <timers.h>
#include <task.h>
//#include <kwrap/flag.h>


int printf(const char *format, ...);

#define iot_printf printf
#ifndef ZEROSIZE
#define ZEROSIZE 1
#endif

#define BITS_PER_LONG 32
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define max(a, b) ((a)>(b) ? (a) : (b))
#define min(a, b) ((a)<(b) ? (a) : (b))
#define ffz(x)  __ffs(~(x))

#define  rcu_read_lock()
#define  rcu_read_unlock()

#endif

