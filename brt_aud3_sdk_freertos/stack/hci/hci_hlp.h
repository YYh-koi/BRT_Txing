/*---------------------------------------------------------------------------
Module Name:
	hci_hlp.h
Abstract:
	This module defines HCI internal structure, macro & functions.
Author:
	Wang Yao
---------------------------------------------------------------------------*/

#ifndef G_HCI_HLP_H
#define G_HCI_HLP_H

#include "hci_ui.h"

extern struct FsmInst *hci_fsminst;

/*---------------------------------------------------------------------------*/
/* 								HCI Interface Macros	 	       	  	     */
/*---------------------------------------------------------------------------*/
enum {
	/* Ret in HCI_Command_CheckBlock */
	HCI_CMD_ALLOW,
	HCI_CMD_BLOCK_BYNUM,
	HCI_CMD_BLOCK_BYOPCODE,

	/* Debug Only */
	HCI_CMD_FINDBYOPCODE,
	HCI_CMD_FINDBYEVENT,
	HCI_CMD_ADD,
	HCI_CMD_REMOVE
};

struct HCI_BitPointerStru {/* For HCI_Coding_GetBits */
	UINT8 *p;/* Current Octet pointer */
	UINT8 off;/* Keep current octet remaining bits usage offset */
};

struct HCI_XParamStru {
	UINT8 len;
	UINT8 data[1];
};

enum {/* Mask in struct HCI_CommandNodeStru */
	HCI_CMDMASK_COMMANDSTATUS = 		0x01,/* Set after receive Command_Status */
	HCI_CMDMASK_NO_FLOW_CONTROL = 		0x02/* Vendor with no returning or host_number_of_complete_packets */
};

struct HCI_CommandNodeStru {/* Level 1 command node */
	struct FsmTimer *timer;/* RTX/ERTX Timer */
	void *data;/* raw structure for context. */
	void *context;/* application context pointer */
	UINT16 opcode;/* etc: HCI_OPC_INQUIRY */
	UINT16 struinfo;/* Command & Return structure convert info */
	UINT8 evindex;/* Waiting for this event for complete, if command op may wait more than 1 event, check table for patch */
	UINT8 mask;/* etc: HCI_CMDMASK_COMMANDSTATUS */
};

struct HCI_RTXTimerStru {
	void *local;/* to find local controller node */
	void *cmd;/* to find command node in local */
};

struct HCI_LocalTimerStru {
	void *local;/* to find local controller node */
};

/*---------------------------------------------------------------------------*/
/* 							Local/Remote Object structure		   	  	     */
/*---------------------------------------------------------------------------*/
enum {/* timeout[x] in struct HCI_LocalInfoStru */
	HCI_LINFO_TIMEOUT_PAGE,
	HCI_LINFO_TIMEOUT_CONN_ACCEPT,
	HCI_LINFO_TIMEOUT_LOGICAL_LINK_ACCEPT,

	HCI_LINFO_TIMEOUT_SLOTS/* x counter */
};

enum {/* packet_length[x] & packet_num[x] in struct HCI_LocalInfoStru */
	HCI_LINFO_PACKET_ACL,
	HCI_LINFO_PACKET_LE,/* Maybe different with ACL */
	HCI_LINFO_PACKET_DATABLOCK,
#ifdef CONFIG_HC2HOST_FC_ENABLE
	HCI_LINFO_PACKET_CONTROLLER_TO_HOST,
#endif

	HCI_LINFO_PACKET_SLOTS/* x counter */
};

enum {/* mask in struct HCI_LocalInfoStru */
	HCI_LINFO_MASK_IN_PERIODIC_INQUIRY_MODE = 		0x0001,
		
	/* Two methods of data flow control are defined: 'packet-based' flow control and 
	'data-block-based' flow control, known as buffer management */
	HCI_LINFO_MASK_BLOCK_BASED_DATA_FLOW_CONTROL = 	0x0002,
	HCI_LINFO_MASK_LOOPBACK_MODE = 					0x0004,/* Loopback mode, not used now */
	HCI_LINFO_MASK_SET_EVENT_FILTER_INQUIRY = 		0x0008,

	HCI_LINFO_MASK_LIMITED = 						0x0010,/* Limited scan/advertise for LE, FIXME, scan+adv toghter? */
	HCI_LINFO_MASK_STATIC_ADDRESS = 				0x0020,/* Static address only generate once when power on, use to control generate only once */
	HCI_LINFO_MASK_RANDOM_ADDRESS = 				0x0040,/* Link is using Local random address(otherwise, public), own address type in adv or connect */
	HCI_LINFO_MASK_BONDABLE = 						0x0080,/* Local bondable */
	HCI_LINFO_MASK_LE_MASTER = 						0x0100,/* Local is LE_Slave / Master, for LE, there are no Role change */
	HCI_LINFO_MASK_FAST_AUTO_SLOW = 				0x0200,/* LE Advertise/Scan Fast to Slow automaticly */
	HCI_LINFO_MASK_CONTROLLER_TO_HOST_FLOW_CONTROL = 0x0800,
	HCI_LINFO_MASK_LE_SHARE_ACL_BUFFER = 			0x1000,/* LE_Read_Buffer_Size return 0, shared between BR/EDR and LE transports */
	HCI_LINFO_MASK_RESET_KEEP_BITS = 				(HCI_LINFO_MASK_BONDABLE)
};

struct HCI_LocalInfoStru {/* Sub stru of struct HCI_LocalStru */
	UINT32 timeout[HCI_LINFO_TIMEOUT_SLOTS];		/* [x] as HCI_LINFO_TIMEOUT_PAGE */
	UINT16 packet_length[HCI_LINFO_PACKET_SLOTS];	/* [x] as HCI_LINFO_PACKET_ACL */
	UINT16 packet_num[HCI_LINFO_PACKET_SLOTS];		/* [x] as HCI_LINFO_PACKET_ACL */
	UINT16 mask;									/* Bitmask etc:HCI_LINFO_MASK_IN_PERIODIC_INQUIRY_MODE */
	UINT8 hci_version;								/* HCI_HCI_VERSION_4_0 */
	UINT8 supported_commands[64];					/* Supported Command Bit mask table */
	UINT8 lmp_features[2][8];						/* FIXME, extended features 8 pages? current allocate 2 page? */
	UINT8 le_features[8];							/* LE features */
	UINT8 white_list_size;							/* Controller capablity */
	struct ListStru white_list;						/* Node as struct HCI_AddressStru */
};

