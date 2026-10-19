/*---------------------------------------------------------------------------
Module Name:
	att_hlp.h
Abstract:
	This module defines the attribute protocol macros. ATT is in little endian.
Author:
	Wang Yao
---------------------------------------------------------------------------*/

#ifndef G_ATT_HLP_H
#define G_ATT_HLP_H

#include "hci_ui.h"
#include "l2cap_ui.h"
#include "att_ui.h"
#include "smp_ui.h"

/*---------------------------------------------------------------------------*/
/* 									ATT Macros							     */
/*---------------------------------------------------------------------------*/
extern struct FsmInst *att_fsminst;

#ifdef CONFIG_BT_DEBUG
#undef ATT_DBG//						1/* DBG flag */
#endif

#define ATT_BUFFER_RESERVE			(UINT16)(HCI_RESERVE + L2CAP_CO_RESERVE)
#define ATT_CUDATA					((struct ATT_CUDataStru *)(att_fsminst->user_data))
#define ATT_TLLIST					(ATT_CUDATA->tllist)
#define ATT_ITEMLIST				(ATT_CUDATA->att)
#define ATT_RANGELIST				(ATT_CUDATA->range)
#define ATT_REGSERVICE				(ATT_CUDATA->regservice)/* Temp list if included service has not registered yet */
#define ATT_NOTIFCBKLIST			(ATT_CUDATA->notifcbk)
#define ATT_PSM 					0x001F
#define ATT_GETBUF(pack_)			(struct BuffStru *)((pack_) - ATT_BUFFER_RESERVE - (sizeof(UINT32) << 1))/* From singal ptr to buf */

#define ATT_DECODE2BYTE 			DECODE2BYTE_LITTLE
#define ATT_ENCODE2BYTE				ENCODE2BYTE_LITTLE

enum {
	ATT_MTU_LE_DEFAULT =  			23,/* LE */
	ATT_MTU_BREDR_MIN =				48,
	ATT_MTU_LOCAL_RECEIVE =			517/* MAX Local Receive MTU */
};

/*---------------------------------------------------------------------------
3.2.5 Attribute Permissions
	An attribute has a set of permission values associated with it. The 
	permissions associated with an attribute specifies that it may be read 
	and/or written. The permissions associated with the attribute specifies 
	the security level required for read and/or write access, as well as 
	notification and/or indication. The permissions of a given attribute are 
	defined by a higher layer specification, and are not discoverable using 
	the attribute protocol.
---------------------------------------------------------------------------*/
#define ATT_ITEM_INTERNAL_DATAP(item_)		((item_)->u.data.value)
#define ATT_ITEM_INTERNAL_DATALEN(item_)	((item_)->u.data.len)

enum {/* mask in struct ATT_ItemTLStru */
	ATT_ITEM_NEED2UPDATE = 1
};

struct ATT_ItemTLStru {/* node in u->list of struct ATT_ItemStru */
	struct HCI_AddressStru addr;
	UINT8 mask;/* etc: ATT_ITEM_NEED2UPDATE */
	struct ATT_ValueStru data;
};

struct ATT_ItemStru {/* Logical Attribute Representation node(Element in Attribute database) */
	UINT16 hdl;
	UINT16 permission;/* mask, etc: ATT_READ, ATT_AUTHENTICATION_NEED */
	UINT8 type;/* Value type, etc: ATT_VALUE_INTERNAL */
	struct ATT_UUIDStru uuid;

	union {/* selected by ATT_CPMASK_EXTERNAL_CBK in permission */
		struct ATT_ValueStru data;/* Internal, default */
		struct ListStru list;/* TL Related, node is struct ATT_ItemTLStru */
	} u;
};

