/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	map_hlp.h
Abstract:
	This file defines the internal data structure, constants and function
	declarations.
Author:
    Y.D.X
Revision History:
	1.	2013.10.31	Created.
------------------------------------------------------------------------*/
#ifndef _MAP_HLP_H
#define _MAP_HLP_H

#include "map_ui.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* Common*/
/* Additional client status masks */
#define MAP_CLNT_STM_DISCONNECT	        0x01
#define MAP_MAC_STM_PUT_CONTINUE	    0x02
#define MAP_MAC_STM_NOTIFICATION_ON	    0x04

/*App Param tag ID*/
typedef enum {
	MAP_APP_PARAM_MAXLISTCOUNT = 1,
	MAP_APP_PARAM_STARTOFFSET,
	MAP_APP_PARAM_FILTERMSGTYPE,
	MAP_APP_PARAM_FILTERPERIODBEGIN,
	MAP_APP_PARAM_FILTERPERIODEND,
	MAP_APP_PARAM_FILTERREADSTATUS,
	MAP_APP_PARAM_FILTERRECIPIENT,
	MAP_APP_PARAM_FILTERORIGINATOR,
	MAP_APP_PARAM_FILTERPRIORITY,
	MAP_APP_PARAM_ATTACHMENT,
	MAP_APP_PARAM_TRANSPARENT,
	MAP_APP_PARAM_RETRY,
	MAP_APP_PARAM_NEWMESSAGE,
	MAP_APP_PARAM_NOTIFYSTATUS,
	MAP_APP_PARAM_MASINSTID,
	MAP_APP_PARAM_PARAMETER_MASK,
	MAP_APP_PARAM_FOLDERLISTINGSIZE,
	MAP_APP_PARAM_MSGLISTINGSIZE,
	MAP_APP_PARAM_SUBJECT_LENGTH,
	MAP_APP_PARAM_CHARSET,
	MAP_APP_PARAM_FRACTION_REQ,
	MAP_APP_PARAM_FRACTION_DELIVER,
	MAP_APP_PARAM_STATUS_INDICATOR,
	MAP_APP_PARAM_STATUS_VALUE,
	MAP_APP_PARAM_MSE_TIME,
} tMAPH_AppParamType;

/* Operation type */
typedef enum {
	MAP_OP_CMD_SETNOTIFICATION,
	MAP_OP_CMD_SETMESSAGESTATUS,
	MAP_OP_CMD_UPDATEINBOX,

	MAP_OP_GET_FOLDERLIST,
	MAP_OP_GET_MESSAGELIST,
	MAP_OP_GET_MESSAGE,

	MAP_OP_PUSH_MESSAGE,
	MAP_OP_SET_FOLDER,

	MAP_OP_SEND_EVENT
} tMAPH_OPType;

#define MAP_PUSHMSG_PARAM_FLAGS			(MAP_FLAG_TRANSPARENT|MAP_FLAG_RETRY|MAP_FLAG_CHARSET)
#define MAP_GETFOLDERLIST_PARAM_FLAGS	(MAP_FLAG_MAXLISTCOUNT|MAP_FLAG_LISTSTARTOFFSET)
#define MAP_GETMSGLIST_PARAM_FLAGS		(MAP_FLAG_MAXLISTCOUNT|MAP_FLAG_LISTSTARTOFFSET|MAP_FLAG_SUBJECTLENGTH|\
										 MAP_FLAG_PARAMETERMASK|MAP_FLAG_FILTER_MESSAGETYPE|MAP_FLAG_FILTER_PERIODBEGIN|\
										 MAP_FLAG_FILTER_PERIODEND|MAP_FLAG_FILTER_READSTATUS|MAP_FLAG_FILTER_RECIPIENT|\
										 MAP_FLAG_FILTER_ORIGINATOR|MAP_FLAG_FILTER_PRIORITY)
#define MAP_GETMSG_PARAM_FLAGS			(MAP_FLAG_ATTACHMENT|MAP_FLAG_CHARSET|MAP_FLAG_FRACTIONREQUEST)

#define MAP_GETFOLDERLIST_RSP_PARAM_FLAGS (MAP_FLAG_FOLDERLISTINGSIZE)
#define MAP_GETMSGLIST_RSP_PARAM_FLAGS  (MAP_FLAG_NEWMESSAGE|MAP_FLAG_MESSAGELISTINGSIZE|MAP_FLAG_MSETIME)
#define MAP_GETMSG_RSP_PARAM_FLAGS      (MAP_FLAG_FRACTIONDELIVER)

