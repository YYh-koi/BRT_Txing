/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Module Name:
	hid_le.h
Abstract:
Author:   
	Song Xiaowei
---------------------------------------------------------------------------*/
#ifndef HID_GATT_H
#define HID_GATT_H

#include "global.h"
#include "hci_ui.h"
#include "l2cap_ui.h"
#include "sdap_ui.h"
#include "gap_ui.h"
#include "hid_ui.h"

#include "att_ui.h"
#include "hci_le.h"

#ifdef CONFIG_ATT
/* HID Infomation Characteristic flags Value. */
#define HID_LE_REMOTE_WAKE_FLAG							GATT_HID_INFORMATION_REMOTEWAKE_THE_DEVICE_IS_DESIGNED_TO_BE_CAPABLE_OF_PROVIDING_WAKE_UP_SIGNAL_TO_A_HID_HOST
#define HID_LE_NORMAL_CONNECTABLE_FLAG					GATT_HID_INFORMATION_NORMALLYCONNECTABLE_THE_DEVICE_IS_NORMALLY_CONNECTABLE

#define HID_LE_SUSPEND									GATT_HID_CONTROL_POINT_HID_CONTROL_POINT_COMMAND_SUSPEND	/* Informs HID Device that HID Host is entering the Suspend State. */
#define HID_LE_EXIT_SUSPEND								GATT_HID_CONTROL_POINT_HID_CONTROL_POINT_COMMAND_EXIT_SUSPEND	/* Informs HID Device that HID Host is exiting the Suspend State. */

#define HID_LE_CCC_NONE									0x00
#define HID_LE_CCC_NOTIFICATION							0x01
#define HID_LE_CCC_INDICATION							0x02

/* Timer out millisec */
#define HIDH_LE_DEV_DIR_ADV_TIMEOUT						1280

/* HID Event */
typedef enum {
	TASK_ERR,
		
	HOST_CACHE_FRAME = 0x01,
	HOST_GET_HIDINFO,
	HOST_GET_PNPINFO,
	HOST_ENABLE_NOTIF,
	HOST_DISABLE_NOTIF,
	HOST_SET_SUSPEND,
	HOST_SET_REPORT,
	HOST_GET_REPORT,
	HOST_SET_PROTOCOL,
	HOST_GET_PROTOCOL,
	HOST_DATA,
	HOST_DISCONNECT,
} HIDH_LE_EventIDEnum;

typedef enum {
	SERVICE_LIST = 0,
	SERVICE,
	INCLUDE,
	CHARACTER,/* Declaration handle or value handle match */
	DESCRIPTOR
} HIDH_LE_ItemTypeEnum;

struct HID_GATT_InfomationCharacteristicStru {
	UINT16	bcd;			/* 16-bit unsigned integer representing version number of base USB HID Specification implemented by HID Device. */
	UINT8	country_code;	/* 8-bit integer identifying country HID Device hardware is localized for. Most hardware is not localized (value 0x00) */
	UINT8	flags;			/* Bit 0: RemoteWake - Boolean value indicating whether HID Device is capable of sending a wake-signal to a HID Host.
									0bXXXXXXX0 = HID_INFOMATION_CHARATERISTIC_FLAG_REMOTE_WAKE_FALSE
									0bXXXXXXX1 = HID_INFOMATION_CHARATERISTIC_FLAG_REMOTE_WAKE_TRUE 
							   Bit 1: NormallyConnectable - Boolean value indicating whether HID Device will be advertising when bonded but not connected.
									0bXXXXXX0X = HID_INFOMATION_CHARATERISTIC_FLAG_NORMALLY_CONNECTABLE_FALSE
									0xXXXXXX1X = HID_INFOMATION_CHARATERISTIC_FLAG_NORMALLY_CONNECTABLE_TRUE */
};

struct HIDH_LE_HostTaskStru {
	struct HIDH_LE_EventStru *cur_event;
	UINT8 *task_index;
	void *cur_item;
	HIDH_LE_ItemTypeEnum item_type;
	UINT8 task_id;
};

struct HIDH_LE_EventStru {
	HANDLE conn_hdl;
	struct ListStru task_list;
	HIDH_LE_EventIDEnum event_id;
};

struct HIDH_LE_HostConnInfoStru {
	struct ListStru service_list;
	struct HIDH_LE_EventStru *cur_event;
};

struct HIDH_LE_SvcListStru {
	struct ListStru service_list;
	UINT8 rep_id_num;
};

struct HIDH_LE_RepSizeStru {
	UINT16 input_size;
	UINT16 output_size;
	UINT16 feature_size;
	UINT8 rep_id;
};

struct HIDH_LE_DescStru {
	UINT8 rep_id_num;
	struct HIDH_LE_RepSizeStru rep_size[1];
};

