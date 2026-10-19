/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_manager.c
*
*	Description:	Internal Functions For Bluetooth Manager
*
*
*	Create Date:	2014/06/22
***********************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
//#include <termios.h>

#include "internal_manager.h"
#include "global.h"
#include "btlist.h"
#include "gap_ui.h"
#include "internal_gap.h"
#include "brt_trace.h"
#include "hci_ui.h"

#define LOG(fmt, ...) bse_log_info("hlp_manager", fmt, ## __VA_ARGS__)

static int max_bind_devices = 6;
struct ManagerUserDataStru *manager_user_data = NULL;
static UINT8 brname_buff[32];
static UINT8 brpinc_buff[16];

static void manager_event_indicate(unsigned int event, void *param)
{
	if (manager_user_data != NULL) {
		if (manager_user_data->event_cbk != NULL) {
			manager_user_data->event_cbk((PROFILE_ID_MGR << 8) | event, param);
		}
	}
}

static void hlp_set_nrec_disable_config()
{
#ifdef CONFIG_OS_FREERTOS
	manager_user_data->local_info->flag |= LOCAL_DEV_SET_NREC_DISABLE;
#else
	int val_len = 16;
	UINT8 *val = NEW(val_len);
	memset(val, 0, val_len);
	GetLocalProp("nrec_disable", val, &val_len);
	if((int)strtol(val, NULL, 16) == 1) {
		manager_user_data->local_info->flag |= LOCAL_DEV_SET_NREC_DISABLE;
	}
	FREE(val);
#endif	
}

BOOL hlp_get_nrec_disable_config()
{
	return manager_user_data->local_info->flag & LOCAL_DEV_SET_NREC_DISABLE ? TRUE : FALSE;
}

void hlp_manager_init(module_event_cbk_t event_cbk)
{
	int name_len = DEV_NAME_LEN;

	manager_user_data = (struct ManagerUserDataStru *)NEW(sizeof(struct ManagerUserDataStru));
	memset(manager_user_data, 0, sizeof(struct ManagerUserDataStru));
	manager_user_data->event_cbk = event_cbk;
	manager_user_data->dev_list = BtNewListNew(sizeof(bt_remote_device_t));

	manager_user_data->local_info = (struct LocalDevInfo *)NEW(sizeof(struct LocalDevInfo));
	memset(manager_user_data->local_info, 0, sizeof(struct LocalDevInfo));
	manager_user_data->local_info->cls[0] = 0x08;
	manager_user_data->local_info->cls[1] = 0x04;
	manager_user_data->local_info->cls[2] = 0x24;

	GetLocalProp("name", manager_user_data->local_info->name, &name_len);
	if(!manager_user_data->local_info->name[0]) {
		LOG("GetLocalProp failed");
		strcpy(manager_user_data->local_info->name, MANAGER_DEFAULT_LOCAL_DEVICE_NAME);
	}
	if(brname_buff[0])
	{
		LOG("GetLocalProp failed");
		memset(manager_user_data->local_info->name, 0, 32);
		//strcpy(manager_user_data->local_info->name, MANAGER_DEFAULT_LOCAL_DEVICE_NAME);
		memcpy(manager_user_data->local_info->name, brname_buff, 32);
	}
	//printf("hlp_manager_init manager_user_data->local_info->name = %s\n", manager_user_data->local_info->name);
	strcpy(manager_user_data->local_info->pin, MANAGER_DEFAULT_LOCAL_DEVICE_PINCODE);
	manager_user_data->local_info->flag = 0;

	manager_user_data->uart = (struct UARTConfig *)NEW(sizeof(struct UARTConfig));
	memset(manager_user_data->uart, 0, sizeof(struct UARTConfig));
	manager_user_data->uart->Baudrate = 115200;
	manager_user_data->uart->Databits = 8;
	manager_user_data->uart->Stopbits = 0;
	manager_user_data->uart->Parity = 0;

	manager_user_data->flag_bt_e2prom_enable = FALSE;

	hlp_manager_set_status(ST_MANAGER_INIT);
	hlp_manager_load_configure(NULL);
}

void hlp_manager_server_start(void)
{
	hlp_manager_set_status(ST_MANAGER_READY);
}

void hlp_manager_server_stop(void)
{
	hlp_manager_set_status(ST_MANAGER_INIT);
}

void hlp_manager_done(void)
{
	if (manager_user_data == NULL)
		return;

	if (manager_user_data->dev_list != NULL) {
		NewListRemoveAll(manager_user_data->dev_list);
		BtNewListFree(manager_user_data->dev_list);
	}
	if (manager_user_data->local_info != NULL) {
		FREE(manager_user_data->local_info);
	}
	if (manager_user_data->uart != NULL) {
		FREE(manager_user_data->uart);
	}
	FREE(manager_user_data);
	manager_user_data = NULL;

}

void hlp_manager_remove_device(UINT8 index)
{
	struct BtList *list = MANAGER_USER_DATA->dev_list;

	if (index == 0) {
		NewListRemoveAll(list);
	} else {
		bt_remote_device_t *pDev = (bt_remote_device_t *)(list->head);
		UINT8 i = 0;

		while (pDev != NULL) {
			i++;
			if (i == index) {
				NewListRemoveAt(list, (UINT8 *)pDev);
				BtListMemberFree(pDev);

				break;
			}
			pDev = (bt_remote_device_t *)NEXT(pDev);
		}
	}

	hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
}