struct HCI_LocalNOCPNodeStru {
	void *func;
	void *ev;
};

enum {/* Timer event of struct HCI_LocalStru */
	HCI_LOCALRTX_NOCP = 							1,/* NOCP */
	HCI_LOCALRTX_LE_ADVERTISE = 					2,/* Time Limited, DONOT change sequence, 2 */
	HCI_LOCALRTX_LE_SCAN = 							3,/* Time Limited, DONOT change sequence, 3 */
	HCI_LOCALRTX_LE_FAST_AUTO_SLOW_ADVERTISE = 		4,/* Fast to slow, DONOT change sequence, 4 */
	HCI_LOCALRTX_LE_FAST_AUTO_SLOW_SCAN = 			5,/* Fast to slow, DONOT change sequence, 5 */
	HCI_LOCALRTX_LE_RESOLVABLE_PRIVATE_ADDRESS = 	6,/* Change address */
	HCI_LOCALRTX_UNKNOWN_ACLDATA =					7,/* Cache ACL data before connection handle awares */

	HCI_LOCALRTX_TIMER_COUNT/* Num of Local Timer */
};

enum {/* index of addr[x] in struct HCI_LocalStru */
	HCI_ADDR_PUBLIC_STATIC = 						0,/* Public or Static, distinguish by the 49th bit in addr */
	HCI_ADDR_RANDOM = 								1/* Current random address, maybe static */
};

enum {/* le_status in local controller object struct HCI_LocalStru */
	/* Bitmask */
	HCI_LOCAL_LESTATUS_MASK_DISABLEING = 			(0x01),/* 1 for XOR purpose, donot change to other value */
	HCI_LOCAL_LESTATUS_MASK_ENABLEING = 			(0x02),
	HCI_LOCAL_LESTATUS_MASK = 						(HCI_LOCAL_LESTATUS_MASK_DISABLEING | HCI_LOCAL_LESTATUS_MASK_ENABLEING),

	/* Enum */
	HCI_LOCAL_LESTATUS_IDLE = 						(0),/* Connected include */
	HCI_LOCAL_LESTATUS_SCAN_ENABLEING = 			(0x20 | HCI_LOCAL_LESTATUS_MASK_ENABLEING),/* Set Scan Enable Command has been send to L1 */
	HCI_LOCAL_LESTATUS_SCAN = 						(0x20),/* Scan Enabled */
	HCI_LOCAL_LESTATUS_SCAN_DISABLEING = 			(0x20 | HCI_LOCAL_LESTATUS_MASK_DISABLEING),/* Set Scan Disable Command has been send to L1 */
	HCI_LOCAL_LESTATUS_INITIATING = 				(0x40),/* LE Create Connection Command has been send to L1 */
	HCI_LOCAL_LESTATUS_ADVERTISE_ENABLEING = 		(0x80 | HCI_LOCAL_LESTATUS_MASK_ENABLEING),/* Set Advertise Enable Command has been send to L1 */
	HCI_LOCAL_LESTATUS_ADVERTISE = 					(0x80),/* Advertising */
	HCI_LOCAL_LESTATUS_ADVERTISE_DISABLEING = 		(0x80 | HCI_LOCAL_LESTATUS_MASK_DISABLEING)/* Set Advertise Disable Command has been send to L1 */
};

struct HCI_LocalStru {						/* Local controller include BREDR/AMP */
	HANDLE tl_hdl;							/* Transport Layer Handle */
	TransportLayerStru *func;				/* Transport Layer Functions */
	struct FsmTimer *timer[HCI_LOCALRTX_TIMER_COUNT];/* Limited advertising or scanning for LE, NOCP delay control */
	struct ListStru acldata_list;			/* QoS send buffer list */
	struct ListStru cmd_list;				/* Level 1, struct HCI_CommandNodeStru */
	struct ListStru cmd2_list;				/* Level 2, struct HCI_CommandQueueStru */
	struct ListStru nocp_list;				/* Pending Local controller NOCP/NOCB list, struct HCI_LocalNOCPNodeStru */
	struct ListStru unknown_data;			/* Unknown ACLdata before connection handle awares, a patch for driver */
	struct HCI_CommandNodeStru *pendcmd;	/* Pointer to the 1st pendding command in cmd_list, pendcmd is not sent yet */
	struct HCI_LocalInfoStru info;			/* Local controller info */
	struct HCI_AddressStru addr[3];			/* etc: HCI_ADDR_PUBLIC_STATIC, AMP controller does not have BD, cap only vailed in [0=Public] */
	HCI_MPInt dcounter[2];					/* etc: HCI_DCOUNTER_SEND */
	UINT8 ampid;							/* HCI allocated Controller ID */
	UINT8 amp_status;						/* etc: HCI_AMP_STATUS_AVAILABLE_BUT_POWERED_DOWN */
	UINT8 le_status;						/* etc, HCI_LOCAL_LESTATUS_SCAN_ENABLEING */
	UINT8 scan_adv_status;					/* target status, HCI_LOCAL_LESTATUS_SCAN, HCI_LOCAL_LESTATUS_ADVERTISE, HCI_LOCAL_LESTATUS_IDLE */
	UINT8 cmd_num;							/* From Command_Status and Command_Complete num_hci_command_packets */
	UINT8 security_mode;					/* etc: HCI_SECURITY_MODE1 */
};

struct HCI_Security_RemoteStru {
	UINT8 io_capability;                   	/* [033],Enum */
	UINT8 oob_data_present;                	/* [034],Enum etc HCI_OOB_DATA_PRESENT_FROM_REMOTE_DEVICE_PRESENT, Remote have my OOB */
	UINT8 authentication_requirements;  	/* [035],Enum, Remote, from HCI_EV_IO_Capability_Response */
	UINT8 pin_code_len;						/* Default PINCode Length, if 0, PINCode not valid */
	UINT8 pin_code[16];						/* Default PINCode */
	UINT8 *oob_data;						/* Remote OOB data if exist, 2*16 octets, C/R, Local have peer`s OOB */
};

