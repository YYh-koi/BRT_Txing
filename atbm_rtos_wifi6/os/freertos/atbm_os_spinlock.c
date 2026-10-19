#include "atbm_sysops.h"


#ifndef USE_MUTEX_FOR_SPINLOCK
#define USE_MUTEX_FOR_SPINLOCK 0
#endif

void freertos_spin_lock_init(atbm_spinlock_t *x)
{
#if USE_MUTEX_FOR_SPINLOCK
	//*x = xSemaphoreCreateMutex();
	*x =(atbm_mutex*)malloc(sizeof(atbm_mutex));
	vSemaphoreCreateBinary(*x);
#endif
}
void freertos_spin_lock(atbm_spinlock_t *x)
{
#if USE_MUTEX_FOR_SPINLOCK
	while(xSemaphoreTake(*x, 60 * 1000) != pdTRUE)
		printf("[%s] %s(%p) failed, retry\n", pcTaskGetName(NULL), __FUNCTION__, x);
#else
	taskENTER_CRITICAL();
	//vTaskSuspendAll();
#endif
}

void freertos_spin_unlock(atbm_spinlock_t *x)
{
#if USE_MUTEX_FOR_SPINLOCK
		xSemaphoreGive(*x);
#else
	taskEXIT_CRITICAL();
	//xTaskResumeAll();
#endif
	

}

void freertos_spin_lock_irqsave(atbm_spinlock_t *x, unsigned long * flags)
{
#if USE_MUTEX_FOR_SPINLOCK
	while(xSemaphoreTake(*x, 60 * 1000 ) != pdTRUE)
		printf("[%s] %s(%p) failed, retry\n", pcTaskGetName(NULL), __FUNCTION__, x);
#else
	taskENTER_CRITICAL();
	//vTaskSuspendAll();
#endif
}

void freertos_spin_unlock_irqrestore(atbm_spinlock_t *x, unsigned long flags)
{
#if USE_MUTEX_FOR_SPINLOCK
		xSemaphoreGive(*x);
#else
		taskEXIT_CRITICAL();
		
	//xTaskResumeAll();
#endif

}