enum {/* type in struct ATT_TransStru */
	ATT_TRANSTYPE_OUTSTANDING = 	(0x80),								/* Bitmask, need wait for response */
	ATT_TRANSTYPE_REQ = 			(0x01 | ATT_TRANSTYPE_OUTSTANDING),	/* Request */
	ATT_TRANSTYPE_RSP = 			(0x02),								/* Response */
	ATT_TRANSTYPE_IND = 			(0x03 | ATT_TRANSTYPE_OUTSTANDING),	/* GATT_TASK_INDICATIONS */
	ATT_TRANSTYPE_NOTIF = 			(0x05),								/* GATT_TASK_NOTIFICATIONS */
	ATT_TRANSTYPE_CMD = 			(0x06)								/* GATT_TASK_WRITE_WITHOUT_RESPONSE */
};

struct ATT_TransStru {/* Transcation Context */
	UINT8 type;
	UINT8 op;/* Rsp type, waiting for this op */
	struct BuffStru *data;
};

enum {/* TL Status */
	ATT_TLST_IDLE = 				0x00,/* New created, include BREDR, ACLLink is ready, need to create l2cap channel */
	ATT_TLST_ONGOING = 				0x01,/* ATT channel/LELink request outstanding */
	ATT_TLST_READY = 				0x02/* ATT ready to use, LELink or L2CAP dynamic channel is ready */
};

enum {/* mask in struct ATT_TLStru */
	ATT_TLMASK_L2CAP = 				0x01,
	ATT_TLMASK_TASK_ONGOING = 		0x02,
	ATT_TLMASK_SMP_REQ = 			0x04,/* 1st task or received signed write requesting SMP */
	ATT_TLMASK_SMP_CFM = 			0x08,/* 1st task or received signed write got SMP result */
	ATT_TLMASK_REFLE = 				0x10/* LE Link Ref flag */
};

enum {/* Time out value */
	ATT_PDURTX_TIME = 				12000/* FIXME, consider LE interval issue, A transaction not completed within 30 seconds shall time out. */
};

enum {/* Time out event */
	ATT_EV_PDURTX = 				1,
	ATT_EV_TLIDLE = 				2
};

enum {/* index[x] of struct ATT_TLStru */
	ATT_TLIDX_PENDIND = 			0,
	ATT_TLIDX_TASK = 				1
};

struct ATT_TLStru {/* ATT Bearer (L2CAP Fixed CID/PSM) */
	struct FsmTimer *timer;				/* Transaction timer or TL Idle Timer */
	struct ATT_TransStru *trans;		/* Transcation, Request or Command? for client, Indication or Notification? for server */
	struct ListStru task;				/* struct GATT_TaskCommonStru, GATT task */
	struct ListStru qwrite;				/* struct ATT_HandleValueStru, Server use to record peer client instances */
	struct ListStru indlist;			/* etc: struct ATT_PendIndStru, pendding received PDU, waiting for upper deal with ATT database */
	struct HCI_AddressStru addr;		/* Remote addr */

	/* Transport Layer Bearer */
	void *hci;
	union {
		UINT16 cid;						/* 0x0000 is invalid */
		UINT16 hcihdl;					/* HCI_INVAILD_HANDLE, 0xFFFF is invalid */
	} u;

	UINT16 mtu;							/* ATT_MTU */
	UINT8 index[2];						/* etc, ATT_TLIDX_PENDIND, indlist node index, global */
	UINT8 status;						/* ATT_TLST_IDLE */
	UINT8 mask;							/* ATT_TLMASK_L2CAP */
};

struct ATT_CUDataStru {/* Root */
	struct ListStru tllist;/* struct ATT_TLStru, ATT Bearer L2CAP Link */
	struct ListStru att;/* struct ATT_ItemStru, Attribute records list Local as server */
	struct ListStru range;/* struct ATT_HandleRangeStru, ATT Handle ranges space allocation */
	struct ListStru regservice;/* struct GATT_ServiceStru, temply save unsolved include service, kill when convert into item */
	struct ListStru notifcbk;/* void *cbk, Client cbk for server notification & indication on value_handle */
};

enum {
	ATT_SIGNED = 						0x80,/* Bit7, Authentication Signature Flag */
	ATT_COMMAND = 						0x40,/* Bit6 */
	ATT_RSP = 							0x01/* Use to distinguish between request and response */
};

