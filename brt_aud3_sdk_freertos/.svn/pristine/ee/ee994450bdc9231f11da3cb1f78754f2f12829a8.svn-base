/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	pbap_ui.h
Abstract:
	This file defines the data structure, constants and function declarations
	of API.
Author:
    Y.D.X
Revision History:
	1.	2013.10.31	Created.
------------------------------------------------------------------------*/
#ifndef _PBAP_UI_H
#define _PBAP_UI_H

#include "global.h"
#include "hci_ui.h"
#include "obex_ui.h"
#include "gap_ui.h"
#include "sdap_ui.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* UNICODE identify the parent folder */
static const WCHAR g_pbap_parent_folder[] = {'.', '.', 0};

/* Possible error code group values */
#define PBAP_ER_GROUP_PBAP			  0x0000
#define PBAP_ER_GROUP_HCI			  (ER_TYPE_HCI << 8)
#define PBAP_ER_GROUP_L2CAP			  (ER_TYPE_L2CAP << 8)
#define PBAP_ER_GROUP_RFCOMM		  (ER_TYPE_RFCOMM << 8)
#define PBAP_ER_GROUP_OBEX			  (ER_TYPE_OBEX << 8)

#define PBAP_ER_GROUP_MASK			  0xFE00
#define PBAP_GET_ER_GROUP(_res)		  (_res & PBAP_ER_GROUP_MASK)
		  
/* Possible result codes */
/* PBAP private result code, OBEX standard response code - PBAP_ER_GROUP_PBAP */
typedef enum {
	/* PBAP private result code */
	ER_PBAP_SUCCESS,				/* Success */

	ER_PBAP_REPEATED_CONNECTION,	/* A connection to the same PSE service exists */

	ER_PBAP_SDP_CONNECTION_FAILURE,	/* Fail to create SDP connection */
	ER_PBAP_PSE_NOT_AVAILABLE,		/* Can't find PSE service record or the PSE service is labeled as unavailable */
	ER_PBAP_REMOTE_SDP_TRANSACTION_ERROR,	/* Error occur during SDP transaction */

	ER_PBAP_INVALID_HANDLE,			/* The specified handle is not a valid PBAP instance handle */
	ER_PBAP_NO_CONNECTION,			/* The connection is not ready to execute the operation requested */
	ER_PBAP_LOCAL_BUSY,				/* Another operation is incomplete */
	ER_PBAP_LOCAL_ABORT,			/* Operation aborted by local APP */
	ER_PBAP_LOCAL_CLOSE,			/* The PBAP Stack is being closed. */

    ER_PBAP_TOO_MORE_SERVICES,      /* Local services of specified type have reached the limit. */

	/* OBEX standard response code, only those listed in the PBAP Spec is defined explicitly */
	ER_PBAP_OBEX_RSP_BAD_REQUEST = OBEX_RSP_BAD_REQUEST,
	ER_PBAP_OBEX_RSP_UNAUTHORIZED = OBEX_RSP_UNAUTHORIZED,
	ER_PBAP_OBEX_RSP_FORBIDDEN = OBEX_RSP_FORBIDDEN,
	ER_PBAP_OBEX_RSP_NOT_FOUND = OBEX_RSP_NOT_FOUND,
	ER_PBAP_OBEX_RSP_NOT_ACCEPTABLE = OBEX_RSP_NOT_ACCEPTABLE,
	ER_PBAP_OBEX_RSP_PRECONDITION_FAILED = OBEX_RSP_PRECONDITION_FAILED,
	ER_PBAP_OBEX_RSP_SERVICE_UNAVAILABLE = OBEX_RSP_SERVICE_UNAVAILABLE
	
} tPBAP_PbapResult;
/* 
	OBEX standard response code - PBAP_ER_GROUP_PBAP
	e.g. (PBAP_ER_GROUP_PBAP + OBEX_RSP_BAD_REQUEST)
*/
/* 
	HCI error code - PBAP_ER_GROUP_HCI
	e.g. (PBAP_ER_GROUP_HCI + HCI_STATUS_PAGE_TIMEOUT)
*/
/* 
	L2CAP error code - PBAP_ER_GROUP_L2CAP
	e.g. (PBAP_ER_GROUP_L2CAP + ER_L2CAP_RTXTIMEOUT)
*/
/*
	RFCOMM error code - PBAP_ER_GROUP_RFCOMM
	e.g. (PBAP_ER_GROUP_RFCOMM + ER_RFCOMM_UA_IND_DISC)
*/
/*
	OBEX private error code - PBAP_ER_GROUP_OBEX
	e.g. (PBAP_ER_GROUP_OBEX + ER_OBEX_DISCONNECT_REQ_BEFORE_OBEXCONNECT)
*/