enum {/* mask in struct HCI_RemoteStru */
	HCI_REMOTEMASK_HOST_FEATURES = 	0x01	/* When got remote host features, don`t ask again */
};

enum {
	HCI_REMOTE_MIN = 0,
	HCI_REMOTE_MAX = 1
};

struct HCI_RemoteStru {						/* Only Remote Primary BREDR/LE controller, AMP is not a remote obj */
	struct HCI_Security_RemoteStru sec;
	struct HCI_AddressStru addr;			/* HCI use current address, whatever type it is, the address for controller */
	UINT16 clock_offset;					/* etc: HCI_CLOCK_OFFSET_VALID | x */
	UINT8 page_scan_repetition_mode;		/* etc: HCI_PAGE_SCAN_REPETITION_MODE_R0 */
	UINT8 mask;								/* etc: HCI_REMOTEMASK_HOST_FEATURES */
	UINT8 lmp_features[2][8];				/* FIXME, extended features 8 pages? current allocate 2 page? need bitmask if we have already asked */
	UINT16 conn_interval[2];                /* etc: HCI_REMOTE_MIN, Min, Max, [210],Time, From AD Perpherial Perfered Parameter */
};

/*---------------------------------------------------------------------------*/
/* 							Root Object structure						     */
/*---------------------------------------------------------------------------*/
struct HCI_RootStru {						/* hci_fsminst->user_data, HCI Root */
	struct ListStru local;					/* struct HCI_LocalStru, Local controllers */
	struct ListStru remote;					/* struct HCI_RemoteStru, Remote devices */
	struct ListStru link;					/* struct HCI_LinkStru, Links, include unestablished */
	struct ListStru evcbk;					/* struct HCI_EvCBKStru, HCI user callbacks */
	struct ListStru security;				/* struct HCI_SecurityInfoStru, security requirement registered by GAP */
	TL_CallbackStru *tlbase;				/* Poor transport layer interface.... */
	UINT8 io_capability;					/* etc: HCI_IO_CAPABILITY_DISPLAYONLY */
	UINT8 afhmap[11];						/* 81bits, AFH RF Channels map for BREDR, LE, WIFI */
};

/*---------------------------------------------------------------------------*/
/* 							Link Object structures					  	     */
/*---------------------------------------------------------------------------*/
struct HCI_LinkStru {						/* Common header of a Link */
	struct HCI_LocalStru *local;
	struct HCI_RemoteStru *remote;
	UINT16 connection_handle;				/* ACL/LE/Log Handle */
	UINT8 type;								/* HCI_LINK_TYPE_ACL, HCI_LINK_TYPE_LE, HCI_LINK_TYPE_LOG */
};

struct HCI_ACLLinkSecurityStru {
	struct ListStru reqlist;				/* struct HCI_SecurityNodeStru */
	UINT16 mask;							/* etc: HCI_SECURITY_AUTHENTICATE, HCI_SECURITY_LINKKEY */
	UINT8 gamp_lk[32];						/* 256bits save GAMP_LK = h2 (W, 'gamp', 32) or New_ GAMP_LK = h2 (GAMP_LK, 'gamp', 32) */
	UINT8 link_key[16];
	UINT8 key_type;							/* etc: HCI_KEY_TYPE_COMBINATION_KEY */
	UINT8 fail_counter;						/* Pairing Failes 3 consecutive times, ACLLink disconnect with error code 0x05 - Authentication Failure */
};

enum {/* Timer event */
	HCI_ACLLINK_TIMER_LAZYRTX = 1,
	HCI_ACLLINK_TIMER_TRANSRTX/* for L2CAP LE Update parameter */
};

enum {/* ACL status, status in HCI_ACLHDL level, in L2CAP_ACLLinkStru/HCI_LELinkStru */
	HCI_ACL_DATARECV = 						0x80,/* Status Bitmask, Valid status to receive ACL data */
	HCI_ACL_LECONN = 						0x40,/* Connecting or Cancel, LE command has no BD, so only one outstanding command may exist */
	HCI_ACL_LOCAL_DISCONNECT = 				0x20,/* Disconnecting or Cancel, only local initiated disconnection may lead to reconnect */

	HCI_ACL_IDLE = 							(0),
	HCI_ACL_CONNECTING = 					(1 | HCI_ACL_LECONN),
	HCI_ACL_READY = 						(2),
	HCI_ACL_READY_WITH_REMOTE_INFO = 		(3 | HCI_ACL_DATARECV),
	HCI_ACL_DISCONNECTING = 				(4 | HCI_ACL_DATARECV | HCI_ACL_LOCAL_DISCONNECT),
	HCI_ACL_CANCEL = 						(5 | HCI_ACL_LECONN | HCI_ACL_LOCAL_DISCONNECT),
	HCI_ACL_LECONNECT_WAITLOCAL = 			(6)/* Idle, but waiting for local status changes, etc, local is scaning, disable before initiate connection */
};

struct HCI_RecvContextStru {
	struct ListStru recvdata_list;			/* Uncompleted l2cap acl data packets, struct BuffStru ** */
	struct BuffStru **uncomplete_node;		/* 1st uncomplete l2cap packet node, this is used to cut recvdata_list into 2 parts,
											before, is completed nodes but status not ready(Geting Remote Info), 
											after, is uncompleted L2CAP segments */
	UINT32 exp_len;							/* length of data to complete current waiting l2cap data packet */
};

struct HCI_SendContextStru {
	UINT16 sending_num;						/* Host to controller outstanding packet before NoCP */
};