enum {/* Attribute Opcode Summary */
	ATT_OP_ERROR_RSP = 					0x01,
	ATT_OP_EXCHANGE_MTU_REQ = 			0x02,
	ATT_OP_EXCHANGE_MTU_RSP = 			0x03,
	ATT_OP_FIND_INFORMATION_REQ = 		0x04,
	ATT_OP_FIND_INFORMATION_RSP = 		0x05,
	ATT_OP_FIND_BY_TYPE_VALUE_REQ = 	0x06,
	ATT_OP_FIND_BY_TYPE_VALUE_RSP = 	0x07,
	ATT_OP_READ_BY_TYPE_REQ = 			0x08,
	ATT_OP_READ_BY_TYPE_RSP = 			0x09,
	ATT_OP_READ_REQ = 					0x0A,
	ATT_OP_READ_RSP = 					0x0B,
	ATT_OP_READ_BLOB_REQ = 				0x0C,
	ATT_OP_READ_BLOB_RSP = 				0x0D,
	ATT_OP_READ_MULTIPLE_REQ = 			0x0E,
	ATT_OP_READ_MULTIPLE_RSP = 			0x0F,
	ATT_OP_READ_BY_GROUPTYPE_REQ = 		0x10,
	ATT_OP_READ_BY_GROUPTYPE_RSP = 		0x11,
	ATT_OP_WRITE_REQ = 					0x12,
	ATT_OP_WRITE_RSP = 					0x13,
	ATT_OP_WRITE_COMMAND = 				0x52,/* 0x12 | ATT_COMMAND */
	ATT_OP_WRITE_COMMAND_SIGNED = 		0xD2,/* 0x12 | ATT_COMMAND | ATT_SIGNED */
	ATT_OP_PREPARE_WRITE_REQ = 			0x16,
	ATT_OP_PREPARE_WRITE_RSP = 			0x17,
	ATT_OP_EXECUTE_WRITE_REQ = 			0x18,
	ATT_OP_EXECUTE_WRITE_RSP = 			0x19,
	ATT_OP_HANDLE_VALUE_NOTIFICATION = 	0x1B,/* 27, no 0x1A, 0x1C */
	ATT_OP_HANDLE_VALUE_INDICATION = 	0x1D,/* 29 */
	ATT_OP_HANDLE_VALUE_CONFIRMATION = 	0x1E/* 30 */
};

enum {/* Format in Find Information Response */
	ATT_FIRF_HANDLE_16UUID = 			0x01,/* A list of 1 or more handles with their 16 bit Bluetooth UUIDs */
	ATT_FIRF_HANDLE_128UUID = 			0x02/* A list of 1 or more handles with their 128 bit UUIDs */
};

enum {/* Flags in Execute Write Request */
	ATT_EWRF_CANCEL_ALL = 				0x00,
	ATT_EWRF_IMMEDIATELY_WRITE_ALL = 	0x01
};

struct ATT_HandleRangeStru {/* ATT_RANGELIST node, for service handle range */
	void *cbk;/* Service owner, (*(GATTFunc *)(cbk))(context, ev, arg); */
	UINT16 starthdl;/* Service handle */
	UINT16 endhdl;/* Service end handle */
};

enum {/* mask index used in ATT_Item_GetByHandle */
	ATT_FINDMASK_READ_NOT_PERMITTED = 		0x0001,
	ATT_FINDMASK_WRITE_NOT_PERMITTED = 		0x0002,
	ATT_FINDMASK_REQUEST_NOT_SUPPORTED = 	0x0004,
	ATT_FINDMASK_INVALID_OFFSET = 			0x0008,
	ATT_FINDMASK_INVALID_VALUE_LEN = 		0x0010,
	ATT_FINDMASK_PREPARE_QUEUE_FULL = 		0x0020,

	/* FIXME, was designed as when encount these PDU types, requires for security, 
	But security only applies on some attributes */
	ATT_FINDMASK_SECURITY = 				0x0040,/* Authorization, Authentication, Encryption, KeySize */
	ATT_FINDMASK_ATTRIBUTE_NOT_LONG = 		0x0080,/* Read_Blob on short Attribute, is allowed, By Apple */
	ATT_FINDMASK_UNSUPPORTED_GROUP_TYPE = 	0x0100,
	ATT_FINDMASK_COMMAND = 					0x0200/* No response */
};