void hlp_manager_remove_bond(UINT8 *bd)
{
	struct BtList *list = MANAGER_USER_DATA->dev_list;
	bt_remote_device_t *pDev = (bt_remote_device_t *)(list->head);

	if (bd == NULL) {
		return;
	}

	while (pDev != NULL) {
		if (memcmp(bd, pDev->bd, BD_ADDR_LEN) == 0) {
			NewListRemoveAt(list, (UINT8 *)pDev);
			BtListMemberFree(pDev);
			hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
			break;
		}
		pDev = (bt_remote_device_t *)NEXT(pDev);
	}
}

void hlp_manager_insert_reconn_profile(UINT8* bd, UINT8 reconn_state)
{
	struct BtList *list = MANAGER_USER_DATA->dev_list;
	bt_remote_device_t *pDev = (bt_remote_device_t *)(list->head);

	if (bd == NULL) {
		return;
	}

	while (pDev != NULL) {
		if (memcmp(bd, pDev->bd, BD_ADDR_LEN) == 0) {
			pDev->reconn_profile = reconn_state;
			hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
			break;
		}
		pDev = (bt_remote_device_t *)NEXT(pDev);
	}

	return;
}

void hlp_manager_insert_link_key(bt_remote_device_t *p_device_info)
{
	if(p_device_info != NULL) {
#ifdef SAVE_BONDED_DEVICES_TO_CONFIG_FILE
		char address[2 * BD_ADDR_LEN + 1];
		unsigned char *remote_bd;
		char key[6];
		UINT8 count = 0;
		UINT8 i = 0;
#endif
		if(hlp_manager_add_device(p_device_info, 0)) {
			LOG("Device Added in LinkKeyNotify");
			hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
		}
#ifdef SAVE_BONDED_DEVICES_TO_CONFIG_FILE
		memset(key, 0, 6);
		bt_remote_device_t *rmt_dev = (bt_remote_device_t *)MANAGER_USER_DATA->dev_list->head;

		count = hlp_manager_get_device_count();
		count = count > 10 ? 10 : count;
		snprintf(key, 6, "%d", count);
		SetLocalProp("pairlisttotal", key);

		while (rmt_dev != NULL && i < 10) {
			remote_bd = rmt_dev->bd;
			memset(address, 0, 2 * BD_ADDR_LEN + 1);
			snprintf(address, 2 * BD_ADDR_LEN + 1, "%02X%02X%02X%02X%02X%02X", remote_bd[5], remote_bd[4], remote_bd[3],
			         remote_bd[2], remote_bd[1], remote_bd[0]);
			snprintf(key, 6, "addr%1d", i);
			SetLocalProp(key, address);

			snprintf(key, 6, "name%1d", i);
			if (rmt_dev->name[0]) {
				SetLocalProp(key, rmt_dev->name);
			} else {
				SetLocalProp(key, "noname");
			}
			i++;
			rmt_dev = (bt_remote_device_t *)NEXT_MEM(MANAGER_USER_DATA->dev_list, rmt_dev);
		}
#endif
	}
}

void hlp_manager_insert_uuid(bt_remote_device_t *p_device_info)
{
    if(p_device_info != NULL) {
        LOG("[manager]hlp_manager_insert_uuid,name[%s]:%d", p_device_info->name, p_device_info->carplay);
        if(hlp_manager_add_device(p_device_info, 0)) {
            LOG("[manager]Device Added in Uuid,name[%s]:%d", p_device_info->name, p_device_info->carplay);
            hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
        }
    }
}