enum {/* mask in struct HCI_LinkStru */
	HCI_ACLMASK_LOCAL_DISCONNECT = 			0x01,/* Local initiated connection, local manage disconnect of the link */
	HCI_ACLMASK_READ_REMOTE_FEATURES = 		0x02,/* Reading remote supported features during ACL setup */
	HCI_ACLMASK_READ_REMOTE_EXT_FEATURES = 	0x04,/* Reading remote extended features during ACL setup */
	HCI_ACLMASK_REMOTE_INITIATING = 		0x08 /* Receiving connection request from the remote */
};

struct HCI_ACLLinkStru {/* Each remote shall have only one ACL-U */
	struct HCI_LinkStru link;
	struct HCI_SendContextStru send;
	struct HCI_RecvContextStru recv;

	/* Common part of ACL/LE */
	struct FsmTimer *timer; 			/* ACL lazy idle timer, delay ACL disconnection */
	UINT8 ref_count;					/* +-, How many cid session and echo/info ref this hci_hdl */
	UINT8 status; 						/* etc: HCI_ACL_CONNECTING */
	UINT8 mask;							/* etc: HCI_ACLMASK_LOCAL_DISCONNECT */
	UINT8 response_num;					/* Waiting for accept/reject the incoming connection upper cbk counter */

	/* Different part with LE */
	struct HCI_ACLLinkSecurityStru sec;
	UINT8 link_mode;					/* etc: HCI_CURRENT_MODE_SNIFF */
};

struct HCI_LELinkStru {/* Header part shall be equal to struct HCI_ACLLinkStru, Each remote shall have only one LE-U */
	struct HCI_LinkStru link;
	struct HCI_SendContextStru send;
	struct HCI_RecvContextStru recv;

	/* Common part of ACL/LE */
	struct FsmTimer *timer; 			/* ACL lazy idle timer, delay ACL disconnection */
	UINT8 ref_count;					/* +-, so signed, How many cid session and echo/info ref this hci_hdl */
	UINT8 status; 						/* etc: HCI_ACL_CONNECTING */
	UINT8 mask;							/* etc: HCI_ACLMASK_LOCAL_DISCONNECT */
	UINT8 response_num;					/* Waiting for accept/reject the incoming connection upper cbk counter */

	/* Different part with ACL */
	UINT8 le_features[8];
	UINT16 leconn_latency;              /* [243],Val */
	UINT16 supervision_timeout;         /* [211],Time */
};

struct HCI_LOGLinkStru {/* Physical link is removed since handle is allocated by host */
	struct HCI_LinkStru link;
	struct HCI_SendContextStru send;
	UINT8 tx_flow_spec_id;
};

struct HCI_Set_Channel_ClassificationStru {
	UINT32 c;/* Central Frequency, etc,2412 */
	UINT32 w;/* Bandwidth, 22,40 */
	UINT8 mask;/* etc, HCI_SETAFH_OCCUPY */
};

#define HCI_CMDSTRU 		void
#define HCI_CMDPAR(arg_)	((HCI_CMDSTRU *)(arg_))
#define HCI_CMDNEW 			NEW(sizeof(HCI_CMDSTRU))

/*---------------------------------------------------------------------------*/
/* 							HCI Data/Command Related		      	  	     */
/*---------------------------------------------------------------------------*/
enum {/* flag in HCI_ACLData_RecvDealList */
	/* Both Uncertain and Uncomplete */
	HCI_ACLDATA_RECV_REMOVE_ALL,

	/* This is designed for ACL/LE Ready, but going to read remote info before local can use the link, Not for 
	data received before connection complete event because we have no connection handle to get link object, UPF42 */
	HCI_ACLDATA_RECV_CBK_UNCERTAIN,

	/* Partial HCI Packets */
	HCI_ACLDATA_RECV_REMOVE_UNCOMPLETE
};

enum {/* Datatype in HCI_TL_RecvData */
	HCI_DATATYPE_COMMAND = 					1,
	HCI_DATATYPE_ACL = 						2,
	HCI_DATATYPE_SCO = 						3,
	HCI_DATATYPE_EVENT = 					4
};

struct HCI_TLRecvStru {/* Arg in HCIC_TL_RecvEvent */
	HANDLE host_hdl;
	UINT8 *data;/* Newed by HCI */
	UINT8 *end;/* end pointer of data, the 0xFD after last octet of valid data */
};

enum {/* Command RTX timeout */
	/* Since the maximum response timeout is strongly dependent on the HCI-Transport Layer used, 
	it is recommended to use a default value of one second for this timer */
	HCI_TIME_CMDRTX = 						2000,		/* Before Command Status */
	HCI_TIME_CMDERTX = 						10000,		/* After Command Status */
	HCI_TIME_CMDPINCODE = 					30000 * 2,	/* Both side user PIN Code input time */
	HCI_TIME_CMDDELTA = 					500,		/* Addtional time for calcuated time */
	HCI_TIME_ACLLAZY = 						1000,		/* ACL connection cast time, so when no body use ACLLink, still waiting some seconds */
	HCI_TIME_ACCEPT_CONNECTION_TIMEOUT = 	0xB540,		/* 0x0001~0xB540, 0.625~29s */
	HCI_TIME_LE_CREATE_CONNECTION = 		10000,
	HCI_TIME_LMP_TIMEOUT = 				30000
};

enum {/* Ret in HCI_Command_Compare2Handle */
	HCI_CMDHDL_MATCH = 						0,
	HCI_CMDHDL_2NULL = 						1,
	HCI_CMDHDL_NOTMATCH = 					2
};

enum {/* Flag in HCI_Local_RemoveOne */
	HCI_LOCAL_REMOVE,
	HCI_LOCAL_RESET
};

enum {/* addhead in HCI_CommandL2_AddNode */
	HCI_CMDL2_ADDHEAD,
	HCI_CMDL2_ADDTAIL
};

enum {/* return in HCI_Check_Set_Event_Filter_Inquiry */
	HCI_SET_EVENT_FILTER_ACTION_ADD_INQUIRY,
	HCI_SET_EVENT_FILTER_ACTION_CLEAR_INQUIRY,
	HCI_SET_EVENT_FILTER_ACTION_CLEAR_ALL,
	HCI_SET_EVENT_FILTER_ACTION_OTHER
};

