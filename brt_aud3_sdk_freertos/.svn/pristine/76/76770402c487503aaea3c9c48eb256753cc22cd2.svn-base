/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_manager.h
*
*	Description:	Internal Functions For HandFree
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _HLP_MANAGER_H_
#define _HLP_MANAGER_H_

#include "btdatatype.h"
#include "profile.h"

/* Version */
#define BT_APP_VERSION        "BRT_BSE_AUD3_RTOS_20260428_r33287"
//#define BT_APP_VERSION        "BW121,V5.7.3"
/* State */
enum {
    ST_MANAGER_UNINIT,
    ST_MANAGER_INIT,
    ST_MANAGER_READY,
    ST_MANAGER_END,
};

enum {
    EV_OTHER_LOCAL_STATE,
    EV_OTHER_PAIRIED_DEVICE_IND,
	EV_OTHER_PAIRIED_DEVICE_END,
    EV_OTHER_LOCAL_BD_IND,
    EV_OTHER_LOCAL_LE_BD_IND,
    EV_OTHER_LOCAL_MIC_GAIN,
    EV_OTHER_LOCAL_SPK_GAIN,
    EV_OTHER_LOCAL_SCAN_MODE_IND,
    EV_OTHER_LOCAL_DEVICE_NAME,
	EV_OTHER_CHANNEL_LOCAL_DEVICE_NAME,
    EV_OTHER_LOCAL_PINCODE,
    EV_OTHER_CURRENT_VOLUME,
    EV_OTHER_AUTO_ANSWER_CONNECT_IND,
	EV_OTHER_DUT_MODE_CONFIGURE,
	EV_EVENT_OK,
};

#define MANAGER_DEFAULT_LOCAL_DEVICE_NAME        "bluetooth_auto"
#define MANAGER_DEFAULT_LOCAL_DEVICE_PINCODE	"0000"

#define LINK_KEY_LEN	16
#define PIN_CODE_LEN	8
#ifdef CONFIG_UNLIMIT_RESOURCE
#define DEV_NAME_LEN	248  //The max size in bluetooth spec.
#else
#define DEV_NAME_LEN	31
#endif
#define CLS_DEV_LEN		3
#define MAX_SERVICE_NUM 16

#define	LOCAL_DEV_SET_AUTO_CONNECT	0x01
#define	LOCAL_DEV_SET_AUTO_ANSWER	0x02
#define LOCAL_DEV_SET_NREC_DISABLE  0X04
#define	LOCAL_DEV_FLAG_ENABLE		0x80

#define EEPROM_START_ADDR	0x00DA
#define EEPROM_END_ADDR		0x016B
struct  eir_uuid_t{
    UINT8 uuid_len;
    UINT8 uuid[240];
};

typedef struct tag_bt_remote_device {
	UINT8 bd[BD_ADDR_LEN];
	UINT8 link_key[LINK_KEY_LEN];
	UINT8 name[DEV_NAME_LEN + 1];
	UINT8 bond_state;
	UINT32 class_of_device;
	UINT8 reconn_profile;
	UINT8 carplay;
	UINT8 rssi;
	UINT8 cap;
	UINT8 num_service;
	UINT16 services[MAX_SERVICE_NUM];
	struct  eir_uuid_t uuid;
} bt_remote_device_t;

typedef struct tag_bt_remote_name {
	UINT8 bd[BD_ADDR_LEN];
	UINT8 name[DEV_NAME_LEN + 1];
}tag_bt_remote_name;

struct UARTConfig {
	UINT32 Baudrate;  //!< Baudrate
	UINT8 Databits;   //!< Data bit length
	UINT8 Stopbits;   //!< Stop bit length
	UINT8 Parity;     //!< Parity
};

struct LocalDevInfo {
	UINT8 flag;
	UINT8 bd_addr[BD_ADDR_LEN];
	UINT8 pin[PIN_CODE_LEN+1];
	UINT8 name[DEV_NAME_LEN+1];
	UINT8 cls[CLS_DEV_LEN];
};

struct DevInfoGotEvStru {
	unsigned char data_plen;
	unsigned char data[1];
};/* 0x02[LenI:1] */

struct ManagerUserDataStru {
	struct BtList *dev_list;
	struct LocalDevInfo *local_info;
	struct UARTConfig *uart;

