/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
Module Name:
	hid_hlp.h
Abstract:
Author:   
	Song Xiaowei
---------------------------------------------------------------------------*/

#ifndef HID_HLP_H
#define HID_HLP_H

#define _HID_BREDR

#include "global.h"
#include "hci_ui.h"
#include "l2cap_ui.h"
#include "sdap_ui.h"
#include "gap_ui.h"
#include "hid_ui.h"

#ifdef CONFIG_OS_WIN32
#include "btwin32/hid_inter_ui.h"
#endif /* CONFIG_OS_WIN32 */

extern struct FsmInst *hid_fsminst;

#define HIDH_DECODE2BYTE_LITTLE						DECODE2BYTE_LITTLE
#define HIDH_ENCODE2BYTE_LITTLE						ENCODE2BYTE_LITTLE
#define HIDH_DECODE2BYTE_LITTLE_BYLEN(ptr, len)		(UINT16)((len > 1) ? DECODE2BYTE_LITTLE(ptr) : (UINT16)*(ptr))

/* Events of control module */
enum {
	EV_HID_HOST_START,
	EV_HID_HOST_STOP,
	EV_HID_HOST_CONN_REQ,
	EV_HID_HOST_CONN_IND,
	EV_HID_HOST_CONN_COMPLETE,
	EV_HID_HOST_DISCONN,
	EV_HID_HOST_DISCONN_COMPLETE,
	EV_HID_HOST_CABLE_PLUG,
	EV_HID_HOST_CABLE_UNPLUG,
	EV_HID_HOST_MESSAGE_SEND,
	EV_HID_HOST_MESSAGE_CFM,
	EV_HID_HOST_MESSAGE_IND,
	EV_HID_HOST_DATA_SEND,
	EV_HID_HOST_DATA_IND,
	EV_HID_HOST_SDAPINFOREQ_LE,
	EV_HID_HOST_SDAPINFOCFM_LE,
	EV_HID_HOST_PNPINFOREQ_LE,
	EV_HID_HOST_PNPINFOCFM_LE,
	EV_HID_HOST_SETSVCLIST_LE,
	EV_HID_HOST_GETSVCLIST_LE,

	EV_HID_DEVICE_START,
	EV_HID_DEVICE_STOP,
	EV_HID_DEVICE_CONN_REQ,
	EV_HID_DEVICE_CONN_IND,
	EV_HID_DEVICE_CONN_COMPLETE,
	EV_HID_DEVICE_DISCONN_REQ,
	EV_HID_DEVICE_DISCONN_COMPLETE,
	EV_HID_DEVICE_CABLE_PLUG,
	EV_HID_DEVICE_CABLE_UNPLUG,
	EV_HID_DEVICE_MESSAGE_IND,
	EV_HID_DEVICE_DATA_REQ,
	EV_HID_DEVICE_DATA_IND,
	EV_HID_DEVICE_REGHIDSVC_LE,
	
	EV_HID_TIME_OUT,
	EV_HID_DONE
};

/* HID Transaction Type */
#define HIDH_HANDSHAKE					0x0
#define HIDH_CONTROL					0x1
#define HIDH_GET_REPORT					0x4
#define HIDH_SET_REPORT					0x5
#define HIDH_GET_PROTOCOL				0x6
#define HIDH_SET_PROTOCOL				0x7
#define HIDH_GET_IDLE					0x8
#define HIDH_SET_IDLE					0x9
#define HIDH_DATA						0xA
#define HIDH_DATC						0xB

/* GET_REPORT size */
#define	HIDH_NO_SIZE					0x0
#define	HIDH_SPECIFIED_SIZE				0x1

/* Plugged Mask */
#define HIDH_MASK_PLUGGED				0x01
#define HIDH_MASK_SUPPORT_VIRTUAL_CABLE	0x02

/* Control operation code */
#define HIDH_NOP						0x0
#define HIDH_HARD_RESET					0x1
#define HIDH_SOFT_RESET					0x2
#define HIDH_SUSPEND					0x3
#define HIDH_EXIT_SUSPEND				0x4
#define HIDH_VIRTUAL_CABLE_UNPLUG		0x5