static tag_bt_remote_name pair_rname;
void hlp_manager_insert_device_name(UINT8 *bd, UINT8 *p_name)
{
	LOG("hlp_manager_insert_device_name-[%s]", p_name);
	if(p_name != NULL) {

		memset(pair_rname.bd, 0, BD_ADDR_LEN);
		memset(pair_rname.name, 0, DEV_NAME_LEN + 1);

		bt_remote_device_t *p_device_info = NULL;
		bt_remote_device_t *p_device_info_temp = NULL;

		//debug
		bt_remote_device_t *p_device_info_ex = NULL;
		p_device_info_ex = hlp_manager_find_target_device(bd);
		if(p_device_info_ex != NULL)
		{
			LOG("p_device_info_ex->bd: %02x %02x %02x %02x %02x %02xp_device_info_ex-name = %s", p_device_info_ex->bd[0], p_device_info_ex->bd[1],
			p_device_info_ex->bd[2], p_device_info_ex->bd[3], p_device_info_ex->bd[4], p_device_info_ex->bd[5], p_device_info_ex->name);
		}
		int len;
		p_device_info = (bt_remote_device_t *)NewListFind(MANAGER_USER_DATA->dev_list, bd, BD_ADDR_LEN);
		if(p_device_info != NULL) {
			LOG("p_device_info->bd: %02x %02x %02x %02x %02x %02xp_device_info_ex-name = %s", p_device_info->bd[0], p_device_info->bd[1],
			p_device_info->bd[2], p_device_info->bd[3], p_device_info->bd[4], p_device_info->bd[5], p_device_info->name);
			
			p_device_info_temp = (bt_remote_device_t *)NEW(sizeof(bt_remote_device_t));
			memcpy(p_device_info_temp, p_device_info, sizeof(bt_remote_device_t));
			memset(p_device_info_temp->name, 0, DEV_NAME_LEN + 1);

			len = strlen(p_name);
			len = len > DEV_NAME_LEN ? DEV_NAME_LEN : len;
			utf8_strncpy(p_device_info_temp->name, p_name, len);

			if (hlp_manager_add_device(p_device_info_temp,0)) {
				LOG("Device Added in RMTDevNameGot");
				hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
			}
			FREE(p_device_info_temp);
		} else {
			
			len = strlen(p_name);
			len = len > DEV_NAME_LEN ? DEV_NAME_LEN : len;
			utf8_strncpy(pair_rname.name, p_name, len);

			memcpy(pair_rname.bd, bd, BD_ADDR_LEN);
		}

		
		/*p_device_info_temp = (bt_remote_device_t *)NEW(sizeof(bt_remote_device_t));
		if(p_device_info != NULL) {
			memcpy(p_device_info_temp, p_device_info, sizeof(bt_remote_device_t));
		}
		memset(p_device_info_temp->name, 0, DEV_NAME_LEN + 1);
		memset(p_device_info_temp->bd, 0, BD_ADDR_LEN);

		len = strlen(p_name);
		len = len > DEV_NAME_LEN ? DEV_NAME_LEN : len;
		utf8_strncpy(p_device_info_temp->name, p_name, len);
		memcpy(p_device_info_temp->bd, bd, BD_ADDR_LEN);

		if (hlp_manager_add_device(p_device_info_temp,0)) {
			LOG("Device Added in RMTDevNameGot");
			hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
		}
		FREE(p_device_info_temp);*/
	}
}

void hlp_manager_get_romete_info_ex(UINT8 *bd, char *name)
{
	memcpy(name, pair_rname.name, strlen(pair_rname.name));
	memcpy(bd, pair_rname.bd, BD_ADDR_LEN);
}

void hlp_manager_get_romete_name_ex(UINT8 *bd, char *name)
{
	LOG("hlp_manager_get_romete_name_ex %s ", pair_rname.name);
	if(memcmp(bd, pair_rname.bd, BD_ADDR_LEN) == 0)
		memcpy(name, pair_rname.name, strlen(pair_rname.name));
}

void hlp_manager_insert_device_info(UINT8 type, UINT8 *param)
{
	static bt_remote_device_t *p_device_info = NULL;
	static UINT8 count = 0;

	if (p_device_info == NULL) {
		p_device_info = (bt_remote_device_t *)NEW(sizeof(bt_remote_device_t));
		memset(p_device_info, 0, sizeof(bt_remote_device_t));
	}

	if (type == 1) {
		memcpy(MANAGER_USER_DATA->last_bd_addr, param, BD_ADDR_LEN);
		memcpy(p_device_info->bd, param, BD_ADDR_LEN);
		count += type;
	} else if (type == 2) {
		memcpy(p_device_info->link_key, param, LINK_KEY_LEN);
		count += type;
	} else if (type == 3) {
		memcpy(p_device_info->name, param, (strlen(param) > DEV_NAME_LEN) ? DEV_NAME_LEN : strlen(param));
		count += type;
	}

	if (count >= 3) {
		if (hlp_manager_add_device(p_device_info, 0) == 1) {
			hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
		}
	}
	count = 0;
	FREE(p_device_info);
	p_device_info = NULL;
}