/* Local MNS service instance status */
typedef enum {
	MAP_SVC_ST_WAI4CFM,	        /* 0 */
	MAP_SVC_ST_READY,	        /* 1 */
} tMAPH_SvcStatus;

/* MNS connection instance status */
typedef enum {
	MAP_MNC_ST_DISCOVERING,	    /* 0 */
	MAP_MNC_ST_CONNECTING,		/* 1 */
	MAP_MNC_ST_CONNECTED,		/* 2 */
	MAP_MNC_ST_SEND_EVENT,		/* 3 */
} tMAPH_MNCStatus;

/* MCE */
/* Local MAS client instance status */
typedef enum {
	MAP_MAC_ST_DISCOVERING,	    /* 0 */
	MAP_MAC_ST_CONNECTING,		/* 1 */
	MAP_MAC_ST_CONNECTED,		/* 2 */
	MAP_MAC_ST_SET_FOLDER,		/* 3 */
	MAP_MAC_ST_GET_OBJECT,		/* 4 */
	MAP_MAC_ST_PUSH_MESSAGE,	/* 5 */
	MAP_MAC_ST_PUT_COMMAND		/* 6 */
} tMAPH_MACStatus;

#define MAX_MAS_RECORD_COUNT	12

/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
typedef struct {
	tMAP_EventCbk	*cbk;			/* callback to handle connection confirmation and link lost indication */
	void			*context;		/* Application defined data structure used to identify the request */
	UINT32			channel_psm;	/* server channel or PSM of the PSE. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM). */
	UINT8			bd[BD_ADDR_LEN];/* Device address of the MAS */
	UINT8           mas_inst_id;	/* MASInstanceID of the MAS service. */	
	UINT8           sup_msg_types;	/* SupportedMessageTypes of the MAS service, bit mask of MAP_SUPPORTED_MSGTYPE_EMAIL, etc.. */	
} tMAPH_ClntStartStru;

typedef struct {
	UINT8           *header_src;    /* Pointer to the data source of header being stuffed to the OBEX packet */
	UINT16          src_size;       /* Size of the header_src */
	UINT16          src_offset;     /* Start of the header_src that has not been stuffed, 0 based */
	UINT8           src_allocated;  /* 1 - header_src shall be freed. 0 - header_src shall not be freed. */
} tMAPH_ComposeHeaderStru;

/* MCE */
/* Context structure for MAP_DiscoverMAS */
typedef struct {
    UINT8  			bd[BD_ADDR_LEN];/* Bluetooth device address of the MAS server */
	UINT16 			conditions;		/* Conditions specifies the features of Service interested, bit masks, e.g. MAP_CONDITION_EMAIL_SUPPORT */
    void  			*context;       /* Application defined context data */
    tMAP_EventCbk	*cbk;  			/* Application defined callback function */

	UINT16 			trans_hdl;		/* Transaction handle used to identify the SDP connection. */
    UINT8 			svc_hdl_count;  /* Total number of service record handles */
    UINT8 			svc_hdl_idx;   	/* Service record handle index of current querying record */
    UINT32 			*svc_hdls;     	/* Array of service record handles */
} tMAPH_DiscoverMASContextStru;

typedef struct {
	void			*mac_handle;	/* Handle identify the connection */
	tMAP_EventCbk	*cbk;			/* callback to handle SetPath confirmation */
	void			*context;		/* Application defined data structure used to identify the request */
	UINT16			is_parent;		/* 1 - parent folder; 0 - root or child folder */
	WCHAR			folder[1];		/* Name of the current folder. Empty string - root or parent; other string - child. Big Endian internally. System order at UI. */
} tMAPH_SetFolderReqStru;

typedef struct {
	void			*mac_handle;	/* Handle identify the connection */
	tMAP_EventCbk	*cbk;			/* callback to handle SetPath confirmation */
	void			*context;		/* Application defined data structure used to identify the request */
	UINT8			cmd_op;			/* Type of the Put Command. One of tMAPH_OPType:: MAP_OP_CMD_XXX */
	UINT8			status;			/* Notification Status if cmd_op is MAP_OP_CMD_SETNOTIFICATION. One of tMAP_OnOffMask. */
									/* The Message Status Indicator and its value if cmd_op is MAP_OP_CMD_SETMESSAGESTATUS. One of tMAP_MessageStatus. */
									/* Ignored if cmd_op is MAP_OP_CMD_UPDATEINBOX. */
	WCHAR			msg_handle[1];	/* Message handle, NULL terminated UNICODE string with 16 hexadecimal digits. Only used when cmd_op is MAP_OP_CMD_SETMESSAGESTATUS. */
} tMAPH_PutCmdReqStru;