	UINT8 last_bd_addr[BD_ADDR_LEN];
	UINT8 flag_bt_e2prom_enable;
	UINT8 state;
	module_event_cbk_t event_cbk;
};

extern profile_t g_mgr_profile;
extern struct ManagerUserDataStru *manager_user_data;
#define MANAGER_USER_DATA		(manager_user_data)
#define MANAGER_STATE			(manager_user_data->state)

/**********************/
#ifdef CONFIG_SAVE_INFO_TO_MODULE
#define MAX_DEV_COUNT		1
#else
#define MAX_DEV_COUNT		6
#endif
#define DEV_INFO_LEN	(sizeof(struct LocalDevInfo) + sizeof(struct UARTConfig) + 1 + MAX_DEV_COUNT*sizeof(bt_remote_device_t) + 1)
#define DEV_INFO_POS_RMT_DEVS		(sizeof(struct LocalDevInfo) + sizeof(struct UARTConfig))
#define DEV_INFO_LEN_RMT_DEVS		(1 + hlp_manager_get_device_count()*sizeof(bt_remote_device_t))
#define DEV_INFO_POS_CUR_CBINFO	(sizeof(struct LocalDevInfo) + sizeof(struct UARTConfig) + 1 + sizeof(bt_remote_device_t) - sizeof(struct CBFsmUserDataStru) - 1)
#define DEV_INFO_LEN_CUR_CBINFO	(sizeof(struct CBFsmUserDataStru) + 1)

#define DEV_INFO_VALID_BYTE				0xAB


void hlp_manager_init(module_event_cbk_t event_cbk);
void hlp_manager_server_start(void);
void hlp_manager_server_stop(void);
void hlp_manager_done(void);
void hlp_manager_remove_device(UINT8 index);
void hlp_manager_remove_bond(UINT8 *bd);
void hlp_manager_insert_link_key(bt_remote_device_t *p_device_info);
void hlp_manager_insert_device_name(UINT8 *bd, UINT8 *p_name);
void hlp_manager_save_configure(int offset,int nLen);
void hlp_manager_load_configure(struct DevInfoGotEvStru *dev_info_got);
UINT8 hlp_manager_add_device(bt_remote_device_t *pDevArg, UINT8 flag);
void hlp_manager_set_status(UINT8 status);
UINT8 hlp_manager_get_status(void);
UINT8 *hlp_manager_get_local_bd_addr(void);
UINT8 *hlp_manager_get_local_pincode(void);
UINT8 *hlp_manager_get_local_device_class(void);
UINT8 *hlp_manager_get_local_name(void);
void hlp_manager_set_last_device_bd_addr(UINT8 *bd);
UINT8 *hlp_manager_get_last_device_bd_addr(void);
UINT8 app_dev_get_address(UINT8 i, UINT8 *bd);
UINT8 *hlp_dev_get_link_key(UINT8 *bd);
UINT8 hlp_manager_get_device_count(void);
void hlp_manager_save_local_device_bd(UINT8 *bd);
void hlp_manager_set_local_device_name(UINT8 *p_name);
void hlp_manager_set_local_device_name_ex(UINT8 *name);
void hlp_manager_set_local_device_pin(UINT8 *p_pin);
void hlp_manager_query_device_list(void);
void hlp_manager_query_local_device_name(void);
void hlp_manager_query_local_device_pin(void);
void hlp_manager_query_local_device_bd_addr(void);
void hlp_manager_get_local_device_bd_addr(char *p_addr);
void hlp_manager_query_local_configure(void);
void hlp_manager_insert_reconn_profile(UINT8* bd, UINT8 reconn_state);
UINT8 hlp_manager_get_reconnet_profile(UINT8 * bd);
BOOL hlp_get_nrec_disable_config();
void hlp_manager_enter_dut_mode(void);
void hlp_manager_leave_dut_mode(void);
void hlp_set_max_bond_devices(int value);
bt_remote_device_t *hlp_manager_find_target_device(UINT8*bd);
void hlp_manager_get_romete_name_ex(UINT8 *bd, char *name);
void hlp_manager_get_romete_info_ex(UINT8 *bd, char *name);
struct BtList *hlp_manager_get_device_list(void);
void hlp_manager_insert_last_connected_device(void *arg);
#endif /*_HLP_MANAGER_H_*/