/* Type of event */
typedef enum {
    /* Common */
	PBAP_EVT_CONNECT_COMPLETE,
	PBAP_EVT_DISCONNECT_COMPLETE,

    /* PCE */
	PBAP_EVT_CLIENT_START_CFM,
	PBAP_EVT_CLIENT_SERVICE_INFO,
	PBAP_EVT_AUTHENTICATION_IND,
	PBAP_EVT_SET_PHONEBOOK_COMPLETE,
	PBAP_EVT_OBJECT_DATA_IND,
	PBAP_EVT_ASK4_GET_REQUEST_IND,
	PBAP_EVT_PULL_PHONEBOOK_COMPLETE,
	PBAP_EVT_PULL_VCARDLIST_COMPLETE,
	PBAP_EVT_PULL_VCARD_COMPLETE,

    /* PSE */

} tPBAP_Event_Type;

/* Possible bit mask values of <structure>::flag */
#define PBAP_FLAG_ORDER						0x0001
#define PBAP_FLAG_SEARCHVALUE				0x0002
#define PBAP_FLAG_SEARCHATTRIBUTE			0x0004
#define PBAP_FLAG_MAXLISTCOUNT				0x0008
#define PBAP_FLAG_LISTSTARTOFFSET			0x0010
#define PBAP_FLAG_FILTER					0x0020
#define PBAP_FLAG_FORMAT					0x0040
#define PBAP_FLAG_PHONEBOOKSIZE				0x0080
#define PBAP_FLAG_NEWMISSEDCALLS			0x0100

/* 
	Filter bit mask supported by PBAP1.0. 
	Possible values for parameter 'flag' of PBAP_FilterComposer. 
*/
typedef enum {
	PBAP_FILTER_VERSION,
	PBAP_FILTER_FN,
	PBAP_FILTER_N,
	PBAP_FILTER_PHOTO,
	PBAP_FILTER_BDAY,
	PBAP_FILTER_ADR,
	PBAP_FILTER_LABEL,
	PBAP_FILTER_TEL,
	PBAP_FILTER_EMAIL,
	PBAP_FILTER_MAILER,
	PBAP_FILTER_TZ,
	PBAP_FILTER_GEO,
	PBAP_FILTER_TITLE,
	PBAP_FILTER_ROLE,
	PBAP_FILTER_LOGO,
	PBAP_FILTER_AGENT,
	PBAP_FILTER_ORG,
	PBAP_FILTER_NOTE,
	PBAP_FILTER_REV,
	PBAP_FILTER_SOUND,
	PBAP_FILTER_URL,
	PBAP_FILTER_UID,
	PBAP_FILTER_KEY,
	PBAP_FILTER_NICKNAME,
	PBAP_FILTER_CATEGORIES,
	PBAP_FILTER_PROID,
	PBAP_FILTER_CLASS,
	PBAP_FILTER_SORT_STRING,
	PBAP_FILTER_X_IRMC_CALL_DATETIME,
	PBAP_FILTER_PROPRIETARY_FILTER = 0x27,
	PBAP_FILTER_INVALID = 0xFF
} tPBAP_Filter;

/* Possible values of tPBAP_PullPhoneBookReqStru::format */
typedef enum {
	PBAP_FORMAT_VCARD21,
	PBAP_FORMAT_VCARD30
} tPBAP_Format;

/* Possible values of tPBAP_PullvCardListReqStru::order */
typedef enum {
	PBAP_ORDER_INDEXED,
	PBAP_ORDER_ALPHABETICAL,
	PBAP_ORDER_PHONETICAL
} tPBAP_Order;

/* Possible values of tPBAP_PullvCardListReqStru::search_attribute */
typedef enum {
	PBAP_SCHATTR_NAME,
	PBAP_SCHATTR_NUMBER,
	PBAP_SCHATTR_SOUND
} tPBAP_SearchAttribute;