typedef struct {
	UINT32			flag;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_MAXLISTCOUNT etc. */
	UINT8			get_op;				/* Type of Get operation; also specify the type of object to get. One of tMAPH_OPType::MAP_OP_GET_XXX */
	/* Request */
	UINT16  		max_count;			/* MaxListCount parameter. It is ignored if MAP_FLAG_MAXLISTCOUNT is not set in the flag */
	UINT16  		list_offset;		/* ListStartOffset parameter. It is ignored if MAP_FLAG_LISTSTARTOFFSET is not set in the flag */
	UINT32  		parameter_mask;     /* ParameterMask parameter, bit mask of MAP_PARMASK_SUBJECT, etc.. It is ignored if MAP_FLAG_PARAMETERMASK is not set in the flag */
	UINT8   		subject_length;     /* SubjectLength parameter. It is ignored if MAP_FLAG_SUBJECTLENGTH is not set in the flag */
	UINT8   		filter_message_type;/* FilterMessageType parameter, bit mask of MAP_FILTER_MSGTYPE_MMS, etc.. It is ignored if MAP_FLAG_FILTER_MESSAGETYPE is not set in the flag */
	UINT8   		filter_read_status; /* FilterReadStatus parameter, one of tMAP_FilterReadStatus. It is ignored if MAP_FLAG_FILTER_READSTATUS is not set in the flag */
	UINT8   		filter_priority;    /* FilterPriority parameter, one of tMAP_FilterPriority. It is ignored if MAP_FLAG_FILTER_PRIORITY is not set in the flag */
	UCHAR   		*filter_period_begin;/* FilterPeriodBegin parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_FILTER_PERIODBEGIN is not set in the flag */
	UCHAR   		*filter_period_end; /* FilterPeriodEnd parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_FILTER_PERIODEND is not set in the flag */
	UCHAR   		*filter_originator; /* FilterOriginator parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_FILTER_ORIGINATOR is not set in the flag */
	UCHAR   		*filter_recipient;  /* FilterRecipient parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_FILTER_RECIPIENT is not set in the flag */
	UINT8   		attachment;		    /* Attachment parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_ATTACHMENT is not set in the flag */
	UINT8   		charset;	        /* Charset parameter, one of tMAP_Charset. It is ignored if MAP_FLAG_CHARSET is not set in the flag */
	UINT8   		fraction_request;   /* FractionRequest parameter, one of tMAP_FractionRequest. It is ignored if MAP_FLAG_FRACTIONREQUEST is not set in the flag */
	/* Response */
	UINT16			list_size;			/* MessageListingSize or FolderListingSize parameter. It is ignored if none of MAP_FLAG_FOLDERLISTINGSIZE and MAP_FLAG_MESSAGELISTINGSIZE are set in the flag */
	UINT8			new_msg;   		    /* NewMessage parameter. It is ignored if MAP_FLAG_NEWMESSAGE is not set in the flag */
	UINT8			fraction_deliver;  	/* FractionDeliver parameter, one of tMAP_FractionDeliver. It is ignored if MAP_FLAG_FRACTIONDELIVER is not set in the flag */
	UCHAR  		    *mse_time;	        /* MSETime parameter, shall be "YYYYMMDDTHHMMSS+(-)hhmm". Shall be freed manually. It is ignored if MAP_FLAG_MSETIME is not set in the flag */
	/* MCE Special */
	void			*mac_handle;		/* Handle identify the connection */
	tMAP_EventCbk	*cbk;				/* callback to handle object data indication and final confirmation */
	void			*context;			/* Application defined data structure used to identify the request */
	WCHAR   		*name;	        	/* Name of the foler for GetMessageListing; Message Handle for GetMessage. */
} tMAPH_GetObjReqStru;