typedef void (HCI_CbkTableFunc) (struct HCI_LocalStru *local, void *par);/* HCI internal callback function dispatcher */

#define HCI_ROOT								((struct HCI_RootStru *)(hci_fsminst->user_data))
#define HCI_LOCALLIST							(&(HCI_ROOT->local))
#define HCI_REMOTELIST							(&(HCI_ROOT->remote))
#define HCI_LINKLIST							(&(HCI_ROOT->link))/* ACL,LE,Logical Links */
#define HCI_EVCBKLIST							(&(HCI_ROOT->evcbk))/* Callback Tables */
#define HCI_SECURITYLIST						(&(HCI_ROOT->security))
#define HCI_SECURITY_LOCALIO					(HCI_ROOT->io_capability)
#define HCI_AFHMAP								(HCI_ROOT->afhmap)
#define HCI_BUFPRI(b)							(*(UINT8 *)((UINT8 *)(b) + 8))
#define HCI_SUPCMDGAP_LAST						(&hci_table_supportcmdgap[(sizeof(hci_table_supportcmdgap) / sizeof(struct HCI_OpCodeGapStru)) - 1])
#define HCI_EVGAP_LAST							(&hci_table_evgap[(sizeof(hci_table_evgap) / sizeof(struct HCI_IndexGapStru)) - 1])
#define HCI_EVOP_LAST 							(&hci_table_evop[(sizeof(hci_table_evop) / sizeof(struct HCI_EventCommandStru)) - 1])
#define HCI_CODINGP_EVENT(evi_)					(&hci_table_coding_Event[hci_table_codingoff_Event[evi_]])
#define HCI_LINKEVO(link_)						(((link_)->link.type == HCI_LINK_TYPE_ACL) ? HCI_EVO_ACL_DATA_IND : HCI_EVO_LE_DATA_IND)/* ACL or LE */
#define HCI_CMDERR(status_)						(((status_) << 16) | HCI_EVO_COMMAND_ERROR)/* status for HCI_Command_Remove, or HCI_OPC_ for HCI_Cbk */
#define HCI_Command_SendInternal(local_, par_, opscode_) 	HCI_Command_SendInternalCore((local_), HCI_Command_CreateNode((par_), (NULL), (opscode_)), (TRUE))
#define HCI_Command_SendInternal_Link(local_, par_, link_, opscode_) 	HCI_Command_SendInternalCore((local_), HCI_Command_CreateNode((par_), (link_), (opscode_)), (TRUE))

/* Macro components of struinfo */
#define HCI_STRU2REQOFF(struinfo_)				((UINT8)(struinfo_))/* Req struinfo offset */
#define HCI_STRU2RETOFF(struinfo_)				((UINT8)((struinfo_) >> 8))/* Ret struinfo offset, 0xFF for not exist */

/* UINT32 op_ev */
#define HCI_OPEV(o, e)							(((o) << 16) | (e))
#define HCI_OP(s)								((UINT16)((s) >> 16))
#define HCI_EV(s)								((UINT16)(s))

/* HCI is little endian */
#define HCI_DECODE2BYTE							DECODE2BYTE_LITTLE
#define HCI_DECODE4BYTE							DECODE4BYTE_LITTLE
#define HCI_ENCODE2BYTE							ENCODE2BYTE_LITTLE
#define HCI_ENCODE4BYTE							ENCODE4BYTE_LITTLE

/*---------------------------------------------------------------------------*/
/* 						HCI Structure <-> BitStream Macros 	      	  	     */
/*---------------------------------------------------------------------------*/
#ifdef CONFIG_ALIGN_FOURBYTES
#define CONFIG_ALIGN_BTYES						4
#elif defined(CONFIG_ALIGN_TWOBYTES)
#define CONFIG_ALIGN_BTYES						2
#else /* defined(CONFIG_ALIGN_ONEBYTE) */
#define CONFIG_ALIGN_BTYES						1
#endif
#define HCI_PADSTEP(s)							(MIN(CONFIG_ALIGN_BTYES, (s)))
#define HCI_PADDING(p, l)						((p) += ((HCI_PADSTEP(l) - ((UINT32)(p) % HCI_PADSTEP(l))) % HCI_PADSTEP(l)))

enum {
	HCI_CODE_0BITS = 							0x00,
	HCI_CODE_2BITS = 							0x01,
	HCI_CODE_4BITS = 							0x02,
	HCI_CODE_8BITS = 							0x03,

	/* 2Bits Basic Type Items */
	HCI_CODE_N1 = 								0x00,/* Num */
	HCI_CODE_U1 = 								0x01,/* 1Oct - UINT8 */
	HCI_CODE_U2 = 								0x02,/* 2Oct - UINT16 */
	HCI_CODE_U4 = 								0x03/* 4Oct - UINT32 */
};

enum {/* flag in HCI_CodingLen */
	HCI_CODING_INSTRU = 						0x01,/* Src input is structure, not air */
	HCI_CODING_OUTSRC = 						0x02,/* output src length, not dst */

	HCI_CODING_GETSTRULEN = 					(HCI_CODING_INSTRU | HCI_CODING_OUTSRC),/* Input stru, Output stru length */
	HCI_CODING_GETAIR = 						(HCI_CODING_INSTRU),/* Get PDU or length of PDU to the controller */
	HCI_CODING_GETSTRU = 						0/* GET structure or its length to upper layer */
};

enum {/* event in HCIC_Security_Request */
	HCI_SECURITY_EVENT_REQUEST = 				1,
	HCI_SECURITY_EVENT_ABORT = 					2
};

enum {/* Bitmask in authentication_requirements enum */
	HCI_AUTHENTICATION_REQUIREMENTS_NOMITM = 	0x00,
	HCI_AUTHENTICATION_REQUIREMENTS_MITM = 		0x01
};

/*---------------------------------------------------------------------------*/
/* 							Function Definations		        	  	     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Description:
	This function is called to send a packet into Transport Layer.
---------------------------------------------------------------------------*/
void HCI_TL_SendData(struct HCI_LocalStru *local, UINT8 *data, UINT32 len);

