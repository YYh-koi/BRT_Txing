/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/


#ifndef ATBM_OS_ATOMIC_H
#define ATBM_OS_ATOMIC_H

typedef struct atomic_t
{
	  volatile int counter;
} atbm_atomic_t;

int atbm_test_bit(int nr,atbm_uint32 * addr);
int atbm_clear_bit(int nr,atbm_uint32 * addr);
int atbm_set_bit(int nr,atbm_uint32* addr);
int atbm_find_first_zero_bit(atbm_uint32* addr, int nr);

int freertos_atomic_xchg(atbm_atomic_t *v, int i);
int freertos_atomic_add_return(int i, atbm_atomic_t *v);
int freertos_atomic_sub_return(int i, atbm_atomic_t *v);
int freertos_find_first_zero_bit(atbm_uint32* addr, int nr);
atbm_void freertos_atomic_add(int i, atbm_atomic_t *v);

int freertos_atomic_read(atbm_atomic_t *v);
atbm_void freertos_atomic_set(atbm_atomic_t *v, int i);

#define atbm_atomic_read			freertos_atomic_read
#define atbm_atomic_set 			freertos_atomic_set

#define atbm_atomic_xchg  			freertos_atomic_xchg
#define atbm_atomic_add_return  	freertos_atomic_add_return

#define atbm_atomic_sub_return		freertos_atomic_sub_return
#define atbm_atomic_add				freertos_atomic_add

unsigned int freertos_local_irq_save(atbm_void);
atbm_void freertos_local_irq_restore(unsigned int cpu_sr);

#define atbm_local_irq_save freertos_local_irq_save
#define atbm_local_irq_restore freertos_local_irq_restore


#endif /* ATBM_OS_ATOMIC_H */