typedef struct {
	UINT32			flag;		        /* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_TRANSPARENT etc. */
	/* Request */
	UINT8   		transparent;		/* Transparent parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_TRANSPARENT is not set in the flag */
	UINT8   		retry;	            /* Charset parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_RETRY is not set in the flag */
	UINT8   		charset;	        /* Charset parameter, one of tMAP_Charset. It is ignored if MAP_FLAG_CHARSET is not set in the flag */
	WCHAR   		*folder;	        /* Charset parameter, one of tMAP_Charset. It is ignored if MAP_FLAG_CHARSET is not set in the flag */
	/* Response */
	WCHAR  		    *msg_handle;	    /* Handle assigned by the MSE to the message that was pushed. Shall not be freed directly. It is valid only when the result is ER_MAP_SUCCESS. */
	/* MCE Special */
	void			*mac_handle;        /* Handle identify the connection */
	tMAP_EventCbk	*cbk;				/* callback to handle object push indications, confirmation and link lost indication */
	void			*context;			/* Application defined data structure used to identify the request */
	struct OBEX_Put_PassiveReqStru *passive_req;    /* Temporary passive request parameter */
} tMAPH_PushMsgReqStru;

typedef struct {
	void			*mac_handle;        /* Handle identify the connection */
	UINT32          data_size;			/* Size of data ready to be sent */
} tMAPH_PutContinueStru;

typedef struct {
	UINT8				bd[BD_ADDR_LEN];/* Device address of the MAS */
	UINT8				status;			/* One of tMAPH_MACStatus */
	UINT8				status_mask;	/* Additional status information, e.g. MAP_CLNT_STM_DISCONNECT */
	tMAP_EventCbk		*conn_cbk;		/* callback to handle connection confirmation and link lost indication */
	void				*context;		/* Application defined data structure used to identify the ClientStart request */
	UINT32				channel_psm;	/* server channel or PSM of the MSE. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM). */
	UINT32				obex_tl_handle;	/* OBEX connection handle */
	UINT8           	mas_inst_id;	/* MASInstanceID of the MAS service. Use to match the MSE sending the notification */	
	UINT16				max_outdata_size;/* Maximum size of data allowed in one Put request */
	union {
	tMAPH_ClntStartStru		*clnt_start_p;
	tMAPH_SetFolderReqStru	*set_path_p;
	tMAPH_GetObjReqStru	    *get_req_p;
	tMAPH_PushMsgReqStru	*put_req_p;
	tMAPH_PutCmdReqStru	    *cmd_req_p;
	}					cur_op;			/* determined by inst_status */
	tMAPH_ComposeHeaderStru cur_header;	/* Header being composed */
} tMAPH_LMACInstStru;

typedef struct {
	UINT8				bd[BD_ADDR_LEN];/* Device address of the peer */
	UINT8				status;			/* One of tMAPH_MNCStatus */
	UINT8				status_mask;	/* Additional status information, e.g. MAP_CLNT_STM_DISCONNECT */
	UINT8				mas_inst_id;	/* MASInstanceID of the MAS service that sending the latest Event-Report-Object. */
	UINT32				obex_tl_handle;	/* OBEX connection handle */
} tMAPH_RMNCInstStru;

typedef struct {
	void				*context;		/* Application defined data structure used to identify the ServerStart request */
	tMAP_EventCbk		*svr_cbk;	    /* callback to handle server indications */
    UINT32              sdp_handle;     /* SDP service record handle of this MAS service. Only used to return value to the application. The value specified by the application is ignored. */
	UINT32				obex_rf_handle;	/* OBEX server handle for RFCOMM */
	UINT32				obex_l2_handle;	/* OBEX server handle for L2CAP - MAP1.2 feature */
	UINT8				status;			/* One of tMAPH_SvcStatus */
	struct ListStru 	rmnc_list;		/* tMAPH_RMNCInstStru */
} tMAPH_MNSInstStru;

/* MSE */
typedef struct {
	void				*context;		/* Application defined data structure used to identify the ServerStart request */
	tMAP_EventCbk		*svr_cbk;	    /* callback to handle server indications */
    UINT32              sdp_handle;     /* SDP service record handle of this MAS service. Only used to return value to the application. The value specified by the application is ignored. */
	UINT32				obex_rf_handle;	/* OBEX server handle for RFCOMM */
	UINT32				obex_l2_handle;	/* OBEX server handle for L2CAP - MAP1.2 feature */
	UINT8				status;			/* One of tMAPH_SvcStatus */
	struct ListStru 	rmac_list;		/* tMAPH_RMACInstStru */
} tMAPH_MASInstStru;

