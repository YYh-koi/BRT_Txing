/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	hid_ui.h
Abstract:
Author:   
	Song Xiaowei
-----------------------------------------------------------------------------*/

#ifndef HID_UI_H
#define HID_UI_H

#include "hci_ui.h"

#ifdef CONFIG_BT_DEBUG
#define HID_DEBUG
#endif

#define HID_CONTROL_PSM		0x0011
#define HID_INTERRUPT_PSM	0x0013

/* HID Remote Role */
enum {
	HID_REMOTE_ROLE_HOST,
	HID_REMOTE_ROLE_DEVICE,
};

/* HID L2CAP Channel Type */
enum {
	HID_CHANNEL_CONTROL,
	HID_CHANNEL_INTERRUPT,
};

/* HID Event */
#define HID_EV_CABLE_UNPLUG				0x01
#define HID_EV_SET_SUSPEND				0x02
#define HID_EV_HANDSHAKE				0x03
#define HID_EV_GET_REPORT				0x04
#define HID_EV_SET_REPORT				0x05
#define HID_EV_GET_PROTOCOL				0x06
#define HID_EV_SET_PROTOCOL				0x07
#define HID_EV_DATA						0x0A

#define HID_EV_HIDINFO					0x0C
#define HID_EV_PNPINFO					0x0D
#define HID_EV_CONNECT					0x0E
#define HID_EV_DISCONNECT				0x0F

#define HID_EV_SET_SVCLIST_LE			0x10
#define HID_EV_GET_SVCLIST_LE			0x11

/* HID result code for request operation */
#define HID_FAILED						0xFFFF	/* Not defined in spec */
#define HID_SUCCESSFUL					0x00
#define HID_ERR_NOT_READY				0x01
#define HID_ERR_INVALID_REPORT_ID		0x02
#define HID_ERR_UNSUPPORTED_REQUEST		0x03
#define HID_ERR_INVALID_PARAMETER		0x04
#define HID_ERR_UNKNOWN					0x0E
#define HID_ERR_FATAL					0x0F
#define HID_ERR_TIMEOUT					0x10	/* Not defined in spec */

/* HID suspend flag */
#define HID_SUSPEND						0x03
#define HID_EXIT_SUSPEND				0x04

/* HID Report Type */
#define HID_OTHER_REPORT				0x0
#define HID_INPUT_REPORT				0x1
#define HID_OUTPUT_REPORT				0x2
#define HID_FEATURE_REPORT				0x3

/* HID protocol */
#define HID_BOOT_MODE					0x0
#define HID_REPORT_MODE					0x1

/* HID Boot Mode report ID */
#define HID_BOOT_REPORT_ID_KEYBOARD 	0x01
#define HID_BOOT_REPORT_ID_MOUSE 		0x02

/* HID sub-class definition */
#define HID_SUBCLASS_KEYBOARD			0x40    
#define HID_SUBCLASS_POINT				0x80
#define HID_SUBCLASS_KEYORPOINT			0xC0
#define HID_SUBCLASS_UNCLASSIFIED		0x00 
#define HID_SUBCLASS_JOYSTICK			0x04
#define HID_SUBCLASS_GAMEPAD			0x08
#define HID_SUBCLASS_REMCONTROL			0x0C
#define HID_SUBCLASS_SENSE				0x10

/* HID connection event processing callback function type */
typedef void (HID_CallBack)(UINT8 event, HANDLE conn_hdl, void *param);

struct HID_ConnectStru {
	UINT16 result;
	struct HCI_AddressStru addr;
	UINT8 conn_cap;
};

struct HID_GetReportStru {
	UINT8	report_type;
	UINT8	report_id;		/* ReportID of the requested report. It shall be set to 0 if the ReportID is not required. */
	UINT16	buf_size;		/* The required BufferSize in the GET_REPORT payload. It shall be set to 0 if the ReportSize if not required. */
};

struct HID_DataStru {
	UINT8	report_type;
	UINT16	data_len;
	UINT8	data[1];		/* Report data for the device. */
};