/* Possible values of tPBAP_ObjectDataStru::is_final */
typedef enum {
	PBAP_DATA_PARTIAL,
	PBAP_DATA_COMPLETE,
	PBAP_DATA_INCOMPLETE
} tPBAP_Completeness;

/* Possible values of tPBAP_Ask4GetReqIndCbk::next_op */
typedef enum {
	PBAP_GET_CONTINUE,
	PBAP_GET_ABORTING,
	PBAP_GET_PENDING
} tPBAP_GetContinueFlag;

/* Possible bit mask values of Digest Challenge options */
#define PBAP_DCOPT_USERID_REQUIRED		1
#define PBAP_DCOPT_READ_ONLY			2

/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
/* Parameter of PBAP_EVT_CONNECT_COMPLETE and PBAP_EVT_DISCONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  result;             /* Connection result */
	UINT32  channel_psm;        /* server channel or PSM of the PSE connected. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM) */
} tPBAP_ConnectCompleteStru;

/* Application Parameters header in the PullPhoneBook request */
typedef struct {
	UINT8   flag;				/* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT8	filter[8];			/* Filter parameter. It is ignored if PBAP_FLAG_FILTER is not set in the flag */
	UINT8	format;				/* Format parameter, one of tPBAP_Format. It is ignored if PBAP_FLAG_FORMAT is not set in the flag */
	UINT16  max_list_count;		/* MaxListCount parameter. It is ignored if PBAP_FLAG_MAXLISTCOUNT is not set in the flag */
	UINT16  list_start_offset;	/* ListStartOffset parameter. It is ignored if PBAP_FLAG_LISTSTARTOFFSET is not set in the flag */
} tPBAP_PullPhoneBookReqStru;

/* Application Parameters header in the PullvCardListing request */
typedef struct {
	UINT8   flag;				/* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_SEARCHATTRIBUTE etc. */
	UINT8	order;				/* Order parameter, one of tPBAP_Order. It is ignored if PBAP_FLAG_ORDER is not set in the flag */
	UINT8	search_attribute;	/* SearchAttribute parameter, one of tPBAP_SearchAttribute. It is ignored if PBAP_FLAG_SEARCHATTRIBUTE is not set in the flag */
	UCHAR	*search_value;		/* SearchValue parameter, NULL terminated UTF-8 string. It is ignored if PBAP_FLAG_SEARCHVALUE is not set in the flag */
	UINT16  max_list_count;		/* MaxListCount parameter. It is ignored if PBAP_FLAG_MAXLISTCOUNT is not set in the flag */
	UINT16  list_start_offset;	/* ListStartOffset parameter. It is ignored if PBAP_FLAG_LISTSTARTOFFSET is not set in the flag */
} tPBAP_PullvCardListReqStru;

/* Application Parameters header in the PullvCardEntry request */
typedef struct {
	UINT8   flag;				/* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT8	filter[8];			/* Filter parameter. It is ignored if PBAP_FLAG_FILTER is not set in the flag */
	UINT8	format;				/* Format parameter, one of tPBAP_Format. It is ignored if PBAP_FLAG_FORMAT is not set in the flag */
} tPBAP_PullvCardReqStru;

/* PCE */
/* Parameter of PBAP_EVT_AUTHENTICATION_IND */
typedef struct {
	UINT8	bd[BD_ADDR_LEN];	/* Bluetooth device address of the PSE */
	UINT8	options;			/* Options field of Digest Challenge, combinations of PBAP_DCOPT_xxxx */
	UINT8	*realm;				/* Realm field of Digest Challenge. NULL if not present */ 	 
} tPBAP_AuthenIndStru;

/* Parameter of PBAP_EVT_PULL_PHONEBOOK_COMPLETE and PBAP_EVT_PULL_VCARDLIST_COMPLETE */
typedef struct {
	UINT16  result;
	UINT16  flag;				/* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_PHONEBOOKSIZE etc. */
	UINT16  pb_size;			/* PhoneBookSize parameter. It is ignored if PBAP_FLAG_PHONEBOOKSIZE is not set in the flag */
	UINT8	new_missed_calls;	/* NewMissedCalls parameter. It is ignored if PBAP_FLAG_NEWMISSEDCALLS is not set in the flag */
} tPBAP_PullPhoneBookCompleteStru, tPBAP_PullvCardListCompleteStru;