void hlp_manager_insert_last_connected_device(void *arg)
{
	bt_remote_device_t *pDev;
	bt_remote_device_t *pDevArg;
	pDevArg = (bt_remote_device_t *)NEW(sizeof(bt_remote_device_t));
	pDev = (bt_remote_device_t *)NewListFind(MANAGER_USER_DATA->dev_list, MANAGER_USER_DATA->last_bd_addr, BD_ADDR_LEN);
	if (pDev) {
		memset(pDevArg,0,sizeof(bt_remote_device_t));
		memcpy(pDevArg,pDev,sizeof(bt_remote_device_t));
		if(hlp_manager_add_device(pDevArg,0)) {
			LOG("Device Added in slc established");
			hlp_manager_save_configure(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
		}
	}
	FREE(pDevArg);
}

#if 0
void SLCEstablished_t(struct FsmInst *fi, UCHAR event, void *arg)
{
	bt_remote_device_t *pDev;
	bt_remote_device_t *pDevArg;
	pDevArg = (bt_remote_device_t *)NEW(sizeof(bt_remote_device_t));
	pDev = (bt_remote_device_t *)NewListFind(DEVMGR_FI_USER_DATA->dev_list,DEVMGR_FI_USER_DATA->last_bd_addr,BD_ADDR_LEN);
	if (pDev) {
		memcpy(pDevArg,pDev,sizeof(bt_remote_device_t));
	} else {
		memset(pDevArg,0,sizeof(bt_remote_device_t))	;
		memcpy(pDevArg->bd,DEVMGR_FI_USER_DATA->last_bd_addr,BD_ADDR_LEN);
	}
	if(hlp_manager_add_device(pDevArg,0)) {
		LOG("Device Added in SLCEstablished");
		DevInfoSet(DEV_INFO_POS_RMT_DEVS, DEV_INFO_LEN_RMT_DEVS);
	}
	FREE(pDevArg);
}
#endif

void hlp_manager_save_configure(int offset,int nLen)
{
	bt_remote_device_t *pDev;
	UINT16 len = DEV_INFO_LEN;
	UINT8 *pData4DevCount = NULL;
	UINT8 *data = NULL;
	UINT8 *p_data = NULL;

	data = p_data = (UINT8 *)NEW(len);
	memset(data, 0, len);
	memcpy(data, MANAGER_USER_DATA->local_info, sizeof(struct LocalDevInfo));
	data += sizeof(struct LocalDevInfo);
	memcpy(data, MANAGER_USER_DATA->uart, sizeof(struct UARTConfig));
	data += sizeof(struct UARTConfig);
	pData4DevCount = data;
	pData4DevCount[0] = 0;
	data += 1;
	pDev = (bt_remote_device_t *)(MANAGER_USER_DATA->dev_list->head);
	while (pDev) {
		LOG("Device %d:%s",pData4DevCount[0],pDev->name);
		memcpy(data, pDev, sizeof(bt_remote_device_t));
		data += sizeof(bt_remote_device_t);
		pData4DevCount[0]++;
		if(MAX_DEV_COUNT == pData4DevCount[0]) {
			break;
		}
		pDev = (bt_remote_device_t *)NEXT_MEM(MANAGER_USER_DATA->dev_list, (UCHAR *)pDev);
	}
	//Create Check Sum Field
	{
		p_data[len-1] = DEV_INFO_VALID_BYTE;
	}

	if(MANAGER_USER_DATA->flag_bt_e2prom_enable == TRUE) {
#ifdef CONFIG_BT_CHIP_ISSC
		app_comm_write_eeprom(1, EEPROM_START_ADDR, len, p_data);
#endif
	} else {
		if (MANAGER_USER_DATA->local_info->flag & LOCAL_DEV_FLAG_ENABLE ) {
#ifdef CONFIG_FLASH_SAVE_FILE
			WriteTo(1, 0, len, p_data);
#else
			WriteTo(1, offset, nLen, p_data + offset);
#endif
		} else {
			p_data[0] |= LOCAL_DEV_FLAG_ENABLE;
			WriteTo(1, 0, len, p_data);
		}
	}

	FREE(p_data);
}

void hlp_manager_load_configure(struct DevInfoGotEvStru *dev_info_got)
{
	UINT16 len = 0;
	UINT8 *pDevInfo = NULL;

	MANAGER_USER_DATA->flag_bt_e2prom_enable = (dev_info_got != NULL) ? (TRUE) : (FALSE);

	if (MANAGER_USER_DATA->flag_bt_e2prom_enable == FALSE) {
		len = DEV_INFO_LEN;
		pDevInfo = NEW(len);
		memset(pDevInfo, 0, len);
		ReadFrom(1, 0, len, pDevInfo);
	} else {
		pDevInfo = (dev_info_got->data + 4);
		len = dev_info_got->data_plen - 4;
	}

	if (pDevInfo != NULL) {
		if (DEV_INFO_VALID_BYTE == pDevInfo[len-1]) {
			UINT8 *data = pDevInfo;
			UINT8 flag = data[0];

			LOG("%d data got", len); //4
			if (flag & LOCAL_DEV_FLAG_ENABLE) {
				bt_remote_device_t *pDev;
				UINT8 nDevCount, i;

				memcpy(MANAGER_USER_DATA->local_info, data, sizeof(struct LocalDevInfo));
				data += sizeof(struct LocalDevInfo);
				memcpy(MANAGER_USER_DATA->uart, data, sizeof(struct UARTConfig));
				data += sizeof(struct UARTConfig);
				nDevCount = data[0];
				data += 1;

				// Clear the device list before fill the loaded devices
				NewListRemoveAll(MANAGER_USER_DATA->dev_list);

				for(i=0; i<nDevCount; i++) {
					pDev = (bt_remote_device_t *)BtListMemberNew(sizeof(bt_remote_device_t));
					memcpy(pDev, data, sizeof(bt_remote_device_t));
					data += sizeof(bt_remote_device_t);
					NewListAddTail(MANAGER_USER_DATA->dev_list, (UINT8 *)pDev);
					hlp_gap_load_paired_device(pDev);
					if (i == 0) {
						memcpy(MANAGER_USER_DATA->last_bd_addr, pDev->bd, BD_ADDR_LEN);
					}
				}
			}
		} else {
			LOG("DevInfoGot Check Sum Fail.");
		}
	}
	if (MANAGER_USER_DATA->flag_bt_e2prom_enable == FALSE) {
		FREE(pDevInfo);
	}
	hlp_set_nrec_disable_config();
}

UINT8 hlp_manager_add_device(bt_remote_device_t *pDevArg, UINT8 flag)
{
	bt_remote_device_t *pDev = NULL;
	bt_remote_device_t *pHead = NULL;
	bt_remote_device_t *pTail = NULL;
	struct BtList *p_device_list = MANAGER_USER_DATA->dev_list;
	UINT8  bAdded = 0;

	if (pDevArg != NULL) {
		pHead = (bt_remote_device_t *)NewListGetHead(p_device_list);
		pTail = (bt_remote_device_t *)NewListGetTail(p_device_list);
		if (((flag == 0) && (pHead != NULL) && (memcmp(pHead, pDevArg, sizeof(bt_remote_device_t)) == 0))
		    ||((flag == 1) && (pTail != NULL) && (memcmp(pTail, pDevArg, sizeof(bt_remote_device_t)) == 0))) {

			LOG("Do not Need to Change Database");
			bAdded = 0;
		} else {
			bAdded = 1;
			pDev = (bt_remote_device_t *)NewListFind(p_device_list, pDevArg->bd, BD_ADDR_LEN);
			if (pDev) {
				LOG("Device Found while Add Device");
				NewListRemoveAt(p_device_list, (UINT8 *)pDev);
			} else {
				LOG("It is a new Device");
				pDev = (bt_remote_device_t *)BtListMemberNew(sizeof(bt_remote_device_t));
			}
			memcpy(pDev, pDevArg, sizeof(bt_remote_device_t));
			if (flag == 0) {		//Head
				NewListAddHead(p_device_list, (UINT8 *)pDev);
				if (hlp_manager_get_device_count() > max_bind_devices) {//MAX_DEV_COUNT
					pDev = (bt_remote_device_t *)NewRemoveTail(p_device_list);
					BtListMemberFree(pDev);
				}
			} else if (flag == 1) {	//Tail
				NewListAddTail(p_device_list, (UINT8 *)pDev);
				if (hlp_manager_get_device_count() > max_bind_devices) {//MAX_DEV_COUNT
					pDev = (bt_remote_device_t *)NewRemoveHead(p_device_list);
					BtListMemberFree(pDev);
				}
			}
		}

		if (pHead != NULL) {
			BtListMemberFree(pHead);
		}
		if (pTail != NULL) {
			BtListMemberFree(pTail);
		}
	}

	return bAdded;
}

void hlp_set_max_bond_devices(int value)
{
	max_bind_devices = value;
	manager_event_indicate(EV_EVENT_OK, NULL);
	//manager_user_data->event_cbk();
}

void hlp_manager_set_status(UINT8 status)
{
	LOG("hlp_manager_set_status, %d", status);

	MANAGER_STATE = status;
}

UINT8 hlp_manager_get_status(void)
{
	return MANAGER_STATE;
}

struct BtList *hlp_manager_get_device_list(void)
{
	return MANAGER_USER_DATA->dev_list;
}

UINT8 *hlp_manager_get_local_bd_addr(void)
{
	return MANAGER_USER_DATA->local_info->bd_addr;
}

UINT8 *hlp_manager_get_local_pincode(void)
{
	return MANAGER_USER_DATA->local_info->pin;
}

UINT8 *hlp_manager_get_local_device_class(void)
{
	manager_user_data->local_info->cls[0] = 0x08;
	manager_user_data->local_info->cls[1] = 0x04;
	manager_user_data->local_info->cls[2] = 0x24;
	return MANAGER_USER_DATA->local_info->cls;
}

UINT8 *hlp_manager_get_local_name(void)
{
	return MANAGER_USER_DATA->local_info->name;
}

void hlp_manager_set_last_device_bd_addr(UINT8 *bd)
{
	if (bd != NULL) {
		memcpy(MANAGER_USER_DATA->last_bd_addr, bd, BD_ADDR_LEN);
	} else {
		memset(MANAGER_USER_DATA->last_bd_addr, 0, BD_ADDR_LEN);
	}
}

UINT8 *hlp_manager_get_last_device_bd_addr(void)
{
	return MANAGER_USER_DATA->last_bd_addr;
}

UINT8 hlp_manager_get_reconnet_profile(UINT8 * bd)
{
	bt_remote_device_t *pDev = (bt_remote_device_t *)NewListFind(MANAGER_USER_DATA->dev_list, bd, BD_ADDR_LEN);

	if (pDev != NULL) {
		return pDev->reconn_profile;
	}

	return 0;
}


UINT8 *hlp_dev_get_link_key(UINT8 *bd)
{
	bt_remote_device_t *pDev = (bt_remote_device_t *)NewListFind(MANAGER_USER_DATA->dev_list, bd, BD_ADDR_LEN);

	if (pDev != NULL) {
		return pDev->link_key;
	} else {
		return NULL;
	}
}

UINT8 app_dev_get_address(UINT8 i, UINT8 *bd)
{
	unsigned char j = 0;
	if(MANAGER_USER_DATA == NULL || MANAGER_USER_DATA->dev_list == NULL)
	{
		LOG("%s, MANAGER list is NULL---------------------------------\n", __func__);
		return 0;
	}
	bt_remote_device_t *p = (bt_remote_device_t *)MANAGER_USER_DATA->dev_list->head;
	while (p != NULL) {
		LOG("%d:%02X:%02X:%02X:%02X:%02X:%02X",j,p->bd[5],p->bd[4],p->bd[3],p->bd[2],p->bd[1],p->bd[0]);
		if(i == j) {
			if(bd) {
				memcpy(bd,p->bd,BD_ADDR_LEN);
			}
			return 1;
		}
		j++;
		p = (bt_remote_device_t *)NEXT_MEM(MANAGER_USER_DATA->dev_list, p);
	}
	return 0;
}

UINT8 hlp_manager_get_device_count(void)
{
	unsigned char nCount = 0;
	bt_remote_device_t *pDev = (bt_remote_device_t *)(MANAGER_USER_DATA->dev_list->head);
	while (pDev) {
		nCount++;
		pDev = (bt_remote_device_t *)NEXT_MEM(MANAGER_USER_DATA->dev_list,(UCHAR *)pDev);
	}
	return nCount;
}

void hlp_manager_query_device_list(void)
{
#ifdef CONFIG_UNLIMIT_RESOURCE
	UINT8 device_info[274];
#else
	UINT8 device_info[64];
#endif
	UINT8 i = 0;
	bt_remote_device_t *p_device = (bt_remote_device_t *)MANAGER_USER_DATA->dev_list->head;

	while (p_device != NULL) {

			snprintf(device_info, sizeof(device_info), "%d%08X%02X%02X%02X%02X%02X%02X%s", i + 1, p_device->class_of_device,
					p_device->bd[5],
					p_device->bd[4],
					p_device->bd[3],
					p_device->bd[2],
					p_device->bd[1],
					p_device->bd[0],
					p_device->name);
			manager_event_indicate(EV_OTHER_PAIRIED_DEVICE_IND, device_info);
			i++;

		p_device = (bt_remote_device_t *)NEXT_MEM(MANAGER_USER_DATA->dev_list, p_device);
	}
	manager_event_indicate(EV_OTHER_PAIRIED_DEVICE_END, NULL);
	manager_event_indicate(EV_EVENT_OK, NULL);
}

void hlp_manager_save_local_device_bd(UINT8 *bd)
{
#if 0
	int fd = -1;
	int sz = -1;
	char path[128] = {0};
	char bdaddr[12] = {0};
	char curr_bdaddr[12] = {0};
#endif
	if (bd) {
		memcpy(MANAGER_USER_DATA->local_info->bd_addr, bd, BD_ADDR_LEN);
#if 0
		/*if(access(dirname("/data/misc/brlink/BD_ADDR.INI"),F_OK) == 0)
		{
			mkdir(dirname("/data/misc/brlink/BD_ADDR.INI"),0777);
		}*/
		strcpy(path, "/data/misc/brlink/BD_ADDR.INI");

		//loop:
		fd = open(path, O_RDWR | O_CREAT, 0777);//O_RDONLY
		LOG("read_default_bdaddr fd:%d",fd);
		LOG("read_default_bdaddr-------------------------------------------------------fd:%d\n",fd);
		if (fd > 0)
		{
			
			sz = read(fd, bdaddr, 20);
			LOG("read_default_bdaddr sz:%d\n", sz);
			if (sz < 0) {
				fprintf(stderr, "read(%s) failed: %s (%d)", path, strerror(errno),
						errno);
				close(fd);
				LOG( "bd read fail, size<0(%s)\r\n", strerror(errno));
				return ;
			}/*else if (sz != 12 && sz != 0) {
				//fprintf(stderr, "read(%s) unexpected size %d", path, sz);
				//close(fd);
				LOG( "bd  read(%d) != 12\r\n", sz);
				ftruncate(fd, sz);
				remove("/data/misc/brlink/BD_ADDR.INI");
				sleep(1);
				goto loop;
			}*/
		

			sprintf(curr_bdaddr, "%02x%02x%02x%02x%02x%02x", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
			LOG("curr_bdaddr = %s bdaddr = %s\n", curr_bdaddr, bdaddr);
			if(memcmp(bdaddr, curr_bdaddr, 12) != 0)
			{
				ftruncate(fd, 0);
				lseek(fd, 0, SEEK_SET);

				sz = write(fd, curr_bdaddr, strlen(curr_bdaddr));
				LOG("write_default_bdaddr sz:%d\n", sz);
				if (sz < 0) {
					fprintf(stderr, "write(%s) failed: %s (%d)", path, strerror(errno),
							errno);
					close(fd);
					LOG("bd write fail, size<0(%s)\r\n", strerror(errno));
					return ;
				} else if (sz != 12) {
					fprintf(stderr, "write(%s) unexpected size %d", path, sz);
					close(fd);
					LOG( "bd  write(%d) != 12\r\n", sz);
					return;
				}
				//close(fd);
			}
		}
		if(fd > 0)
			close(fd);
		LOG( "bd_addr(%s)\r\n", bdaddr);
#endif	
	}
}

void hlp_manager_set_local_device_name_ex(UINT8 *name)
{
	memset(brname_buff, 0, 128);
	memcpy(brname_buff, name, strlen(name));
}

void hlp_manager_set_local_device_name(UINT8 *p_name)
{
	if (p_name != NULL) {
		memset(MANAGER_USER_DATA->local_info->name, 0, DEV_NAME_LEN+1);
		memcpy(MANAGER_USER_DATA->local_info->name
		       , p_name
		       , strlen(p_name) > DEV_NAME_LEN ? DEV_NAME_LEN : strlen(p_name));

		hlp_manager_save_configure(0, DEV_INFO_LEN);
		//manager_event_indicate(EV_EVENT_OK, NULL);
	}
}

void hlp_manager_set_local_device_pin(UINT8 *p_pin)
{
	if (p_pin != NULL) {
		memset(MANAGER_USER_DATA->local_info->pin, 0, PIN_CODE_LEN+1);
		memcpy(MANAGER_USER_DATA->local_info->pin
		       , p_pin
		       , strlen(p_pin) > PIN_CODE_LEN ? PIN_CODE_LEN : strlen(p_pin));

		hlp_manager_save_configure(0, DEV_INFO_LEN);
		//hlp_gap_event_cbk(EV_APP_GAP_OK, NULL);
		//manager_event_indicate(EV_EVENT_OK, NULL);
	}
}

void hlp_manager_query_local_device_name(void)
{
	// FIXME
	if (!manager_user_data) return;

	LOG("MANAGER_USER_DATA->local_info->name = %s", MANAGER_USER_DATA->local_info->name);
	LOG("MANAGER_USER_DATA->local_info->name2 = %s", hlp_manager_get_local_name());
	manager_event_indicate(EV_OTHER_LOCAL_DEVICE_NAME, MANAGER_USER_DATA->local_info->name);
	manager_event_indicate(EV_EVENT_OK, NULL);
}

void hlp_manager_query_local_device_pin(void)
{
	// FIXME
	if (!manager_user_data) return;
	manager_event_indicate(EV_OTHER_LOCAL_PINCODE, MANAGER_USER_DATA->local_info->pin);
	manager_event_indicate(EV_EVENT_OK, NULL);
}

void hlp_manager_query_local_device_bd_addr(void)
{
	UINT8 bd_addr[18];
	// FIXME
	if (!manager_user_data) return;
        LOG("manager_user_data is true");
#ifdef CONFIG_DEFINE_BT_EVENT
    snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
         ,MANAGER_USER_DATA->local_info->bd_addr[5]
         ,MANAGER_USER_DATA->local_info->bd_addr[4]
         ,MANAGER_USER_DATA->local_info->bd_addr[3]
         ,MANAGER_USER_DATA->local_info->bd_addr[2]
         ,MANAGER_USER_DATA->local_info->bd_addr[1]
         ,MANAGER_USER_DATA->local_info->bd_addr[0]);
#else
	snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
	         ,MANAGER_USER_DATA->local_info->bd_addr[5]
	         ,MANAGER_USER_DATA->local_info->bd_addr[4]
	         ,MANAGER_USER_DATA->local_info->bd_addr[3]
	         ,MANAGER_USER_DATA->local_info->bd_addr[2]
	         ,MANAGER_USER_DATA->local_info->bd_addr[1]
	         ,MANAGER_USER_DATA->local_info->bd_addr[0]);
#endif

	manager_event_indicate(EV_OTHER_LOCAL_BD_IND, bd_addr);
	manager_event_indicate(EV_EVENT_OK, NULL);
}