enum {/* mask used in ATT_Item_GetByHandle & ATT_Item_GetInRange */
	
	/* 2 handles */
	ATT_FINDMASKBY_FIND_INFORMATION_REQ = 
		(0),
		
	ATT_FINDMASKBY_FIND_BY_TYPE_VALUE_REQ = 
		(ATT_FINDMASK_REQUEST_NOT_SUPPORTED),
		
	ATT_FINDMASKBY_READ_BY_TYPE_REQ = 
		(ATT_FINDMASK_REQUEST_NOT_SUPPORTED 
		| ATT_FINDMASK_SECURITY 
		| ATT_FINDMASK_READ_NOT_PERMITTED),/* App err */
		
	ATT_FINDMASKBY_READ_BY_GROUPTYPE_REQ = 
		(ATT_FINDMASK_REQUEST_NOT_SUPPORTED
		| ATT_FINDMASK_READ_NOT_PERMITTED 
		| ATT_FINDMASK_UNSUPPORTED_GROUP_TYPE),/* App err */	

	/* 1 handle */
	ATT_FINDMASKBY_READ_REQ = 
		(ATT_FINDMASK_READ_NOT_PERMITTED 
		| ATT_FINDMASK_SECURITY),/* App err */
		
	ATT_FINDMASKBY_READBLOB_REQ = 
		(ATT_FINDMASK_READ_NOT_PERMITTED 
		| ATT_FINDMASK_SECURITY 
		| ATT_FINDMASK_REQUEST_NOT_SUPPORTED 
		| ATT_FINDMASK_INVALID_OFFSET 
		| ATT_FINDMASK_ATTRIBUTE_NOT_LONG),/* App err */
		
	ATT_FINDMASKBY_READMULTIPLE_REQ = 
		(ATT_FINDMASK_READ_NOT_PERMITTED 
		| ATT_FINDMASK_REQUEST_NOT_SUPPORTED 
		| ATT_FINDMASK_SECURITY),/* App err */

	ATT_FINDMASKBY_SIGNED_WRITE_CMD = 
		(ATT_FINDMASK_COMMAND
		| ATT_FINDMASK_WRITE_NOT_PERMITTED 
		| ATT_FINDMASK_INVALID_VALUE_LEN),/* Just check, do not response */
		
	ATT_FINDMASKBY_WRITE_CMD = 
		(ATT_FINDMASK_COMMAND
		| ATT_FINDMASK_SECURITY
		| ATT_FINDMASK_WRITE_NOT_PERMITTED 
		| ATT_FINDMASK_INVALID_VALUE_LEN),/* Just check, do not response */
		
	ATT_FINDMASKBY_WRITE_REQ = 
		(ATT_FINDMASK_REQUEST_NOT_SUPPORTED 
		| ATT_FINDMASK_SECURITY 
		| ATT_FINDMASK_WRITE_NOT_PERMITTED 
		| ATT_FINDMASK_INVALID_VALUE_LEN),/* App err */
		
	ATT_FINDMASKBY_PREPARE_WRITE_REQ = 
		(ATT_FINDMASK_REQUEST_NOT_SUPPORTED 
		| ATT_FINDMASK_SECURITY 
		| ATT_FINDMASK_WRITE_NOT_PERMITTED 
		| ATT_FINDMASK_INVALID_OFFSET
		| ATT_FINDMASK_INVALID_VALUE_LEN
		| ATT_FINDMASK_PREPARE_QUEUE_FULL),/* App err */
};

enum {
	ATT_UUID16_TYPEMASK =			0xFF00,
	ATT_UUID16_GATTSERVICES =		0x1800,/* GATT_UUID_GENERIC_ACCESS */
	ATT_UUID16_GATTATTRIBUTE =		0x2800,/* GATT_UUID_GATT_PRIMARY_SERVICE_DECLARATION */
	ATT_UUID16_GATTDESC =			0x2900,/* GATT_UUID_CHARACTERISTIC_EXTENDED_PROPERTIES */
	ATT_UUID16_GATTCHARACTER =		0x2A00/* GATT_UUID_DEVICE_NAME */
};