#include "hid_hlp.h"
void HID_LE_RegChannel();
void HID_LE_UnregChannel();

#ifdef CONFIG_HID_CLIENT
UINT8 HID_LE_HostSdapInfoReq(struct HIDH_ConnInfoStru *session);
void *HID_LE_HostSdapInfoCfm(struct HIDH_LE_HostConnInfoStru *conn_info);
UINT8 HID_LE_HostPnpInfoReq(struct HIDH_ConnInfoStru *session);
void *HID_LE_HostPnpInfoCfm(struct HIDH_LE_HostConnInfoStru *conn_info);
void HID_LE_HostGetPnpInfoReq(struct HIDH_ConnInfoStru *session);
void *HID_LE_HostGetPnpInfo(struct HIDH_LE_HostConnInfoStru *conn_info);
void HID_LE_Unplug(HANDLE conn_hdl, struct HIDH_LE_HostConnInfoStru *conn_info);
void HID_LE_HostConnReq(struct HIDH_ConnInfoStru *session);
UINT8 HID_LE_HostConnComplete(struct HIDH_LE_HostConnInfoStru *conn_info, HIDH_LE_EventIDEnum *event_id, UINT16 *res);
void HID_LE_HostDisconnReq(HANDLE conn_hdl, struct HIDH_LE_HostConnInfoStru *conn_info);
UINT8 HID_LE_HostDisconnComplete(struct HIDH_ConnInfoStru *session, UINT8 *event_id);
UINT8 HID_LE_MsgSend(HANDLE conn_hdl, struct HIDH_LE_HostConnInfoStru *conn_info, struct HIDH_MsgCommonStru *msg);
void *HID_LE_MsgDecode(struct HIDH_LE_HostConnInfoStru *conn_info, struct HIDH_MsgCommonStru *msg, HIDH_LE_EventIDEnum *event_id, UINT8 *msg_type);
void *HID_LE_DataIndDecode(struct HIDH_LE_HostConnInfoStru *conn_info, struct ATT_HandleValueCbkStru *in);
void HID_LE_HostConnInfoFree(struct HIDH_LE_HostConnInfoStru *conn_info);
void HID_LE_HostSetSvcList(struct HIDH_ConnInfoStru *session, struct ListStru *svc_list);
void *HID_LE_HostGetSvcList(struct HIDH_ConnInfoStru *session);
#endif /*CONFIG_HID_CLIENT */

#ifdef CONFIG_HID_SERVER
void HID_LE_DeviceRegHidSvc(struct SDAP_HIDInfoStru *hid_info);
void HID_LE_DeviceConnInd(struct HIDH_ConnInfoStru *session, struct ATT_PendIndStru *in);
UINT8 HID_LE_DeviceConnComplete(struct HIDH_ConnInfoStru *session, UINT8 *next_state, UINT16 *res);
void HID_LE_DeviceMsgReq(struct HIDH_MsgCommonStru *msg);
#endif /* CONFIG_HID_SERVER */

#else /* CONFIG_ATT */

#define HID_LE_RegChannel()
#define HID_LE_UnregChannel();
#ifdef CONFIG_HID_CLIENT
#define HID_LE_HostSdapInfoReq(a)			0
#define HID_LE_HostSdapInfoCfm(a)			NULL
#define HID_LE_HostGetPnpInfoReq(a)
#define HID_LE_HostGetPnpInfo(a)			NULL
#define HID_LE_Unplug(a, b)					NULL
#define HID_LE_HostConnReq(a)
#define HID_LE_HostConnComplete(a, b, c)	0
#define HID_LE_HostDisconnReq(a, b)
#define HID_LE_HostDisconnComplete(a)		0
#define HID_LE_MsgSend(a, b, c)				HID_ERR_UNSUPPORTED_REQUEST
#define HID_LE_MsgDecode(a, b, c, d)		NULL
#define HID_LE_DataIndDecode(a, b)			NULL
#define HID_LE_HostConnInfoFree(a)
#define HID_LE_HostSetSvcList(a, b)
#define HID_LE_HostGetSvcList(a)			NULL
#endif /*CONFIG_HID_CLIENT */

#ifdef CONFIG_HID_SERVER
#define HID_LE_DeviceRegHidSvc(a)
#define HID_LE_DeviceConnInd(a, b)
#define HID_LE_DeviceConnComplete(a, b, c)	0
#define HID_LE_DeviceMsgReq(a)
#endif /* CONFIG_HID_SERVER */

#define HIDH_LE_DescriptorListCopy(a, b)
#define HIDH_LE_CharacterListCopy(a, b)
#define HIDH_LE_ServiceListCopy(a, b)

#endif /* CONFIG_ATT */
#endif /* HID_GATT_H */
