/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/


#include "atbm_sysops.h"



static void save_and_cli(void)
{
	taskENTER_CRITICAL();
}

static void restore_flags(void)
{
	taskEXIT_CRITICAL();
}

unsigned int freertos_local_irq_save(atbm_void)
{
	taskENTER_CRITICAL();
	return 0;
}

 atbm_void freertos_local_irq_restore(unsigned int cpu_sr)
{
	taskEXIT_CRITICAL();
}

int freertos_atomic_xchg(atbm_atomic_t *v, int i)
{
	int temp;

	save_and_cli();
	temp = v->counter;
	v->counter = i;
	restore_flags();

	return temp;

}

atbm_void freertos_atomic_add(int i, atbm_atomic_t *v)
{
	save_and_cli();
	v->counter += i;
	restore_flags();
}

int freertos_atomic_add_return(int i, atbm_atomic_t *v)
{
	int temp;

	save_and_cli();
	temp = v->counter;
	temp += i;
	v->counter = temp;
	restore_flags();

	return temp;

}

int freertos_atomic_sub_return(int i, atbm_atomic_t *v)
{
	int temp;
	
	save_and_cli();
	temp = v->counter;
	temp -= i;
	v->counter = temp;
	restore_flags();

	return temp;
}

int freertos_atomic_read(atbm_atomic_t *v)
{
	return v->counter;
}

atbm_void freertos_atomic_set(atbm_atomic_t *v, int i)
{
	v->counter = i;
}
#if 0
int __ffs(unsigned int word)
{
	int num = 0;
	if ((word & 0xffff) == 0) {
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0) {
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0) {
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0) {
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

int atbm_find_first_zero_bit(atbm_uint32* addr, int nr)
{
	unsigned int tmp, res, start;
	start = 0;
	
	tmp = addr[0];
	
	while (!tmp) {
		start += BITS_PER_LONG;
		if (start >= nr)
			return nr;

		tmp = addr[start / BITS_PER_LONG];
	}
	res = min(start + ffz(tmp), nr);


	return res;
}
#endif

 int atbm_set_bit(int nr,atbm_uint32* addr)
 {
	 int mask,retval;
	 
	 addr += nr >>5;
	 mask = 1<<(nr & 0x1f);
	 
	 save_and_cli();
	 retval = (mask & *addr) != 0;
	 *addr |= mask;
	 restore_flags();
	 
	 return  retval;
	 
 }
 
 int atbm_clear_bit(int nr,atbm_uint32 * addr)
 {
	 int mask,retval;
	 
	 addr += nr >>5;
	 mask = 1<<(nr & 0x1f);
	 
	 save_and_cli();
	 retval = (mask & *addr) != 0;
	 *addr &= ~mask;
	 restore_flags();
	 
	 return  retval;
	 
 }
 
 
 int atbm_test_bit(int nr,atbm_uint32 * addr)
 {
 
	 int mask;
	 
	 addr += nr >>5;
	 mask = 1<<(nr & 0x1f);
	 
	 return  ((mask & *addr) != 0);
	 
 }

 int atbm_find_first_zero_bit(atbm_uint32 * addr,int size)
 {
	 int i =0;
	 for (i = 0; i <size; i++) {
		 if(atbm_test_bit(i,addr) ==0)
			 return i;
	 }
	 return -1;
 }