#define ATT_UUID16_GETTYPE(uuid_) 	((uuid_) & ATT_UUID16_TYPEMASK)

enum {/* op in ATT_Item_ModifyPermissionCore */
	GATT_PERMISSION_ADD,
	GATT_PERMISSION_REMOVE,
	GATT_PERMISSION_SET
};

/*---------------------------------------------------------------------------*/
/* 					GATT Transaction Function UI Structure				     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Description:
	This function use to send a GATT task according to the task context.
---------------------------------------------------------------------------*/
typedef void (GATTTaskFunc) (struct ATT_TLStru *tl, void *task);
#define GATT_Task_SendPDU(tl_, task_)	(*(GATTTaskFunc *)(((struct GATT_TaskCommonStru *)(task_))->send))((tl_), (task_));

struct GATT_TaskSMP_ContextStru {
	struct HCI_AddressStru addr;		/* Remote addr, to get struct ATT_TLStru *tl */
	UINT8 task_index;					/* Task index in TL namespace, to get struct GATT_TaskCommonStru *task */
};

/*---------------------------------------------------------------------------
Description:
	void *context in task.
---------------------------------------------------------------------------*/
struct GATT_TaskCommonStru {
	struct GATT_TL_ConnectStru tlin;
	GATTTaskFunc *send;/* GATT task convert to ATT PDU functions, etc: GATT_Task_2HDL_UUID */
	UINT8 index;/* index of the task */
	UINT8 task;/* etc: GATT_TASK_EXCHANGE_MTU */
};

struct GATT_MTU_CTStru {
	struct GATT_TaskCommonStru c;
	UINT16 rx_mtu;/* Local receive MTU */
};

struct GATT_2HDL_UUID_CTStru {
	struct GATT_TaskCommonStru c;
	UINT32 hdl;
	struct ATT_UUIDStru uuid;
};

struct GATT_2HDL_CTStru {
	struct GATT_TaskCommonStru c;
	UINT32 hdl;/* Starthdl << 16 | Endhdl */
};

struct GATT_List_CTStru {
	struct GATT_TaskCommonStru c;
	struct ListStru list;/* struct ATT_HandleValueStru or UINT16 * for read req */
};

struct GATT_Read_CTStru {
	struct GATT_TaskCommonStru c;
	UINT16 hdl;
	UINT16 offset;
};

struct GATT_Write_CTStru {
	struct GATT_TaskCommonStru c;
	struct ATT_HandleValueStru *in;
};

void GATT_Task_MTU(struct ATT_TLStru *tl, struct GATT_MTU_CTStru *ct);
void GATT_Task_2HDL_UUID(struct ATT_TLStru *tl, struct GATT_2HDL_UUID_CTStru *ct);
void GATT_Task_2HDL(struct ATT_TLStru *tl, struct GATT_2HDL_CTStru *ct);
void GATT_Task_Read(struct ATT_TLStru *tl, struct GATT_Read_CTStru *ct);
void GATT_Task_List(struct ATT_TLStru *tl, struct GATT_List_CTStru *ct);
void GATT_Task_Read_Multiple(struct ATT_TLStru *tl, struct GATT_List_CTStru *ct);
void GATT_Task_Write(struct ATT_TLStru *tl, struct GATT_Write_CTStru *ct);
void *GATT_Task_Target(UINT32 len, struct GATT_TL_ConnectStru *tlin, UINT8 task, GATTTaskFunc *send);
void GATT_Task_SMP_Req(struct ATT_TLStru *tl, UINT8 task_index, UINT8 sec_mode);

void ATT_UUID_Get(struct ATT_UUIDStru *out, UINT8 *p, UINT8 len);
void ATT_UUID_Put(struct ATT_UUIDStru *in, UINT8 *p);