#define HIDH_SIDE_INIT					0x0
#define HIDH_SIDE_RSP					0x1

/* Timer out millisec */
#define HIDH_REQ_CONN_TIMEOUT			45000
#define HIDH_REQ_TIMEOUT 				500
#define HIDH_DISC_TIMEOUT				1000
#define HIDH_UNPLUG_TIMEOUT				10000

/* Timer ID */
#define HIDH_NO_TIMER					0x01
#define HIDH_HOST_CONN_TIMER			0x02
#define HIDH_HOST_DISC_TIMER			0x03
#define HIDH_HOST_UNPLUG_TIMER			0x04
#define HIDH_HOST_GET_REP_TIMER			0x05
#define HIDH_HOST_SET_REP_TIMER 		0x06
#define HIDH_HOST_GET_PRO_TIMER			0x07
#define HIDH_HOST_SET_PRO_TIMER			0x08

#define HIDH_DEV_CONN_TIMER				0x0A
#define HIDH_DEV_DISC_TIMER				0x0B
#define HIDH_DEV_UNPLUG_TIMER			0x0C

/* Request handle */
#define HIDH_REQ_HDL_BASE				(PROT_HID_BASE + 0x40)
#define HIDH_HOST_CONN_HDL				(HIDH_REQ_HDL_BASE + 0x1)
#define HIDH_HOST_DISC_HDL				(HIDH_REQ_HDL_BASE + 0x2)
#define HIDH_HOST_UNPLUG_HDL			(HIDH_REQ_HDL_BASE + 0x3)
#define HIDH_HOST_GET_REP_HDL 			(HIDH_REQ_HDL_BASE + 0x4)
#define HIDH_HOST_SET_REP_HDL			(HIDH_REQ_HDL_BASE + 0x5)
#define HIDH_HOST_GET_PRO_HDL			(HIDH_REQ_HDL_BASE + 0x6)
#define HIDH_HOST_SET_PRO_HDL			(HIDH_REQ_HDL_BASE + 0x7)
#define HIDH_HOST_SDAP_INFO_LE_HDL		(HIDH_REQ_HDL_BASE + 0x8)
#define HIDH_HOST_PNP_INFO_LE_HDL		(HIDH_REQ_HDL_BASE + 0x9)
#define HIDH_HOST_SET_SVCLIST_LE_HDL	(HIDH_REQ_HDL_BASE + 0xA)
#define HIDH_HOST_GET_SVCLIST_LE_HDL	(HIDH_REQ_HDL_BASE + 0xB)

#define HIDH_DEV_CONN_HDL				(HIDH_REQ_HDL_BASE + 0xC)
#define HIDH_DEV_DISC_HDL				(HIDH_REQ_HDL_BASE + 0xD)
#define HIDH_DEV_UNPLUG_HDL				(HIDH_REQ_HDL_BASE + 0xE)

/* State of session fsminst */
enum {
	ST_HID_IDLE = 0,
	ST_HID_CONNECTING,
	ST_HID_HALFCONN,
	ST_HID_CONNECTED,
	ST_HID_WAITING,
	ST_HID_DISCONN
};

struct HIDH_MsgCommonStru {
	HANDLE hdl;
	struct FsmTimer *timer;
	void *arg;
	UINT8 msg_type;
};

#if (defined(CONFIG_ARMTCC) || defined(CONFIG_ARM) || defined(CONFIG_DIABDCC))
#pragma anon_unions
#endif
struct HIDH_ConnInfoStru {
	union {
		HANDLE conn_hdl;
		struct HCI_AddressStru *addr;
	} u;
	void *conn_info; /* conn_cap == HCI_ADDRESS_CAP_DREDR:		struct HIDH_BREDR_ConnInfoStru */
					 /* conn_cap == HCI_ADDRESS_CAP_LE
					 								 Host:		struct HIDH_LE_HostConnInfoStru */
					 /* 						   Device:		struct FsmTimerStru */
	struct HIDH_MsgCommonStru *req_pending;
	UINT8 conn_cap;
	UINT8 state;
	UINT8 subclass;
	UINT8 plug_mask;
};

