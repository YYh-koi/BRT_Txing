#ifndef ATBM_OS_MUTEX_H
#define ATBM_OS_MUTEX_H
#include "FreeRTOS.h"
#include "atbm_sysops.h"
#include "semphr.h"

typedef  SemaphoreHandle_t atbm_mutex  ;
typedef  SemaphoreHandle_t  atbm_os_wait_queue_head_t  ;

/*mutex*/

int freertos_mutex_init(atbm_mutex *x);
int freertos_mutex_lock(atbm_mutex *x, atbm_uint32 ulTimeout);
int freertos_mutex_unlock(atbm_mutex *x);
int freertos_DeleteMutex(atbm_mutex *x);
int freertos_wait_event_timeout(atbm_mutex *x,atbm_uint32 ticks);
atbm_void freertos_os_init_waitevent(atbm_os_wait_queue_head_t* pulSemId);
atbm_void freertos_os_delete_waitevent(atbm_os_wait_queue_head_t* pulSemId);
void freertos_os_wakeup_event(atbm_os_wait_queue_head_t *pSem);
int atbm_os_wait_event_count_get(atbm_os_wait_queue_head_t *pSem);


#define atbm_os_mutexLockInit freertos_mutex_init
#define atbm_os_mutexLock freertos_mutex_lock
#define atbm_os_mutexUnLock freertos_mutex_unlock
#define atbm_os_DeleteMutex freertos_DeleteMutex

#define atbm_os_wait_event_timeout freertos_wait_event_timeout//freertos_mutex_lock((atbm_mutex *) x,  t)
#define atbm_os_wakeup_event(x) 	freertos_os_wakeup_event(x)
//#define atbm_os_wakeup_event_isr(x) freertos_mutex_unlock((atbm_mutex *)x)
#define atbm_os_mutextryLock(x)		freertos_mutex_lock((atbm_mutex *)x, 10)
#define atbm_os_init_waitevent		freertos_os_init_waitevent
#define atbm_os_delete_waitevent	freertos_os_delete_waitevent



int freertos_wait_event_timeout_ext(atbm_mutex *x,atbm_uint32 ms);



#endif /* ATBM_OS_MUTEX_H */