struct ATT_TLStru *GATT_TL_FindCreateByADDR(struct HCI_AddressStru *addr, UINT8 create, UINT8 require);
void GATT_TL_L2CAP_Cbk(UINT16 msgid, void *arg);
void GATT_Server_ServiceChanged(UINT16 starthdl, UINT16 endhdl);
void GATT_TL_Done(struct ATT_TLStru *tl, UINT8 reason, UINT8 done);

UINT16 ATT_GetMTU(struct ListStru *list, UINT32 maxmtu, UINT8 step);
struct ATT_ItemStru *ATT_Item_FindBy_Handle(UINT16 hdl);
struct ATT_ItemStru *ATT_Item_FindBy_UUID(UINT16 uuid);
struct ATT_ItemStru *ATT_Item_FindBy_UUIDCore(UINT16 start_hdl, struct ATT_UUIDStru *uuid);
struct ATT_ItemStru *ATT_Item_New(UINT16 len, UINT16 hdl, UINT8 valtype);
void ATT_Item_Free(struct ATT_ItemStru *item);
struct ATT_ItemStru *ATT_Item_Reg(UINT16 len, UINT16 hdl, UINT8 valtype, UINT16 properties);
struct ATT_ItemTLStru *ATT_Item_GetTLNode(struct HCI_AddressStru *addr, struct ATT_ItemStru *item, UINT8 create);
UINT8 *ATT_Item_GetValueP(struct HCI_AddressStru *addr, struct ATT_ItemStru *item);
UINT16 ATT_Item_GetValueLen(struct ATT_ItemStru *item);
UINT8 ATT_Item_Write(struct ATT_TLStru *tl, struct ATT_ItemStru *item, UINT8 *p, UINT16 off, UINT16 len);
UINT8 *ATT_PDU_New(UINT32 len);
void ATT_PDU_TLSend(struct BuffStru *in, struct ATT_TLStru *tl);

struct ATT_HandleRangeStru *ATT_Range_Get(UINT16 hdl);
UINT16 ATT_Range_GetGroupEnd(struct ATT_ItemStru *item);
UINT16 ATT_Range_GetFreeSpace(UINT16 starthdl);
UINT16 ATT_Range_Allocate(UINT16 num, void *cbk);
void ATT_Range_Free(UINT16 starthdl);

void ATT_Error_Rsp(struct ATT_TLStru *tl, UINT32 op_hdl_err);
void GATT_Error_Cfm(struct ATT_TLStru *tl, UINT8 errcode);
void ATT_GeneralWrite_Req(struct ATT_TLStru *tl, struct ATT_HandleValueStru *in, UINT8 opcode, UINT8 type);
void ATT_PrepareWrite_PDU(struct ATT_TLStru *tl, struct ATT_HandleValueStru *in, UINT8 opcode);
void ATT_ExecuteWrite_Req(struct ATT_TLStru *tl, UINT8 flags);
void ATT_General_Rsp(struct ATT_TLStru *tl, UINT8 opcode);

void ATT_Recv_Cfm(struct ATT_TLStru *tl, UINT8 *p, UINT8 *end, UINT8 event);
void ATT_Recv_Ind(struct ATT_TLStru *tl, UINT8 *p, UINT8 *end, UINT8 op);
void ATT_Read_More(struct ATT_TLStru *tl, void *cbk, void *context, UINT16 hdl, UINT16 offset);
void ATT_Read_Rsp(struct ATT_TLStru *tl, struct ATT_ItemStru *item, UINT8 op, UINT16 mask, UINT16 offset);
void ATT_ReadMultiple_Rsp(struct ATT_TLStru *tl, struct ListStru *list, UINT16 mask);
void ATT_Write_Rsp(struct ATT_TLStru *tl, UINT8 errcode, UINT8 op, UINT16 hdl);
void ATTC_PendIND_Response(struct FsmInst *fi, UINT8 event, struct ATT_PendIndStru *in);