struct HIDH_FsmUserDataStru {
	SYSTEM_LOCK lock;
#ifdef CONFIG_HID_CLIENT
	struct ListStru host_conn_list; /* List of remote device. (struct HIDH_ConnInfoStru *) */
	HID_CallBack *host_cbk;
#endif /* CONFIG_HID_CLIENT */
#ifdef CONFIG_HID_SERVER
	struct HIDH_ConnInfoStru *dev_conn_info; /* Device connect information. */
	HID_CallBack *dev_cbk;
#ifdef CONFIG_ATT
	struct ListStru dev_le_svc_list;
	UINT8 rep_id_num;
#endif /* CONFIG_ATT */
#endif /* CONFIG_HID_SERVER */
};

struct HIDH_ConnCbkParamStru {
	HANDLE hdl;
	void *param;
	UINT8 conn_cap;
};

struct HIDH_CablePlugStru
{
	struct HCI_AddressStru addr;
	struct SDAP_HIDInfoStru *attr;
	struct SDAP_DIInfoStru *pnp_info;
};

struct HIDH_CableUnplugStru
{
	struct HCI_AddressStru addr;
	UINT8 side;
};

#define HID_CUDATA ((struct HIDH_FsmUserDataStru *)hid_fsminst->user_data)
#define HID_LOCK (((struct HIDH_FsmUserDataStru *)hid_fsminst->user_data)->lock)

#ifdef CONFIG_HID_CLIENT
#define HID_HOST_CONNLIST (HID_CUDATA->host_conn_list)
#endif /* CONFIG_HID_CLIENT */

#ifdef CONFIG_HID_SERVER
#define HID_DEV_CONNINFO (HID_CUDATA->dev_conn_info)
#ifdef CONFIG_ATT
#define HID_DEV_LESVCLIST (HID_CUDATA->dev_le_svc_list)
#endif /* CONFIG_ATT */
#endif /* CONFIG_HID_SERVER */

/*-------------------------------------------------------------------*/
/*				   			 Help Function						     */
/*-------------------------------------------------------------------*/
void *HIDH_FindConnByAddr(struct HCI_AddressStru *addr, UINT8 *remote_role);
struct HIDH_MsgCommonStru *HIDH_CommonMsgNew(HANDLE hdl, UINT8 msg_type, void *arg);
void HIDH_CommonMsgFree(struct HIDH_MsgCommonStru *msg);
void HIDH_AddReqPending(struct HIDH_ConnInfoStru *session, struct HIDH_MsgCommonStru *msg, void *timeout_cbk);
void HIDH_RemoveReqPending(struct HIDH_ConnInfoStru *session, UINT8 cbk_flag);

#ifdef CONFIG_ATT
void HIDH_LE_DescriptorListCopy(struct ListStru *target_list, struct ListStru *list);
void HIDH_LE_CharacterListCopy(struct ListStru *target_list, struct ListStru *list);
void HIDH_LE_ServiceListCopy(struct ListStru *target_list, struct ListStru *list);
#endif

#ifdef CONFIG_HID_CLIENT
void *HIDH_Host_NewConn(struct HCI_AddressStru *addr);
void *HIDH_Host_FindConnByHdl(HANDLE conn_hdl);
void *HIDH_Host_FindConnByAddr(struct HCI_AddressStru *addr);
void HIDH_Host_FreeConn(struct HIDH_ConnInfoStru *session);
void HIDH_Host_ConnCbk(UINT8 event, HANDLE conn_hdl, struct HCI_AddressStru *addr, UINT8 conn_cap, UINT16 result);
void HIDH_Host_RspCbk(HANDLE conn_hdl, UINT8 event, struct HID_ResponseStru *param);
void HIDH_Host_DataIndCbk(HANDLE conn_hdl, void *param);
void HIDH_Host_SvcInfoRspCbk(HANDLE conn_hdl, UINT8 event, void *param);
#endif /* CONFIG_HID_HOST */

#ifdef CONFIG_HID_SERVER
void *HIDH_Device_NewConn(struct HCI_AddressStru *addr);
void HIDH_Device_FreeConn();
void HIDH_Device_ConnCbk(UINT8 event, HANDLE conn_hdl, struct HCI_AddressStru *addr, UINT8 conn_cap, UINT16 result);
void HIDH_Device_CmdCbk(HANDLE conn_hdl, UINT8 event, void *arg);
#endif /* CONFIG_HID_SERVER */