/* Global */
typedef struct {
#ifdef CONFIG_MAP_MCE
	struct ListStru 	lmac_list;		/* tMAPH_LMACInstStru */
	tMAPH_MNSInstStru 	*mns_inst;	    /* Local MNS Service instance */
#endif
} tMAPH_UserDataStru;


/*---------------------------------------------------------------------------
							Globals
----------------------------------------------------------------------------*/
extern struct FsmInst *g_map_fsminst;

#define MAP_CUDATA				((tMAPH_UserDataStru *)(g_map_fsminst->user_data))
#define MAP_LMAC_LIST			(MAP_CUDATA->lmac_list)
#define MAP_LMNS			    (MAP_CUDATA->mns_inst)
#define MAP_RMNC_LIST		    (MAP_CUDATA->mns_inst->rmnc_list)

#define MAP_FREE(m)				{if (m != NULL) FREE(m);}


/*---------------------------------------------------------------------------
							Functions
----------------------------------------------------------------------------*/
/* Common */
void MAP_Fsm_Done(struct FsmInst *fi, UINT8 event, void *arg);
void MAP_Fsm_Disconnect(struct FsmInst *fi, UINT8 event, void **arg);

void MAPH_OBEXConnect(void *obex_cbk, UINT8 *bd, UINT32 channel_psm);
UINT16 MAPH_ConvertOBEXError(UINT16 obex_err);

void MAPH_SendClntStartCfmEv(tMAP_EventCbk *cbk, void *handle, void *context, UINT8 *bd);
void MAPH_SendClntConnectCompleteEv(void *mac_inst, tMAPH_ClntStartStru *clnt_start_arg, UINT16 result);
void MAPH_SendDisconnectCompleteEv(tMAP_EventCbk *cbk, void *handle, void *context, UINT8 *bd, UINT16 reason);

/* MCE */
#ifdef CONFIG_MAP_MCE
/* MAS Client */
void MAC_SearchSAReq(tMAPH_DiscoverMASContextStru *in_context);
void MAC_OpenSearchCfm(tMAPH_DiscoverMASContextStru *in_context, UINT8 *bd, UINT16 status, UINT16 trans_hdl);

void MAC_OBEXCbk(UINT8 msgid, void *arg);
tMAPH_LMACInstStru *MAC_FindInstByBDChannel(void *ignored_item, UINT8 *bd, UINT32 channel_psm);
tMAPH_LMACInstStru *MAC_FindInstByBDInstID(UINT8 *bd, UINT8 mas_inst_id);
void MAC_GetServiceInfoCbk(tMAPH_LMACInstStru *mac_inst, UINT8 *bd, UINT16 result, UINT16 svc_type, struct SDAP_MAPMASInfoStru *sdx_info);
void MAC_InstDone(tMAPH_LMACInstStru *mac_inst, UINT16 reason);

void MAC_ComposeHeaderInd(struct OBEX_ComposeHeaderStru *info);
void MAC_DecodeAppParameters(tMAPH_GetObjReqStru *param, struct BuffStru *pdata);

tMAP_PutContinueFlag MAC_SendAsk4PutReqIndEv(tMAPH_LMACInstStru *mac_inst);
void MAC_SendAsk4ObjDataIndEv(tMAPH_LMACInstStru *mac_inst, struct OBEX_ComposeHeaderStru *info);
void MAC_SendPushMsgCompleteEv(tMAPH_PushMsgReqStru *arg, UINT16 result);
void MAC_SendSetFolderCompleteEv(tMAPH_SetFolderReqStru *arg, UINT16 result);
void MAC_SendCmdCompleteEv(tMAPH_LMACInstStru *mac_inst, tMAPH_PutCmdReqStru *arg, UINT16 result);
tMAP_GetContinueFlag MAC_SendAsk4GetReqIndEv(tMAPH_LMACInstStru *mac_inst);
void MAC_SendObjDataIndEv(tMAPH_LMACInstStru *mac_inst, struct BuffStru *pdata, tMAP_Completeness is_final);
void MAC_SendGetCompleteEv(tMAPH_GetObjReqStru *arg, UINT16 result);

