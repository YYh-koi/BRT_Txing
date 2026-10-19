/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/

#ifndef ATBM_OS_MEM_H
#define ATBM_OS_MEM_H
#include "atbm_sysops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "malloc.h"
#include "mem.h"

extern void *mem_malloc(mem_size_t size);
extern void  mem_free(void *mem);

//void *mem_malloc(atbm_uint32 size);
//void  mem_free(void *mem);

#define atbm_kmalloc(x,y)    	pvPortMalloc(x) 
#define atbm_kfree(x)        	vPortFree(x)              
#define atbm_memcpy         	memcpy 
#define atbm_memset         	memset 
#define atbm_memmove  	memcpy
#define atbm_memcmp 	memcmp 
//#define atbm_calloc			atbm_calloc


static char * atbm_kzalloc(int size,int flags){  
	char * buf=	atbm_kmalloc(size,GFP_KERNEL);
	memset(buf,0,size);
	return  buf;
}

static unsigned long long * atbm_calloc(int size,int flags){  
	unsigned long long * buf=(unsigned long long *)	atbm_kmalloc(size*flags*sizeof(unsigned long long),GFP_KERNEL);
	memset(buf,0,size*flags*sizeof(unsigned long long));
	return  buf;
}

static void *atbm_realloc(void *ptr, atbm_size_t size)
{
	if(ptr){
		atbm_kfree(ptr);
	}
	return atbm_kmalloc(size, GFP_KERNEL);
}

static inline void * atbm_realloc_array(void *ptr, size_t nmemb, size_t size)
{
	if (size && nmemb > (~(size_t) 0) / size)
		return NULL;
	return atbm_realloc(ptr, nmemb * size);
}

static inline void * atbm_memdup(const void *src, atbm_size_t len)
{
	void *r = (void *)atbm_kmalloc(len, GFP_KERNEL);

	if (r && src)
		atbm_memcpy(r, src, len);
	return r;
}


#endif /* ATBM_OS_MEM_H */