void hlp_manager_query_local_device_le_bd_addr(void)
{
	UINT8 bd_addr[18];
#if 0
	// FIXME
	LOG("le manager_user_data is null?");
	if (!manager_user_data) return;
        LOG("manager_user_data is true");
#ifdef CONFIG_DEFINE_BT_EVENT
    snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
         ,MANAGER_USER_DATA->local_info->bd_addr[5]
         ,MANAGER_USER_DATA->local_info->bd_addr[4]
         ,MANAGER_USER_DATA->local_info->bd_addr[3]
         ,MANAGER_USER_DATA->local_info->bd_addr[2]
         ,MANAGER_USER_DATA->local_info->bd_addr[1]
         ,MANAGER_USER_DATA->local_info->bd_addr[0]);
#else
	snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
	         ,MANAGER_USER_DATA->local_info->bd_addr[5]
	         ,MANAGER_USER_DATA->local_info->bd_addr[4]
	         ,MANAGER_USER_DATA->local_info->bd_addr[3]
	         ,MANAGER_USER_DATA->local_info->bd_addr[2]
	         ,MANAGER_USER_DATA->local_info->bd_addr[1]
	         ,MANAGER_USER_DATA->local_info->bd_addr[0]);
#endif

#endif
	struct HCI_InfoRead_LocalStru *loc_info_a = HCI_InfoRead_Local(0);
	if(loc_info_a != NULL) {
		snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
	         ,loc_info_a->addr.bd[5]
	         ,loc_info_a->addr.bd[4]
	         ,loc_info_a->addr.bd[3]
	         ,loc_info_a->addr.bd[2]
	         ,loc_info_a->addr.bd[1]
	         ,loc_info_a->addr.bd[0]);

		FREE(loc_info_a);
	}

	manager_event_indicate(EV_OTHER_LOCAL_LE_BD_IND, bd_addr);
	manager_event_indicate(EV_EVENT_OK, NULL);
}