void GATT_TL_TransInit(struct ATT_TLStru *tl, UINT8 *pdata, UINT8 type);
void GATT_TL_TransDone(struct ATT_TLStru *tl);
void GATTC_TL_DataInd(struct FsmInst *fi, UINT8 event, struct BuffStru *arg);
void GATTC_TL_HCI_LELink_Cbk(struct FsmInst *fi, UINT8 event, struct HCI_LinkCbkStru *in);
void GATTC_TL_Timeout(struct FsmInst *fi, UINT8 event, struct ATT_TLStru **in);
void GATT_HDLValueNotifyCbk(struct ATT_TLStru *tl, UINT16 hdl, UINT16 offset, UINT8 *p, UINT16 len);

void GATT_MTU_Cbk(struct ATT_TLStru *tl);
void GATT_HDLValueCbk(void *cbk, void *context, struct HCI_AddressStru *addr, UINT16 hdl, UINT16 offset, UINT8 *p, UINT32 len);
void GATT_EventCBK(void *cbk, void *context, void *arg, UINT8 ev);
void GATT_Server_NotifySend(void);
void ATT_Item_ModifyPermissionCore(struct ATT_ItemStru *item, UINT8 op, UINT16 permission);

void GATTC_Server_NotifyIndicate(struct FsmInst *fi, UINT8 event, struct GATT_Server_NotifyIndicateStru *in);
void GATTC_Server_RegTree(struct FsmInst *fi, UINT8 event, struct GATT_ServiceStru *service);
void ATTC_Item_ModifyPermission(struct FsmInst *fi, UINT8 event, struct ATT_HandlePermissionStru *in);
struct ATT_ItemStru *ATT_Item_ModifyValueCore(struct ATT_ItemStru *item, struct ATT_ValueStru *val);
void ATTC_Item_ModifyValue(struct FsmInst *fi, UINT8 event, struct ATT_ModifyValueStru *in);
void GATTC_Server_UnRegTree(struct FsmInst *fi, UINT8 event, struct GATT_Hdl_UIStru *in);
void GATTC_Task_Cancel(struct FsmInst *fi, UINT8 event, struct GATT_TaskCancelStru *in);
void GATTC_Task_Create(struct FsmInst *fi, UINT8 event, struct GATT_TaskCommonStru *in);
void GATTC_NotificationCbk_Reg(struct FsmInst *fi, UINT8 event, GATTFunc *cbk);
void GATTC_NotificationCbk_UnReg(struct FsmInst *fi, UINT8 event, GATTFunc *cbk);

void GATT_Task_MoreHandles(struct ATT_TLStru *tl, UINT32 *curhdl, UINT16 lasthdl);
void GATT_Task_Complete(struct ATT_TLStru *tl, struct GATT_TaskCommonStru *task, UINT8 errcode);
void GATT_Task_Send(struct ATT_TLStru *tl);
void GATT_Task_Free(struct GATT_TaskCommonStru *in);
void GATT_Free_Descriptor(struct GATT_DescriptorStru *d);
void GATT_Free_Character(struct GATT_CharacteristicStru *c);
void GATT_Free_Service(struct GATT_ServiceStru *service);

void GATTC_TL_Disconnect(struct FsmInst *fi, UINT8 event, struct GATT_TL_DisconnectStru *arg);

void ATT_DBG_Packet(UINT8 *prompt, struct BuffStru *buf);
void ATT_DBG_Item(struct ATT_ItemStru *item);
void ATT_DBG_TL(UINT8 *prompt, struct ATT_TLStru *tl);
void ATT_DBG_PendInd(UINT8 *prompt, struct ATT_PendIndStru *in);
void GATT_DBG_Task(UINT8 *prompt, struct ATT_TLStru *tl, struct GATT_TaskCommonStru *task, UINT8 errcode);
void ATT_DBG_Item_FindInRanges(UINT8 opcode, struct ListStru *att, UINT8 errcode);
void ATT_DBG_Item_FindByHandleInPDU(UINT8 op, struct ATT_ItemStru *node, UINT8 errcode);
void ATT_DBG_Execute_Write_All(struct ATT_TLStru *tl);

#endif/* G_ATT_HLP_H */

