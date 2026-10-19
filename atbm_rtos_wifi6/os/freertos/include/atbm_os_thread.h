#ifndef ATBM_OS_THREAD_H
#define ATBM_OS_THREAD_H

#include "atbm_sysops.h"

typedef atbm_void * pAtbm_thread_t ;
/*priotry */
#if 0
enum  ATBM_THREAD_PRIO{
	WORK_TASK_PRIO=25,
	BH_TASK_PRIO,
	ELOOP_TASK_PRIO,
	HIF_TASK_PRIO,
	TX_BH_TASK_PRIO,
};
#else
#define  WORK_TASK_PRIO 25
#define  BH_TASK_PRIO 28
#define  ELOOP_TASK_PRIO 26
#define  HIF_TASK_PRIO 27
#define  TX_BH_TASK_PRIO 29
#define TEST_TASK_PRIO 20


#endif
#define RX_BH_TASK_PRIO BH_TASK_PRIO

typedef atbm_void (*task)(atbm_void *p_arg);

pAtbm_thread_t freertos_createThread(char *name,atbm_void (*task)(atbm_void *p_arg),atbm_void *p_arg,int prio);
atbm_void freertos_stopThread(pAtbm_thread_t thread_id);
atbm_void freertos_ThreadStopEvent(pAtbm_thread_t thread_id);
atbm_uint32 atbm_getCurThreadId(void);

#define atbm_createThread freertos_createThread
#define atbm_stopThread freertos_stopThread
#define atbm_ThreadStopEvent freertos_ThreadStopEvent

#define THREAD_RETURN(a)  vTaskDelete(NULL)

#endif /* ATBM_OS_THREAD_H */