/*-------------------------------------------------------------------*/
/*				   			  FSM Control						     */
/*-------------------------------------------------------------------*/
void HIDC_Init();
void HIDC_Done(struct FsmInst *fi, UINT8 event, void *in);

#ifdef CONFIG_HID_CLIENT
void HIDC_Host_Start(struct FsmInst *fi, UINT8 event, void *cbk);
void HIDC_Host_Stop(struct FsmInst *fi, UINT8 event, void *in);
void HIDC_Host_TimeOut(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);

void HIDC_Host_ConnectReq(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_ConnectComplete(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_ConnectInd(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_DisconnReq(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_DisconnComplete(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_Plug(struct FsmInst *fi, UINT8 event, struct HIDH_CablePlugStru *in);
void HIDC_Host_Unplug(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_MsgSend(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_MsgInd(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_DataSend(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_DataInd(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_SdapInfoReq_LE(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_SdapInfoCfm_LE(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_PnpInfoReq_LE(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_PnpInfoCfm_LE(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Host_SetSvcList_LE(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Host_GetSvcList_LE(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
#endif /* CONFIG_HID_HOST */

#ifdef CONFIG_HID_SERVER
void HIDC_Device_Start(struct FsmInst *fi, UINT8 event, void *cbk);
void HIDC_Device_Stop(struct FsmInst *fi, UINT8 event, void *in);
void HIDC_Device_TimeOut(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);

void HIDC_Device_ConnectReq(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Device_ConnectInd(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Device_ConnectComplete(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Device_DisconnectReq(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Device_DisconnectComplete(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Device_Plug(struct FsmInst *fi, UINT8 event, struct HID_ConnectStru *in);
void HIDC_Device_Unplug(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Device_MsgInd(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Device_DataReq(struct FsmInst *fi, UINT8 event, struct HIDH_MsgCommonStru *msg);
void HIDC_Device_DataInd(struct FsmInst *fi, UINT8 event, struct HIDH_ConnCbkParamStru *in);
void HIDC_Device_RegHidSvc_LE(struct FsmInst *fi, UINT8 event, struct HIDH_CablePlugStru *in);
#endif /* CONFIG_HID_SERVER */

#ifdef HID_DEBUG
void HID_DBG_DataInd(struct HCI_AddressStru *addr, struct HID_DataStru *in);
void HID_DBG_MsgSend(struct HIDH_MsgCommonStru *msg);
void HID_DBG_MsgInd(UINT8 msg_type, void *in, struct HCI_AddressStru *addr);
void HID_DBG_Plug(struct HIDH_ConnInfoStru *session);
void HID_DBG_BREDR_ConnReq(struct L2CAP_ConnectStru *in);
void HID_DBG_BREDR_ConnInd(struct L2CAP_ConnectStru *in);
void HID_DBG_BREDR_ConnRsp(struct L2CAP_ConnectStru *in);
void HID_DBG_BREDR_ConnCfm(struct L2CAP_ConnectStru *in);
void HID_DBG_BREDR_ConnComplete(struct L2CAP_ConnectCompleteStru *in);
void HID_DBG_BREDR_DisconnReq(struct L2CAP_DisconnectReqStru *in);
void HID_DBG_BREDR_DisconnComplete(struct L2CAP_DisconnectCompleteStru *in);
void HID_DBG_LE_ServiceTree(struct ListStru *service_list);
void HID_DBG_LE_EventComplete(struct HIDH_LE_EventStru *event);
void HID_DBG_LE_EventCreate(struct HIDH_LE_EventStru *event);
void HID_DBG_LE_GattCbk(struct HIDH_LE_HostTaskStru *task, UINT8 ev, void *arg);
void HID_DBG_LE_TaskCreate(struct HIDH_LE_HostTaskStru *task);
void HID_DBG_LE_TaskComplete(struct HIDH_LE_HostTaskStru *task);
#endif /* HID_DEBUG */

#endif /* HID_HLP_H */
