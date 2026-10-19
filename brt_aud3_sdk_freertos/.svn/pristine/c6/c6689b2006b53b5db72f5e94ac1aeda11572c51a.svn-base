#ifndef __RFC_DEBUG_UTILS_H_
#define __RFC_DEBUG_UTILS_H_

#include "btdatatype.h"
#include "autoconf_bt.h"
#ifdef CONFIG_OS_LINUX
#include "bluedroid/bluetooth.h"


static void print_a_addr(bt_bdaddr_t* bda)
{
	if(bda) {
		BRT_DBG("addr:%x:%x:%x:%x:%x:%x\n", bda->address[0], bda->address[1],bda->address[2], bda->address[3],bda->address[4], bda->address[5]);
	} else {
		BRT_DBG("bd: is null");
	}
}

static void print_b_addr(unsigned char * bda)
{
	if(bda) {
		BRT_DBG("addr:%x:%x:%x:%x:%x:%x\n", bda[0], bda[1],bda[2], bda[3],bda[4], bda[5]);
	} else {
		BRT_DBG("bd: is null");
	}
}

#else

static void print_a_addr(void* bda)
{

}

static void print_b_addr(unsigned char * bda)
{

}

#endif

#endif