/* Parameter of PBAP_EVT_OBJECT_DATA_IND */
typedef struct {	
	UINT8				*data;				/* Data chunk of the Object. It is NULL if the Get operation is terminated abnormally. */
	UINT16				length;				/* Size, in bytes, of the data. It is 0 if the Get operation is terminated abnormally. */
	tPBAP_Completeness	is_final;			/* PBAP_DATA_PARTIAL - More data to be received. PBAP_DATA_COMPLETE - It is the final chunk.
											   PBAP_DATA_INCOMPLETE - The object transfer is aborted by the client and no more data will be received. */
} tPBAP_ObjectDataStru;

/* PSE */

/*---------------------------------------------------------------------------
							Callback Prototype
----------------------------------------------------------------------------*/
/* 
Description:
    General callback function to handle confirmations or indications.
Parameters:
    handle    [in] Handle identify the local PCE or PSE entity reporting the
				   event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] The event type.
    param     [in/out] Additional event parameter. Refer to the following
                   specific event callback definition. It is set to NULL if
                   not specified additionally.
*/
typedef void (tPBAP_EventCbk)(void *handle, void *context, tPBAP_Event_Type event, void *param);

/* 
Description:
    Callback function to handle confirmation to the ClientStart request.
	This confirmation is used to return the lower handle identifying the
	PCE entity. And the APP can then use this handle to cancel the connection
	creation process.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the
				   event. It is also identify the new connection to create.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart request.
    event     [in] Shall be PBAP_EVT_CLIENT_START_CFM.
    bd		  [in] Bluetooth device address of the remote PSE to connect.
*/
typedef void (tPBAP_ClientStartCfmCbk)(void *handle, void *context, tPBAP_Event_Type event, UINT8 *bd);

/* 
Description:
    Callback function to handle OBEX authentication initiated by the PSE.
	Call PBAP_AuthenticationRsp to respond with the proper password.
	NOTE: Only the challenge to the Connection is supported.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the
				   event.
	context	  [in] Application defined data structure used to identify
				   the request being challenged.
    event     [in] Shall be PBAP_EVT_AUTHENTICATION_IND.
    param	  [in] Digest Challenge information.
*/
typedef void (tPBAP_AuthenIndCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_AuthenIndStru *param);

/* 
Description:
    Callback function to handle confirmation to the SetPhoneBook request.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be PBAP_EVT_SET_PHONEBOOK_COMPLETE.
    result    [in] Result of the SetPhonebook operation.
*/
typedef void (tPBAP_SetPhoneBookCompleteCbk)(void *handle, void *context, tPBAP_Event_Type event, UINT16 *result);

/* 
Description:
    Callback function to handle indication of the object content.
	The object can be a	phone book, a vCard-listing	or a vCard. 
	During the Get operation, the APP may receive this indication more than once.
	If the PSE rejects the Get request, the APP may not receive this indication.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be PBAP_EVT_OBJECT_DATA_IND.
    param     [in] Chunk of the object content.
			  
*/
typedef void (tPBAP_ObjectDataIndCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_ObjectDataStru *param);

/* 
Description:
    Callback function to handle indication asking for next Get request.
	This indication allows the application to pend the pulling operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be PBAP_EVT_ASK4_GET_REQUEST_IND.
    next_op   [in/out] On input, it is set to PBAP_GET_CONTINUE. On output,
                   1) Keep it unchanged, the PBAP Stack sends Get request to the
                   PSE immediately after this callback returns.
                   2) Change it to PBAP_GET_ABORTING, the PBAP Stack sends Abort
                   request to the PSE immediately after this callback returns. The
                   pulling operation is then terminated.
                   3) Change it to PBAP_GET_PENDING, the PBAP Stack waits for the
                   application to call PBAP_GetContinueReq to send Get request or
                   to call PBAP_CancelPullObjectReq to terminate the pulling operation.
			  
*/
typedef void (tPBAP_Ask4GetReqIndCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_GetContinueFlag *next_op);