void hlp_manager_get_local_device_bd_addr(char *p_addr)
{
	if(!p_addr)
		return ;
	UINT8 bd_addr[18];
	// FIXME
	LOG("manager_user_data is null?");
	if (!manager_user_data) return;
        LOG("manager_user_data is true");
#ifdef CONFIG_DEFINE_BT_EVENT
    snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
         ,MANAGER_USER_DATA->local_info->bd_addr[5]
         ,MANAGER_USER_DATA->local_info->bd_addr[4]
         ,MANAGER_USER_DATA->local_info->bd_addr[3]
         ,MANAGER_USER_DATA->local_info->bd_addr[2]
         ,MANAGER_USER_DATA->local_info->bd_addr[1]
         ,MANAGER_USER_DATA->local_info->bd_addr[0]);
#else
	snprintf(bd_addr, sizeof(bd_addr), "%02X%02X%02X%02X%02X%02X"
	         ,MANAGER_USER_DATA->local_info->bd_addr[5]
	         ,MANAGER_USER_DATA->local_info->bd_addr[4]
	         ,MANAGER_USER_DATA->local_info->bd_addr[3]
	         ,MANAGER_USER_DATA->local_info->bd_addr[2]
	         ,MANAGER_USER_DATA->local_info->bd_addr[1]
	         ,MANAGER_USER_DATA->local_info->bd_addr[0]);