UINT8 HCI_DT_GetEvIndex(UINT8 evcode);
UINT8 HCI_DT_GetSupCMDBitIndex(UINT16 opcode);

void HCI_Cbk(struct HCI_LocalStru *local, void *context, void *arg, UINT32 len, UINT32 op_ev);
UINT8 HCI_CbkGetNum(UINT16 index);

void HCI_CommandL2_Remove(struct HCI_LocalStru *local, struct HCI_CommandQueueStru *queue, struct HCI_CommandQueueNodeStru *qnode, UINT16 status);

void LE_Local_StatusRequest(struct HCI_LocalStru *local, UINT8 status);
void LE_Local_StatusChanged(struct HCI_LocalStru *local, UINT8 status, UINT8 trysa);
void LE_Local_StatusEnableResult(struct HCI_LocalStru *local, UINT8 success);
void LE_Set_Advertise_Command(struct HCI_LocalStru *local, UINT8 enable);
void LE_Set_Scan_Command(struct HCI_LocalStru *local, UINT8 enable);

void HCI_Local_StartTimer(struct HCI_LocalStru *local, UINT8 *prompt, UINT32 timeout, UINT8 restart, UINT8 event);
void HCIC_Local_RTX(struct FsmInst *fi, UINT8 event, struct HCI_LocalTimerStru *in);
void HCI_Local_Init(struct HCI_LocalStru *local);
void HCI_Local_Remove(struct HCI_LocalStru *local);
void HCI_Local_RemoveOne(struct HCI_LocalStru *local, UINT8 flag);
void HCI_Local_ChangedNotify(struct HCI_LocalStru *local, HANDLE tl_hdl, UINT8 action);
struct HCI_LocalStru *HCI_Local_FindByHDL(HANDLE host_hdl);
struct HCI_LocalStru *HCI_Local_FindByAddr(struct HCI_AddressStru *l);
struct HCI_LocalStru *HCI_Local_FindByID(UINT8 ampid);
struct HCI_LocalStru *HCI_Local_FindByTLHDL(HANDLE tl_hdl);
void HCI_NOCP_FreeAll(struct HCI_LocalStru *local);
void HCI_NOCP_ConsumeNode(struct HCI_LocalStru *local);

void HCI_Remote_ChangeNotify(struct HCI_LocalStru *local, UINT8 src, struct HCI_RemoteStru *remote, UINT8 *class_of_device, UINT8 rssi, UINT8 *eir, UINT8 eir_len);
struct HCI_RemoteStru *HCI_Remote_FindByBD(UINT8 *bd);
struct HCI_RemoteStru *HCI_Remote_FindByAddr(struct HCI_AddressStru *r);
struct HCI_RemoteStru *HCI_Remote_FindCreateByAddr(struct HCI_LocalStru *local, UINT8 src, UINT8 *bd, UINT8 addr_type, UINT8 rssi, UINT8 *eir, UINT8 eir_len, UINT8 cap, UINT8 curaddr);
struct HCI_RemoteStru *HCI_Remote_FindCreateByBD(struct HCI_LocalStru *local, UINT8 src,  UINT8 *bd, UINT8 *class_of_device, UINT8 page_scan_repetition_mode, UINT16 clock_offset, UINT8 rssi, UINT8 *eir, UINT8 eir_len);
void HCI_Remote_Remove(struct HCI_RemoteStru *remote);
void HCI_Remote_RemoveUnuse(struct HCI_RemoteStru *remote);

void HCIC_PhysicalLink(struct FsmInst *fi, UINT8 event, struct HCI_PhysicalLinkStru *in);
void *HCI_Link_Create(struct HCI_LocalStru *local, struct HCI_RemoteStru *remote, UINT8 type);
void HCI_Link_RemoveByLR(void *lr);
void HCI_Link_RemoveAll(void);
void HCI_Link_Remove(struct HCI_ACLLinkStru *link, UINT8 reason);
void *HCI_Link_FindByHDL(struct HCI_LocalStru *local, UINT16 handle);
void HCI_ACLData_RecvCreateBuf(struct HCI_LocalStru *local, struct HCI_ACLLinkStru *link, struct BuffStru *in, UINT16 len);
void HCI_ACLLink_DisconnectSend(struct HCI_ACLLinkStru *acllink, UINT8 reason);

struct HCI_ACLLinkStru *HCI_ACLLink_FindByBDType(struct HCI_LocalStru *local, UINT8 *bd, UINT8 type, UINT8 linktype);
#define HCI_ACLLink_FindByBD(local_, bd_)		HCI_ACLLink_FindByBDType((local_), (bd_), HCI_OWN_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS, HCI_LINK_TYPE_ACL)

void HCI_ACLLink_Cbk(struct HCI_LocalStru *local, struct HCI_ACLLinkStru *link, UINT16 reason_job);
void HCI_ACLLink_Ready(struct HCI_ACLLinkStru *acllink);
void HCI_ACLLink_GetRemoteInfo_Done(struct HCI_ACLLinkStru *acllink);
void HCI_ACLLink_CreateConnectionCancel(struct HCI_ACLLinkStru *link);
void HCIC_ACLLink_UnRef(struct FsmInst *fi, UINT8 event, struct HCI_ACLLinkStru *acllink);
void HCI_ACLLink_CreateConnection(struct HCI_ACLLinkStru *link);
struct HCI_ACLLinkStru *HCI_ACLLink_FindByLR(struct HCI_LocalStru *local, struct HCI_RemoteStru *remote, UINT8 linktype);
struct HCI_ACLLinkStru *HCI_ACLLink_FindByPTR(void *acl);

void HCIC_TL_RecvACLData(struct FsmInst *fi, UINT8 pb, struct BuffStru *in);
void HCIC_TL_RecvSCOData(struct FsmInst *fi, UINT8 event, struct BuffStru *in);

UINT16 HCI_Set_PacketType(struct HCI_LocalStru *local, UINT8 link_type);
void HCI_Set_SynchronousParam(struct HCI_LocalStru *local, struct HCI_Synchronous_ConnectionStru *in, struct HCI_Synchronous_ConnectionStru *esco);
UINT8 HCI_Check_Set_Event_Filter_Inquiry(struct HCI_Set_Event_FilterStru *par);

