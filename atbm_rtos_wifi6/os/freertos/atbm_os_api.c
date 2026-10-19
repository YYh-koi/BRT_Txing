/**************************************************************************************************************
 * altobeam RTOS wifi hmac source code 
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#include "atbm_hal.h"
#include "atbm_api.h"

#include "atbm_sysops.h"
#include "atbm_os_api.h"

extern int atbm_sdio_probe(struct atbm_sdio_func *func,const struct atbm_sdio_device_id *id);
extern int atbm_sdio_disconnect(struct atbm_sdio_func *func);
extern struct atbmwifi_vif *  g_vmac;
extern struct atbmwifi_common g_hw_prv;

atbm_uint32 freertos_os_random(atbm_void)
{
	atbm_uint32 res = xTaskGetTickCount();
	static unsigned long seed = 0xDEADB00B;

	seed = ((seed & 0x007F00FF) << 7) ^
	    ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
	    (res << 13) ^ (res >> 9);    // using the clock too!
	return seed;
}

char *atbm_strdup(const char *s)
{
    int len = strlen(s) + 1;
    char *tmp = (char *)atbm_kmalloc(len, GFP_KERNEL);

    if (!tmp)
        return NULL;

    atbm_memcpy(tmp, s, len);

    return tmp;
}

/*
int atbm_wifi_get_mac(unsigned char *mac_addr)
{
	atbm_memcpy(mac_addr,g_hw_prv.mac_addr,6);

	return 0;
}
*/
int atbmwifi_event_OsCallback(atbm_void *prv,int eventid,atbm_void *param)
{
	struct atbmwifi_vif *priv = prv;
	if(atbmwifi_is_ap_mode(priv->iftype)){
		atbm_uint8 * staMacAddr ;
		switch(eventid){

			case ATBM_WIFI_DEAUTH_EVENT:
				wifi_printk(WIFI_ALWAYS,"event_OsCallback DEAUTH\n");
				
				break;
			case ATBM_WIFI_AUTH_EVENT:
				wifi_printk(WIFI_ALWAYS,"event_OsCallback %s %d\n", __func__, __LINE__);
				break;
			case ATBM_WIFI_ASSOC_EVENT:
				wifi_printk(WIFI_ALWAYS,"event_OsCallback %s %d\n", __func__, __LINE__);
				break;
			case ATBM_WIFI_ASSOCRSP_TXOK_EVENT: 
				staMacAddr =(atbm_uint8 *) param;
				wifi_printk(WIFI_ALWAYS,"event_OsCallback %s %d\n", __func__, __LINE__);
				break;
			case ATBM_WIFI_DEASSOC_EVENT:
				wifi_printk(WIFI_ALWAYS,"event_OsCallback %s %d\n", __func__, __LINE__);
				break;
			case ATBM_WIFI_JOIN_EVENT:			
				staMacAddr =(atbm_uint8 *) param;
				wifi_printk(WIFI_ALWAYS,"event_OsCallback %s %d\n", __func__, __LINE__);
				break;
			case ATBM_WIFI_ENABLE_NET_EVENT:
				wifi_printk(WIFI_ALWAYS,"event_OsCallback %s %d\n", __func__, __LINE__);
				break;
			default:
				break;
		}
	}
	else {	
		switch(eventid){
			case ATBM_WIFI_SCANSTART_EVENT:
				break;
			case ATBM_WIFI_SCANDONE_EVENT:				
//				event.event_type = WLAN_E_SCAN_COMPLETE;
//				WLAN_SYS_StatusCallback(&event);	
				break;
			case ATBM_WIFI_DEAUTH_EVENT:				
				
				break;
			case ATBM_WIFI_AUTH_EVENT:
				break;
			case ATBM_WIFI_ASSOC_EVENT:				
				
				break;
			case ATBM_WIFI_DEASSOC_EVENT:
				break;
			case ATBM_WIFI_JOIN_EVENT:
				wifi_printk(WIFI_ALWAYS,"ATBM_WIFI_JOIN_EVENT\n");
				if(atbmwifi_is_sta_mode(priv->iftype))
					wifi_printk(WIFI_ALWAYS,"ATBM_WIFI_JOIN_EVENT\n");
				break;
			default:
				break;
		}
	}
	return 0;
}
#if 0
static struct atbm_sdio_driver atmbwifi_driver;
static struct atbm_sdio_device_id atbm_sdio_ids[] = {
	//{ SDIO_DEVICE(SDIO_ANY_ID, SDIO_ANY_ID) },
	{ /* end: all zeroes */			},
};

int atbm_sdio_register_init(atbm_void)
{	
	printf("-----atbm_sdio_register_init-----");
	int ret =0;
	
	atbm_memcpy(atmbwifi_driver.name, "atbm6021",sizeof("atbm6021"));;
	//atmbwifi_driver.match_id_table	= atbm_sdio_ids;
	atmbwifi_driver.probe_func		= atbm_sdio_probe;
	atmbwifi_driver.discon_func		= atbm_sdio_disconnect;	
	ret = atbm_sdio_register(&atmbwifi_driver);
	if (ret){
		return ret;
	}
	return 0;
}
int atbm_sdio_register_deinit(atbm_void)
{
	atbm_sdio_deregister(&atmbwifi_driver);
	return 0;
}
#endif