#endif

	//manager_event_indicate(EV_OTHER_LOCAL_BD_IND, bd_addr);
	memcpy(p_addr, bd_addr, 18);
	LOG("p_addr = %s", p_addr);
	return ;
}

void hlp_manager_query_local_configure(void)
{
	UINT8 configure[8];

	// FIXME
	if (!manager_user_data) return;

	snprintf(configure, sizeof(configure), "%d%d"
	         ,(MANAGER_USER_DATA->local_info->flag & LOCAL_DEV_SET_AUTO_ANSWER) == LOCAL_DEV_SET_AUTO_ANSWER
	         ,(MANAGER_USER_DATA->local_info->flag & LOCAL_DEV_SET_AUTO_CONNECT) == LOCAL_DEV_SET_AUTO_CONNECT);

	manager_event_indicate(EV_OTHER_AUTO_ANSWER_CONNECT_IND, configure);
}

void hlp_manager_enter_dut_mode(void)
{
	struct HCI_Set_Event_FilterStru * evt_filter;
	struct HCI_Set_Event_FilterInStru in;
	struct HCI_Set_Event_MaskStru mask;

	LOG("Enter %s.", __FUNCTION__);

    /* set auto accept connection as this is needed during test mode */
    /* Allocate a buffer to hold HCI command */
	in.filter_type = HCI_FILTER_TYPE_CONNECTION_SETUP;
	in.filter_condition_type = HCI_FILTER_CONDITION_TYPE_ALL_DEVICES;
	in.auto_accept_flag = HCI_AUTO_ACCEPT_FLAG_ENABLED;
	evt_filter = HCI_Set_Event_Filter(&in);

	if (evt_filter) {
		hlp_gap_excute_command(HCI_OPS_SET_EVENT_FILTER, NULL, evt_filter, sizeof(struct HCI_Set_Event_FilterStru) + 1);
		FREE(evt_filter);
	}

    /* put device to connectable mode */
    /* put device to discoverable mode */
	hlp_gap_set_scan_mode(HCI_SCAN_ENABLE_INQUIRY_SCAN_ENABLED_PAGE_SCAN_ENABLED);

    /* mask off all of event from controller */
    /*hci_layer_get_interface()->transmit_command(
      hci_packet_factory_get_interface()->make_set_event_mask((const bt_event_mask_t *)("\x00\x00\x00\x00\x00\x00\x00\x00")),
      NULL,
      NULL,
      NULL);*/
	memset(&mask, 0, sizeof(struct HCI_Set_Event_MaskStru));
	hlp_gap_excute_command(HCI_OPS_SET_EVENT_MASK, NULL, &mask, sizeof(struct HCI_Set_Event_MaskStru));

    /* Send the HCI command */
	hlp_gap_excute_command(HCI_OPS_ENABLE_DEVICE_UNDER_TEST_MODE, NULL, NULL, 0);
	manager_event_indicate(EV_OTHER_DUT_MODE_CONFIGURE, (void *) 1);
}

void hlp_manager_leave_dut_mode(void)
{
	hlp_gap_excute_command(HCI_OPS_RESET, NULL, NULL, 0);
	manager_event_indicate(EV_OTHER_DUT_MODE_CONFIGURE, (void *) 0);
}

profile_t g_mgr_profile = {
	hlp_manager_init,
	hlp_manager_done,
	hlp_manager_server_start,
	hlp_manager_server_stop,
};

void hlp_get_sdk_profile()
{
	//manager_event_indicate(EV_GEY_SDK_PROFILE,(void *) 0);
}

bt_remote_device_t *hlp_manager_find_target_device(UINT8*bd)
{
    if (!bd) {
        return NULL;
    }

    struct BtList *list = MANAGER_USER_DATA->dev_list;
    bt_remote_device_t *pDev = (bt_remote_device_t *)(list->head);

    while (pDev != NULL) {
        if (memcmp(pDev->bd, bd, 6) == 0) {
            //UART_Printf("       find target success, name = %s\r\n", pDev->name);
            return pDev;
        }
        pDev = (bt_remote_device_t *)NEXT(pDev);
    }
    return NULL;
}