UINT8 HCI_Command_CheckSupport(struct HCI_LocalStru *local, UINT16 opcode);
void HCI_Command_SendInternalCore(struct HCI_LocalStru *local, struct HCI_CommandNodeStru *cmd, UINT8 check);
struct HCI_CommandNodeStru *HCI_Command_CreateNode(void *par, void *context, UINT32 opscode);
void HCI_CommandL2_SendQueueToL1(struct HCI_LocalStru *local);
struct HCI_CommandQueueNodeStru *HCI_CommandL2_AddNode(struct HCI_LocalStru *local, UINT32 opscode, UINT8 addhead);
void HCI_Command_Remove(struct HCI_LocalStru *local, struct HCI_CommandNodeStru *cmd, void *arg, UINT16 len, UINT32 status_evindex);
void HCI_Command_CheckSend(struct HCI_LocalStru *local);
struct HCI_CommandErrorStru *HCI_Command_ErrorNew(UINT8 *stru, UINT8 poff, UINT16 *lenout, UINT16 status, UINT16 opcode);
void HCI_Command_RemoveByLink(struct HCI_LinkStru *link);
struct HCI_CommandNodeStru *HCI_Command_FindByOpCode(struct HCI_LocalStru *local, UINT16 opcode);
struct HCI_CommandNodeStru *HCI_Command_FindByEvIndex(struct HCI_LocalStru *local, UINT8 evindex, UINT16 opcode);
void HCI_Command_RemoveByOpCode(struct HCI_LocalStru *local, UINT16 opcode, UINT16 status);
void HCI_Command_SendEmergency(struct HCI_LocalStru *local, void *par, UINT32 opscode);

void HCI_ACLData_SendPendding(struct HCI_LocalStru *local);
void HCI_ACLData_RecvDealList(struct HCI_LocalStru *local, struct HCI_ACLLinkStru *link, UINT8 flag);

int HCI_TL_RecvData(HANDLE host_hdl, UINT8 *data, UINT32 len);
void HCIC_TL_RecvEvent(struct FsmInst *fi, UINT8 evi, struct HCI_TLRecvStru *in);

UINT8 HCI_Coding_GetBits(struct HCI_BitPointerStru *bp, UINT8 len);
UINT16 HCI_CodingLen(UINT8 *src, UINT8 *end, const UINT8 *p, UINT8 flag);
void HCI_Coding(UINT8 *src, UINT8 *end, UINT8 *dst, const UINT8 *p, UINT8 flag);
void HCI_Internal_Cbk(HANDLE host_hdl, void *context, void *par, UINT32 evindex);
void HCI_Internal_Cbk_Reg(void);

void HCI_EV_Command_Status_RestartERTX(struct HCI_LocalStru *local, struct HCI_CommandNodeStru *cmd);
void HCI_EV_Command_Status(struct HCI_LocalStru *local, struct HCI_Command_StatusEvStru *in);
void HCI_EV_Command_Complete(struct HCI_LocalStru *local, struct HCI_Command_CompleteEvStru *in);
void HCI_EV_LE_Meta(struct HCI_LocalStru *local, struct HCI_LE_MetaEvStru *ev);
void HCI_EV_Loopback_Command(struct HCI_LocalStru *local, struct HCI_Loopback_CommandEvStru *ev);

void HCI_EV_Link_Key_Request(struct HCI_LocalStru *local, struct HCI_Link_Key_RequestEvStru *ev);
void HCI_EV_Link_Key_Notification(struct HCI_LocalStru *local, struct HCI_Link_Key_NotificationEvStru *ev);
void HCI_EV_PIN_Code_Request(struct HCI_LocalStru *local, struct HCI_PIN_Code_RequestEvStru *ev);
void HCI_EV_Authentication_Complete(struct HCI_LocalStru *local, struct HCI_Authentication_CompleteEvStru *ev);
void HCI_EV_Encryption_Change(struct HCI_LocalStru *local, struct HCI_Encryption_ChangeEvStru *ev);
void HCI_EV_IO_Capability_Request(struct HCI_LocalStru *local, struct HCI_IO_Capability_RequestEvStru *ev);
void HCI_EV_IO_Capability_Response(struct HCI_LocalStru *local, struct HCI_IO_Capability_ResponseEvStru *ev);
void HCI_EV_User_Confirmation_Request(struct HCI_LocalStru *local, struct HCI_User_Confirmation_RequestEvStru *ev);
void HCI_EV_Remote_OOB_Data_Request(struct HCI_LocalStru *local, struct HCI_Remote_OOB_Data_RequestEvStru *ev);

void HCI_EV_Connection_Complete(struct HCI_LocalStru *local, struct HCI_Connection_CompleteEvStru *ev);
void HCI_EV_Connection_Request(struct HCI_LocalStru *local, struct HCI_Connection_RequestEvStru *ev);
void HCI_EV_Synchronous_Connection_Complete(struct HCI_LocalStru *local, struct HCI_Synchronous_Connection_CompleteEvStru *ev);
void HCI_EV_Logical_Link_Complete(struct HCI_LocalStru *local, struct HCI_Logical_Link_CompleteEvStru *ev);
void HCI_EV_LE_Connection_Complete(struct HCI_LocalStru *local, struct HCI_LE_Connection_CompleteEvStru *ev);
void HCI_EV_Disconnection_Complete(struct HCI_LocalStru *local, struct HCI_Disconnection_CompleteEvStru *ev);
void HCI_OP_Create_Connection_Cancel(struct HCI_LocalStru *local, struct HCI_Create_Connection_CancelCompStru *ev);