struct HID_ResponseStru {
	UINT16 result;
	struct HID_DataStru *data_buf;
};

/*----------------------------------------------------------------------*/
/*						HID UI interface function						*/
/*----------------------------------------------------------------------*/
void HID_Init();
void HID_Done();
// void HID_RegCbk(void *cbk);

#ifdef CONFIG_HID_CLIENT
void HID_Host_Start(void *in);
void HID_Host_Stop();

void HID_Host_ConnectReq(struct HID_ConnectStru *in);
void HID_Host_DisconnectReq(HANDLE conn_hdl);
void HID_Host_CablePlug(struct HCI_AddressStru *addr, struct SDAP_HIDInfoStru *attr, struct SDAP_DIInfoStru *pnp_info);
void HID_Host_CableUnplugReq(struct HCI_AddressStru *addr);

void HID_Host_SetSuspendReq(HANDLE conn_hdl, UINT8 suspend_flag);
void HID_Host_DataReq(HANDLE conn_hdl, struct HID_DataStru *in);
void HID_Host_GetReportReq(HANDLE conn_hdl, struct HID_GetReportStru *in);
void HID_Host_SetReportReq(HANDLE conn_hdl, struct HID_DataStru *in);
void HID_Host_GetProtocolReq(HANDLE conn_hdl);
void HID_Host_SetProtocolReq(HANDLE conn_hdl, UINT8 prot);
void HID_Host_GetSdapInfoReq_LE(HANDLE conn_hdl);
void HID_Host_GetPnpInfoReq_LE(HANDLE conn_hdl);
void HID_Host_SetSvcListReq_LE(struct HCI_AddressStru *addr, struct ListStru *svc_list);
void HID_Host_GetSvcListReq_LE(struct HCI_AddressStru *addr);

void *HID_Host_Connect(struct HID_ConnectStru *in);
UINT16 HID_Host_CableUnplug(struct HCI_AddressStru *addr);
UINT16 HID_Host_GetReport(HANDLE conn_hdl, struct HID_GetReportStru *in, struct HID_DataStru **out);
UINT16 HID_Host_SetReport(HANDLE conn_hdl, struct HID_DataStru *in);
UINT16 HID_Host_GetProtocol(HANDLE conn_hdl, UINT8 *prot);
UINT16 HID_Host_SetProtocol(HANDLE conn_hdl, UINT8 prot);
void *HID_Host_GetSdapInfo_LE(HANDLE conn_hdl);
void *HID_Host_GetPnpInfo_LE(HANDLE conn_hdl);
void *HID_Host_GetSvcList_LE(struct HCI_AddressStru *addr);

UINT8 HID_Host_GetPlugFlag(struct HCI_AddressStru *addr);
UINT8 HID_Host_GetSubclass(struct HCI_AddressStru *addr);
#endif /* CONFIG_HID_HOST */

#ifdef CONFIG_HID_SERVER
void HID_Device_Start(void *cbk);
void HID_Device_Stop();

void HID_Device_ConnectReq(struct HCI_AddressStru *addr);
void HID_Device_DisconnectReq(HANDLE conn_hdl);
void HID_Device_CablePlug(struct HID_ConnectStru *in);
void HID_Device_CableUnplugReq(struct HCI_AddressStru *addr);
void HID_Device_MessageRsp(HANDLE conn_hdl, UINT8 result, struct HID_DataStru *in);
void HID_Device_DataReq(HANDLE conn_hdl, struct HID_DataStru *in);

void *HID_Device_Connect(struct HCI_AddressStru *addr);
void HID_Device_Disconnect(HANDLE conn_hdl);
UINT16 HID_Device_CableUnplug(struct HCI_AddressStru *addr);
#endif /* CONFIG_HID_SERVER */

#ifdef CONFIG_ATT
void HIDH_LE_MergeServices(struct ListStru *target_list, struct ListStru *svc_list);
#else
#define HIDH_LE_MergeServices(a, b)
#endif

#endif /* HID_UI_H */