/* 
Description:
    Callback function to handle confirmation to the PullPhoneBook request.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be PBAP_EVT_PULL_PHONEBOOK_COMPLETE.
    param     [in] Result of the PullPhonebook operation.
*/
typedef void (tPBAP_PullPhoneBookCompleteCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_PullPhoneBookCompleteStru *param);

/* 
Description:
    Callback function to handle confirmation to the PullvCardListing request.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be PBAP_EVT_PULL_VCARDLIST_COMPLETE.
    param     [in] Result of the PullvCardListing operation.
*/
typedef void (tPBAP_PullvCardListCompleteCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_PullvCardListCompleteStru *param);

/* 
Description:
    Callback function to handle confirmation to the PullvCardEntry request.
Parameters:
    handle    [in] Handle identify the local PCE entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be PBAP_EVT_PULL_VCARD_COMPLETE.
    result    [in] Result of the PullvCardEntry operation.
*/
typedef void (tPBAP_PullvCardCompleteCbk)(void *handle, void *context, tPBAP_Event_Type event, UINT16 *result);

/* 
Description:
    Callback function to handle the result of connection creation process.
    If local device acts as PSE, this event is created only when the
    connection initiated by the remote PCE is created successfully.
    If local device acts as PCE, this event is created for both success and
    failure situations.
Parameters:
    handle    [in] Handle identify the local PCE or PSE entity reporting the
				   event. It is also identify the new connection created.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be PBAP_EVT_CONNECT_COMPLETE.
    param     [in] Result of the connection creation process.
*/
typedef void (tPBAP_ConnectCompleteCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_ConnectCompleteStru *param);

/* 
Description:
    Callback function to handle disconnect complete event.
Parameters:
    handle    [in] Handle identify the local PCE or PSE entity reporting the
                   event.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be PBAP_EVT_DISCONNECT_COMPLETE.
    param     [in] Information of the connection disconnected.
*/
typedef void (tPBAP_DisconnectCompleteCbk)(void *handle, void *context, tPBAP_Event_Type event, tPBAP_ConnectCompleteStru *param);


/*---------------------------------------------------------------------------
							Interfaces
----------------------------------------------------------------------------*/
/* Common */
void PBAP_Init(void);
void PBAP_Done(void);
void PBAP_DisconnectReq(void *con_handle);

/* PCE */
#ifdef CONFIG_PBAP_CLIENT
void PBAP_ClientStartReq(void *context, UINT8 *bd, UINT32 channel_psm, UINT8 *password, tPBAP_EventCbk *cbk);
#define PBAP_ClientStopReq  PBAP_DisconnectReq

void PBAP_SetPhoneBookReq(void *pce_handle, void *context, WCHAR *folder, tPBAP_SetPhoneBookCompleteCbk *cbk);
/* Set root folder */
#define PBAP_SetRootFolderReq(_hdl, _cont, _cbk)	PBAP_SetPhoneBookReq(_hdl, _cont, NULL, _cbk)
/* Set parent folder */
#define PBAP_SetParentFolderReq(_hdl, _cont, _cbk)	PBAP_SetPhoneBookReq(_hdl, _cont, (WCHAR *)g_pbap_parent_folder, _cbk)
/* Set child folder */
#define PBAP_SetChildFolderReq				PBAP_SetPhoneBookReq

void PBAP_PullPhoneBookReq(void *pce_handle, void *context, WCHAR *path, tPBAP_PullPhoneBookReqStru *req_param, tPBAP_EventCbk *cbk);
void PBAP_PullvCardListReq(void *pce_handle, void *context, WCHAR *folder, tPBAP_PullvCardListReqStru *req_param, tPBAP_EventCbk *cbk);
void PBAP_PullvCardReq(void *pce_handle, void *context, WCHAR *name, tPBAP_PullvCardReqStru *req_param, tPBAP_EventCbk *cbk);
void PBAP_GetContinueReq(void *pce_handle);
void PBAP_AbortPullObjectReq(void *pce_handle);
void PBAP_AuthenticationRsp(void *pce_handle, UCHAR *user_id, UCHAR *password);

void PBAP_FilterComposer(UINT8 filter[8], tPBAP_Filter flag);
#endif

/* PSE */
#ifdef CONFIG_PBAP_SERVER
#endif

#endif