void MAC_OBEXPut(tMAPH_LMACInstStru *mac_inst, struct OBEX_Put_PassiveReqStru *passive_p);
void MAC_OBEXAbort(tMAPH_LMACInstStru *mac_inst);

void MAC_Fsm_ClientStart(struct FsmInst *fi, UINT8 event, tMAPH_ClntStartStru *arg);
void MAC_Fsm_SetFolder(struct FsmInst *fi, UINT8 event, tMAPH_SetFolderReqStru *arg);
void MAC_Fsm_CmdStart(struct FsmInst *fi, UINT8 event, tMAPH_PutCmdReqStru *arg);
void MAC_Fsm_PushMessage(struct FsmInst *fi, UINT8 event, tMAPH_PushMsgReqStru *arg);
void MAC_Fsm_PutContinue(struct FsmInst *fi, UINT8 event, tMAPH_PutContinueStru *arg);
void MAC_Fsm_GetStart(struct FsmInst *fi, UINT8 event, tMAPH_GetObjReqStru *arg);
void MAC_Fsm_GetContinue(struct FsmInst *fi, UINT8 event, void **arg);
void MAC_Fsm_Abort(struct FsmInst *fi, UINT8 event, void **arg);

void MAC_Fsm_OBEXConnectCfm(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectCompleteStru *arg);
void MAC_Fsm_OBEXAuthenInd(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectAuthReqStru *arg);
void MAC_Fsm_OBEXConnectComplete(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectCompleteStru *arg);
void MAC_Fsm_OBEXDisconnectComplete(struct FsmInst *fi, UINT8 event, struct OBEX_DisconnectCompleteStru *arg);
void MAC_Fsm_OBEXSetPathCfm(struct FsmInst *fi, UINT8 event, struct OBEX_SetPathStru *arg);
void MAC_Fsm_OBEXA4PutReq(struct FsmInst *fi, UINT8 event, struct OBEX_Put_PassiveReqStru *arg);
void MAC_Fsm_OBEXA4GetReq(struct FsmInst *fi, UINT8 event, struct OBEX_Get_PassiveReqStru *arg);
void MAC_Fsm_OBEXGetContinue(struct FsmInst *fi, UINT8 event, struct OBEX_Get_CfmStru *arg);
void MAC_Fsm_OBEXOPComplete(struct FsmInst *fi, UINT8 event, struct OBEX_OP_CompleteStru *arg);

/* MNS Service */
void MNS_OBEXCbk(UINT8 msgid, void *arg);
void MNS_MNCInstDone(tMAPH_RMNCInstStru *mnc_inst, UINT16 reason);
void MNS_InstDone(void);
void MNS_SendSvrStartCompleteEv(tMAPH_MNSInstStru *arg, UINT16 result);
void MNS_SendConnectCompleteEv(tMAPH_RMNCInstStru *rmnc_inst, UINT16 result);
void MNS_SendEventReportIndEv(tMAPH_RMNCInstStru *mnc_inst, struct BuffStru *pdata, tMAP_Completeness is_final);

void MNS_Fsm_ServerStart(struct FsmInst *fi, UINT8 event, tMAPH_MNSInstStru *arg);
void MNS_Fsm_ServerStop(struct FsmInst *fi, UINT8 event, void *arg);
void MNS_Fsm_OBEXSvrStartCfm(struct FsmInst *fi, UINT8 event, struct OBEX_ServerCbkStru *arg);
void MNS_Fsm_OBEXA4ConnectRsp(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectIndStru *arg);
void MNS_Fsm_OBEXConnectComplete(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectCompleteStru *arg);
void MMS_Fsm_OBEXDisconnectComplete(struct FsmInst *fi, UINT8 event, struct OBEX_DisconnectCompleteStru *arg);
void MNS_Fsm_OBEXA4GetRsp(struct FsmInst *fi, UINT8 event, struct OBEX_Get_IndStru *arg);
void MNS_Fsm_OBEXA4SetPathRsp(struct FsmInst *fi, UINT8 event, struct OBEX_SetPathStru *arg);
void MNS_Fsm_OBEXA4AbortRsp(struct FsmInst *fi, UINT8 event, struct OBEX_AbortIndStru *arg);
void MNS_Fsm_OBEXPutInd(struct FsmInst *fi, UINT8 event, struct OBEX_Put_IndStru *arg);
#endif

/* MSE */
#ifdef CONFIG_MAP_MSE
#endif

#endif