void HCIC_Init(void);
void HCIC_Done(struct FsmInst *fi, UINT8 event, void *arg);
void HCIC_Cbk_Reg(struct FsmInst *fi, UINT8 event, struct HCI_CbkNodeStru *arg);
void HCIC_Cbk_UnReg(struct FsmInst *fi, UINT8 event, const struct HCI_CbkLineStru *table);
void HCIC_Command_Send(struct FsmInst *fi, UINT8 event, struct HCI_CommandNodeStru *in);
void HCIC_CommandL2_Input(struct FsmInst *fi, UINT8 event, struct HCI_CommandQueueStru *queue);
void HCIC_Command_RTX(struct FsmInst *fi, UINT8 event, struct HCI_RTXTimerStru *in);
void HCIC_TL_Reg(struct FsmInst *fi, UINT8 event, struct HCI_LocalStru *local);
void HCIC_TL_UnReg(struct FsmInst *fi, UINT8 event, void *tl_hdl);
void HCIC_ACLData_Send(struct FsmInst *fi, UINT8 event, struct BuffStru *arg);
void HCIC_SCOData_Send(struct FsmInst *fi, UINT8 event, struct BuffStru *arg);
void HCIC_Create_SCOConnection(struct FsmInst *fi, UINT8 event, struct HCI_Create_SCOUIStru *in);
void HCIC_ACLLink_Disconnect(struct FsmInst *fi, UINT8 event, struct HCI_DisconnectStru *par);
void HCIC_ACLLink_ConnectionResponse(struct FsmInst *fi, UINT8 event, struct HCI_ACLConnection_ResponseUIStru *in);
void HCIC_Synchronous_Connection_Response(struct FsmInst *fi, UINT8 event, struct HCI_Synchronous_Connection_ResponseUIStru *in);
void HCIC_Set_Channel_Classification(struct FsmInst *fi, UINT8 event, struct HCI_Set_Channel_ClassificationStru *par);

void HCI_Security_Go(struct HCI_ACLLinkStru *link);
void HCI_Security_ResultEv(struct HCI_LocalStru *local, UINT16 hdl, UINT16 mask, UINT16 status);
void HCI_Security_LinkKey_CbkSave(struct HCI_LocalStru *local, struct HCI_Link_Key_NotificationEvStru *ev);
void HCI_Security_RemoveNodes_ByACLLink(struct HCI_ACLLinkStru *link);
void HCIC_Security_Authorized(struct FsmInst *fi, UINT8 event, struct HCI_Security_AuthorReqStru *in);
void HCIC_Security_Request(struct FsmInst *fi, UINT8 event, struct HCI_SecurityNodeStru *in);
void HCIC_Security_RequireReg(struct FsmInst *fi, UINT8 event, struct HCI_SecurityInfoStru *info);
void HCIC_Security_RequireUnReg(struct FsmInst *fi, UINT8 event, struct HCI_SecurityInfoStru *info);
void HCIC_Security_SetLocalIO(struct FsmInst *fi, UINT8 event, struct HCI_Security_LocalIOStru *in);
void HCIC_Security_SetBondable(struct FsmInst *fi, UINT8 event, struct HCI_Security_LocalBondableStru *in);
void HCIC_Security_SetOOB(struct FsmInst *fi, UINT8 event, struct HCI_Security_RemoteOOBStru *in);
void HCIC_Security_LinkKey_Reply(struct FsmInst *fi, UINT8 event, struct HCI_Security_Link_Key_Request_ReplyStru *in);
void HCIC_Security_LinkKey_Remove(struct FsmInst *fi, UINT8 event, UINT8 *bd);
void HCIC_Security_PIN_Code_Reply(struct FsmInst *fi, UINT8 event, struct HCI_PIN_Code_Request_ReplyStru *in);

struct HCI_SecurityInfoStru *HCI_Security_ObjectFind(struct HCI_SecurityInfoStru *node, struct HCI_SecurityInfoStru *in);/* Used in DBG */
UINT8 HCI_Security_LinkKeyGood(struct HCI_ACLLinkStru *link, struct HCI_SecurityNodeStru *node);/* Used in DBG */
UINT8 HCI_Security_Check_SupportSSP(struct HCI_LinkStru *link);/* Used in DBG */
void HMAC_SHA256(UINT8 *key, UINT32 key_len, UINT8 *data, UINT32 data_len);

void HCI_DBG_DeviceType(UINT8 device_type);/* for tl_reg */
void HCI_DBG_Local_LEStatus_View(struct HCI_LocalStru *local, UINT8 is_req, UINT8 status);
void HCI_DBG_LocalView(UINT8 *prompt, struct HCI_LocalStru *local);
void HCI_DBG_RemoteView(UINT8 *prompt, struct HCI_RemoteStru *remote);
void HCI_DBG_Command_Req(UINT8 *p, UINT16 len, UINT8 ampid);
void HCI_DBG_Event(UINT8 *p, UINT16 len, UINT8 ampid);
void HCI_DBG_CommandL2(struct HCI_CommandQueueStru *queue, UINT8 *prompt);
void HCI_DBG_CommandNode(struct HCI_LocalStru *local, struct HCI_CommandNodeStru *cmd, UINT8 reason, UINT16 status, UINT16 evindex);
void HCI_DBG_Command_CheckSupport1(struct HCI_LocalStru *local);
void HCI_DBG_Command_CheckSupport2(struct HCI_LocalStru *local, UINT16 opcode);
void HCI_DBG_ViewCodingP(const UINT8 *p);
void HCI_DBG_ACLLink(struct HCI_ACLLinkStru *acllink, UINT8 *prompt);
void HCI_DBG_LE_Meta(struct HCI_LocalStru *local, struct HCI_CommandNodeStru *cmd, UINT8 leev, void *stru, UINT16 len);
void HCI_DBG_Security_Request(struct HCI_SecurityNodeStru *in, UINT8 event);
void HCI_DBG_Security_Go(struct HCI_ACLLinkStru *link, struct HCI_SecurityNodeStru *node);
void HCI_DBG_Security_ResultEv(struct HCI_ACLLinkStru *link, struct HCI_SecurityNodeStru *node, UINT16 mask, UINT16 status);
void HCI_DBG_TL_RecvACLData(struct BuffStru *in);
void HCI_DBG_LEWhiteList(struct HCI_LocalStru *local, struct HCI_AddressStru *addr, UINT8 add);
void HCI_DBG_SetAFH(void);

#endif/* G_HCI_HLP_H */

