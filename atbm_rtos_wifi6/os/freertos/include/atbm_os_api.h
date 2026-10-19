/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/


#ifndef ATBM_OS_API_H
#define ATBM_OS_API_H
#include "atbm_type.h"
#include "atbm_debug.h"
#define __INLINE inline

#ifndef atbm_packed
#define atbm_packed __attribute__ ((packed))
#endif

static __INLINE atbm_uint32 get_unaligned_le32(const atbm_void *p)
{
	atbm_uint32 tmp;
	memcpy(&tmp, p, 4);

	return tmp;
}
char *atbm_strdup(const char *s);
atbm_uint32 freertos_os_random(atbm_void);
int atbm_wifi_get_mac(unsigned char *mac_addr);
int atbm_sdio_register_init(atbm_void);
int atbm_sdio_register_deinit(atbm_void);
int atbmwifi_event_OsCallback(atbm_void *prv,int eventid,atbm_void *param);

#define atbm_os_random freertos_os_random


#endif //ATBM_OS_API_H
