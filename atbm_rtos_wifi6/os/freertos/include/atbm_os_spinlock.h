#ifndef ATBM_OS_SPINLOCK_H
#define ATBM_OS_SPINLOCK_H
//#include <drvOSAPI.h>

#include "atbm_sysops.h"


typedef  atbm_void * atbm_spinlock_t;

void freertos_spin_lock_init(atbm_spinlock_t *x);
void freertos_spin_lock(atbm_spinlock_t *x);
void freertos_spin_unlock(atbm_spinlock_t *x);
void freertos_spin_lock_irqsave(atbm_spinlock_t *x, unsigned long * flags);
void freertos_spin_unlock_irqrestore(atbm_spinlock_t *x, unsigned long  flags);


/*spin lock*/
#define atbm_spin_lock_init  freertos_spin_lock_init 
#define atbm_spin_lock  freertos_spin_lock 
#define atbm_spin_unlock  freertos_spin_unlock 
#define atbm_spin_lock_irqsave  freertos_spin_lock_irqsave
#define atbm_spin_unlock_irqrestore  freertos_spin_unlock_irqrestore

#define atbm_spin_lock_bh freertos_spin_lock
#define atbm_spin_unlock_bh freertos_spin_unlock

#define atbm_disable_irq()                  portDISABLE_INTERRUPTS()
#define atbm_enable_irq(f)                  portENABLE_INTERRUPTS()

#endif /* ATBM_OS_SPINLOCK_H */

