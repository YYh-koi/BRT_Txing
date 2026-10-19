/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	opp_ui.h
Abstract:
	This file defines the data structure, constants and function declarations
	of API.
Author:
    Y.D.X
Revision History:
	1.	2014.05.04	Created.
------------------------------------------------------------------------*/
#ifndef _OPP_UI_H
#define _OPP_UI_H

#include "global.h"
#include "hci_ui.h"
#include "obex_ui.h"
#include "gap_ui.h"
#include "sdap_ui.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* Possible error code group values */
#define OPP_ER_GROUP_OPP			  0x0000
#define OPP_ER_GROUP_HCI			  (ER_TYPE_HCI << 8)
#define OPP_ER_GROUP_L2CAP			  (ER_TYPE_L2CAP << 8)
#define OPP_ER_GROUP_RFCOMM		  	  (ER_TYPE_RFCOMM << 8)
#define OPP_ER_GROUP_OBEX			  (ER_TYPE_OBEX << 8)

#define OPP_ER_GROUP_MASK			  0xFE00
#define OPP_GET_ER_GROUP(_res)		  (_res & OPP_ER_GROUP_MASK)
		  
/* Possible result codes */
/* OPP private result code, OBEX standard response code - OPP_ER_GROUP_OPP */
typedef enum {
	/* OPP private result code */
	ER_OPP_SUCCESS,				/* Success */

	ER_OPP_REPEATED_CONNECTION,	/* A connection to the same OPP service exists */

	ER_OPP_SDP_CONNECTION_FAILURE,	/* Fail to create SDP connection */
	ER_OPP_SERVICE_NOT_AVAILABLE,	/* Can't find OPP service record or the OPP service is labeled as unavailable */
	ER_OPP_REMOTE_SDP_TRANSACTION_ERROR,	/* Error occur during SDP transaction */

	ER_OPP_INVALID_HANDLE,			/* The specified handle is not a valid OPP instance handle */
	ER_OPP_NO_CONNECTION,			/* The connection is not ready to execute the operation requested */
	ER_OPP_LOCAL_BUSY,				/* Another operation is incomplete */
	ER_OPP_LOCAL_ABORT,				/* Operation aborted by local APP */
	ER_OPP_LOCAL_CLOSE,				/* The OPP Stack is being closed. */

    ER_OPP_TOO_MORE_SERVICES,       /* Local services of specified type have reached the limit. */

	/* OBEX standard response code, only those often used is defined explicitly */
	ER_OPP_OBEX_RSP_BAD_REQUEST = OBEX_RSP_BAD_REQUEST,
	ER_OPP_OBEX_RSP_UNAUTHORIZED = OBEX_RSP_UNAUTHORIZED,
	ER_OPP_OBEX_RSP_FORBIDDEN = OBEX_RSP_FORBIDDEN,
	ER_OPP_OBEX_RSP_NOT_FOUND = OBEX_RSP_NOT_FOUND,
	ER_OPP_OBEX_RSP_NOT_ACCEPTABLE = OBEX_RSP_NOT_ACCEPTABLE,
	ER_OPP_OBEX_RSP_PRECONDITION_FAILED = OBEX_RSP_PRECONDITION_FAILED,
	ER_OPP_OBEX_RSP_REQUESTED_ENTITY_TOOLARGE = OBEX_RSP_REQUESTED_ENTITY_TOOLARGE,
	ER_OPP_OBEX_RSP_INTERNAL_SERVER_ERROR = OBEX_RSP_INTERNAL_SERVER_ERROR,
	ER_OPP_OBEX_RSP_NOT_IMPLEMENTED = OBEX_RSP_NOT_IMPLEMENTED,
	ER_OPP_OBEX_RSP_SERVICE_UNAVAILABLE = OBEX_RSP_SERVICE_UNAVAILABLE
	
} tOPP_OPPResult;
/* 
	OBEX standard response code - OPP_ER_GROUP_OPP
	e.g. (OPP_ER_GROUP_OPP + OBEX_RSP_BAD_REQUEST)
*/
/* 
	HCI error code - OPP_ER_GROUP_HCI
	e.g. (OPP_ER_GROUP_HCI + HCI_STATUS_PAGE_TIMEOUT)
*/
/* 
	L2CAP error code - OPP_ER_GROUP_L2CAP
	e.g. (OPP_ER_GROUP_L2CAP + ER_L2CAP_RTXTIMEOUT)
*/
/*
	RFCOMM error code - OPP_ER_GROUP_RFCOMM
	e.g. (OPP_ER_GROUP_RFCOMM + ER_RFCOMM_UA_IND_DISC)
*/
/*
	OBEX private error code - OPP_ER_GROUP_OBEX
	e.g. (OPP_ER_GROUP_OBEX + ER_OBEX_DISCONNECT_REQ_BEFORE_OBEXCONNECT)
*/


/* Type of event */
typedef enum {
    /* Common */
	OPP_EVT_CONNECT_COMPLETE,
	OPP_EVT_DISCONNECT_COMPLETE,

	OPP_EVT_ASK4_OBJECT_DATA_IND,
	OPP_EVT_OBJECT_DATA_IND,

    /* Client */
	OPP_EVT_CLIENT_START_CFM,

	/*OPP_EVT_OBJECT_DATA_IND*/
	OPP_EVT_ASK4_GET_REQUEST_IND,
	OPP_EVT_PULL_VCARD_COMPLETE,

	OPP_EVT_ASK4_PUT_REQUEST_IND,
	/*OPP_EVT_ASK4_OBJECT_DATA_IND*/
    OPP_EVT_PUSH_OBJECT_COMPLETE,

    /* Server */
	OPP_EVT_SERVER_START_COMPLETE,
	OPP_EVT_SERVER_STOP_COMPLETE,

	OPP_EVT_CREATE_OBJECT_IND,
	OPP_EVT_ASK4_PUT_RESPONSE_IND,
	/*OPP_EVT_OBJECT_DATA_IND*/
	/*OPP_EVT_PUSH_OBJECT_COMPLETE*/

	OPP_EVT_OPEN_DEFAULT_VCARD_IND,
	OPP_EVT_ASK4_GET_RESPONSE_IND
	/*OPP_EVT_ASK4_OBJECT_DATA_IND*/
	/*OPP_EVT_PULL_VCARD_COMPLETE*/


} tOPP_Event_Type;

/* Possible values of tOPP_ObjectDataStru::is_final */
typedef enum {
	OPP_DATA_PARTIAL,
	OPP_DATA_COMPLETE,
	OPP_DATA_INCOMPLETE
} tOPP_Completeness;

/* Possible values of tOPP_Ask4GetReqIndCbk::next_op */
typedef enum {
	OPP_OP_CONTINUE,
	OPP_OP_ABORTING,
	OPP_OP_PENDING
} tOPP_GetContinueFlag, tOPP_PutContinueFlag;

/* Possible values of OPP_PutResponse::rsp_needed */
typedef enum {
	OPP_SRMP_WAIT_NOT_REQUIRED,
	OPP_SRMP_WAIT_REQUIRED
} tOPP_SRMPWaitFlag;

/* Possible bit mask values of <structure>::supported_formats */
#define OPP_SUPPORTED_FORMAT_VCARD21			0x0001
#define OPP_SUPPORTED_FORMAT_VCARD30			0x0002
#define OPP_SUPPORTED_FORMAT_VCAL10			    0x0004
#define OPP_SUPPORTED_FORMAT_ICAL20			    0x0008
#define OPP_SUPPORTED_FORMAT_VNOTE			    0x0010
#define OPP_SUPPORTED_FORMAT_VMESSAGE		    0x0020
#define OPP_SUPPORTED_FORMAT_ANYTYPE			0x8000

/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
typedef struct {
	UINT16  channel;    		/* server channel of the OPP service. 0 means unspecifed. */
	UINT16  psm;        		/* dynamic PSM of the OPP service. 0 means unspecified. */
	UINT32  supported_formats;	/* SupportedFormatsList of the OPP service connected, bit mask of OPP_SUPPORTED_FORMAT_VCARD21, etc.. */
	UINT8	*service_name;		/* Service Name, NULL terminated UTF-8 string */
} tOPP_ServerAttributesStru;

/* Parameter of OPP_EVT_CONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  result;             /* Connection result */
	/* The following values shall be ignored in case of connection failure */
	void    *connection_handle; /* Handle identifies the new connection entity. It is usually used by the local service to identify the remote client if it allows multiple connections. */
	UINT32  channel_psm;        /* server channel or PSM of the OPP service connected. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM) */
	UINT32  supported_formats;  /* SupportedFormatsList of the OPP service connected, bit mask of OPP_SUPPORTED_FORMAT_VCARD21, etc.. */
} tOPP_ConnectCompleteStru;

/* Parameter of OPP_EVT_DISCONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  reason;             /* Reason of disconnection */
} tOPP_DisconnectCompleteStru;

typedef struct {
	WCHAR  *name;    		/* Name of the object. Null-terminated UNICODE string. It shall not be NULL. */
	UCHAR  *type;        	/* MIME type of the object. Null-terminated ASCII string. It may be NULL. */
	BtUINT64Stru length;	/* Total length in bytes of the object. 0 may be either 0 bytes or unknown length, so it shall be ignored. */
} tOPP_ObjectInformationStru;

/* Client */
/* Parameter of OPP_EVT_OBJECT_DATA_IND and OPP_EVT_ASK4_OBJECT_DATA_IND */
typedef struct {	
	UINT8				*data;				/* Data chunk of the Object. It is NULL if the Get operation is terminated abnormally. */
	UINT16				length;				/* Size, in bytes, of the data. It is 0 if the Get operation is terminated abnormally. */
	tOPP_Completeness	is_final;			/* OPP_DATA_PARTIAL - More data to be received. OPP_DATA_COMPLETE - It is the final chunk.
											   OPP_DATA_INCOMPLETE - The object transfer is aborted by the client and no more data will be received. */
} tOPP_ObjectDataStru;

/* Parameter of OPP_EVT_ASK4_PUT_REQUEST_IND */
typedef struct {
	tOPP_PutContinueFlag  next_op;          /* Pend, reject or continue the Put operation */
	UINT32                max_data_size;    /* MAX data size expected or available  */
} tOPP_Ask4PutRequestStru;

/* Server */
/* Parameter of OPP_EVT_SERVER_START_COMPLETE */
typedef struct {
	UINT16  result;             /* Server Start result */
	/* The following values shall be ignored in case of a failure */
    UINT32  sdp_record_handle;  /* SDP service record handle of this service. */
	UINT16  channel;    		/* server channel of the service. */
	UINT16  psm;        		/* dynamic PSM of the service. */
	UINT32  supported_formats;	/* SupportedFormatsList of the OPP service connected, bit mask of OPP_SUPPORTED_FORMAT_VCARD21, etc.. */
} tOPP_ServerStartCompleteStru;

/* Parameter of OPP_EVT_ASK4_GET_RESPONSE_IND */
typedef struct {
	tOPP_GetContinueFlag  next_op;          /* Pend, reject or continue the Get operation */
	UINT32                max_data_size;    /* MAX data size expected or available  */
} tOPP_Ask4GetResponseStru;


/*---------------------------------------------------------------------------
							Callback Prototype
----------------------------------------------------------------------------*/
/*
Description:
    General callback function to handle confirmations or indications.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] The event type.
    param     [in/out] Additional event parameter. Refer to the following
                   specific event callback definition. It is set to NULL if
                   not specified additionally.
*/
typedef void (tOPP_EventCbk)(void *handle, void *context, tOPP_Event_Type event, void *param);

/*
Description:
    Callback function to handle the result of connection creation process.
    If local device acts as a server, this event is created only when the
    connection initiated by the remote client is created successfully.
    If local device acts as a client, this event is created for both success
    and failure situations.
Parameters:
    handle    [in] Handle identify the local entity reporting the event.
				   If local acts as a client, it is handle to the new connection
				   created.
				   If local acts as a server, it is handle to the service being
				   connected, and the connection handle is returned by the param.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be OPP_EVT_CONNECT_COMPLETE.
    param     [in] Result of the connection creation process.
*/
typedef void (tOPP_ConnectCompleteCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_ConnectCompleteStru *param);

/*
Description:
    Callback function to handle disconnect complete event.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be OPP_EVT_DISCONNECT_COMPLETE.
    param     [in] Information of the connection disconnected.
*/
typedef void (tOPP_DisconnectCompleteCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_DisconnectCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the ClientStart request.
	This confirmation is used to return the lower handle identifying the
	OPP connection entity. And the APP can then use this handle to cancel
	the connection creation process.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the
				   event. It is also identify the new connection to create.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart request.
    event     [in] Shall be OPP_EVT_CLIENT_START_CFM.
    bd		  [in] Bluetooth device address of the remote OPP server to connect.
*/
typedef void (tOPP_ClientStartCfmCbk)(void *handle, void *context, tOPP_Event_Type event, UINT8 *bd);

/*
Description:
    Callback function to handle indication of the vCard object content.

	During the Get operation, the client APP may receive this indication more
	than once. If the remote server rejects the Get request, the APP may not
	receive this indication.

    During the Put operation. the server APP may receive this indication more
	than once. If the server APP rejects the Put request, it may not receive
	this indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be OPP_EVT_OBJECT_DATA_IND.
    param     [in] Chunk of the object content.
			  
*/
typedef void (tOPP_ObjectDataIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_ObjectDataStru *param);

/*
Description:
    Callback function to handle indication asking for next Get request.
	This indication allows the application to pend the pulling operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be OPP_EVT_ASK4_GET_REQUEST_IND.
    next_op   [in/out] On input, it is set to OPP_OP_CONTINUE. On output,
                   1) Keep it unchanged, the OPP Stack sends Get request to the
                   server immediately after this callback returns.
                   2) Change it to OPP_OP_ABORTING, the OPP Stack sends Abort
                   request to the server immediately after this callback returns. 
                   The pulling operation is then terminated.
                   3) Change it to OPP_OP_PENDING, the OPP Stack waits for the
                   application to call OPP_GetContinueReq to send Get request or
                   to call OPP_AbortReq to terminate the pulling operation.
*/
typedef void (tOPP_Ask4GetReqIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_GetContinueFlag *next_op);

/*
Description:
    Callback function to handle confirmation to the BussinessCardPull request.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be OPP_EVT_PULL_VCARD_COMPLETE.
    result    [in] Result of the BussinessCardPull operation.
*/
typedef void (tOPP_PullvCardCompleteCbk)(void *handle, void *context, tOPP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle indication asking for next Put request.

	This indication allows the application to pend the sending operation so that
	it can prepare the object data in another task.

 	In platforms that reading from local storage with a lower speed, using a
 	separate task for reading may help to improve the performace of internal
 	schedule task of BRT's Bluetooth Stack.

	If the reading operation is quick enough, the application can just ignore
	this indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request. It also identify the message object being pushed.
    event     [in] Shall be OPP_EVT_ASK4_PUT_REQUEST_IND.
    param     [in/out]
                   param->next_op:
                       On input it is set to OPP_OP_CONTINUE.
                       On output,
                       1) Keep it unchanged, the OPP Stack asks for the message
                       object data using OPP_EVT_ASK4_MESSAGE_DATA_IND event and
                       then sends Put request to the server immediately after this
                       callback returns.
                       2) Change it to OPP_OP_ABORTING, the OPP Stack sends Abort
                       request to the server immediately after this callback returns.
                       The pushing operation is then terminated.
                       3) Change it to OPP_OP_PENDING, the OPP Stack waits for the
                       application to prepare the data to be pushed. The application
                       shall then call OPP_PutContinueReq to notify the OPP Stack
                       when the data is ready to send, or OPP_AbortReq
                       to command the OPP Stack to terminate the pushing operation.

                   param->max_data_size:
                       On input it is set to the MAX size of object content that
                       could be sent in the following Put request. The value is
                       for reference only. The application can ignore it.
                       On output,
                       1) If param->next_op is kept OPP_OP_CONTINUE, the value
                       of max_data_size shall specify the size of object content
                       ready for the OPP Stack to send. The OPP Stack will then
                       try to send as much of these data as possible until the
                       OBEX MTU limit is reached.
                       The value 0 specifies that no more content to read and the
                       Final Put request will be sent.
                       2) If param->next_op is changed to either OPP_OP_ABORT or
                       OPP_OP_PENDING, the param->max_data_size value is ignored.
*/
typedef void (tOPP_Ask4PutReqIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_Ask4PutRequestStru *param);

/*
Description:
    Callback function to get object content from the application.

    Each time after getting possitive confirmation to the OPP_EVT_ASK4_PUT_REQUEST_IND
    from the client APP, the OPP Stack tries to require and push as much data as
    possible until the OBEX MTU limit is reached, or the data ready for this Put
    request have all be sent, or the param->is_final is set to OPP_DATA_COMPLETE.
	If the remote server rejects the Put request, the APP may not receive this
	indication.

    Each time after getting possitive confirmation to the OPP_EVT_ASK4_GET_RESPONSE_IND
    from the server APP, the OPP Stack tries to require and push as much data as
    possible until the OBEX MTU limit is reached, or the data ready for this Get
    response have all be sent, or the param->is_final is set to OPP_DATA_COMPLETE.
	If the server APP rejects the Get request, it may not receive this indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request. It also identify the message object to read.
    event     [in] Shall be OPP_EVT_ASK4_OBJECT_DATA_IND.
    param     [in/out]Chunk of the object content.
                   param->data: Specifies the buffer to return the object content.
                   param->length: On input, it specifies the size of param->data.
                   On output, it shall be changed to the size of object content,
                   in bytes, copied to param->data. 0 bytes copied means no more
                   content to read.
                   param->is_final: On input, it is set to OPP_DATA_PARTIAL. On
                   output, is shall kept OPP_DATA_PARTIAL until the final chunk.
			  
*/
typedef void (tOPP_Ask4ObjectDataIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_ObjectDataStru *param);

/*
Description:
    Callback function to handle confirmation to the ObjectPush request.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be OPP_EVT_PUSH_OBJECT_COMPLETE.
    result    [in] Result of the ObjectPush operation.
*/
typedef void (tOPP_PushObjectCompleteCbk)(void *handle, void *context, tOPP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle the result of OPP service initiation.
Parameters:
    handle    [in] Handle identify the local entity reporting the event.
				   It is also identify the new service created.
				   It shall be NULL if the service is failed to initiate.
	context	  [in] Application defined data structure used to identify
				   the ServerStart request.
    event     [in] Shall be OPP_EVT_SERVER_START_COMPLETE.
    param     [in] Result of service initiation.
*/
typedef void (tOPP_ServerStartCompleteCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_ServerStartCompleteStru *param);

/*
Description:
    Callback function to handle the result of deregistering OPP service.
Parameters:
    handle    [in] Handle identify the service being deregistered.
	context	  [in] Application defined data structure used to identify
				   the ServerStart request.
    event     [in] Shall be OPP_EVT_SERVER_STOP_COMPLETE.
    result    [in] Result of service initiation.
*/
typedef void (tOPP_ServerStopCompleteCbk)(void *handle, void *context, tOPP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle indication asking for saving the object pushed by
    the remote client.

	This indication allows the application to determine whether to save the
	object or not.

	The application shall call OPP_PutResponse to reply to this indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be OPP_EVT_CREATE_OBJECT_IND.
    param     [in] Contains the information of the object to save.
*/
typedef void (tOPP_CreateObjIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_ObjectInformationStru *param);

/*
Description:
    Callback function to handle indication asking for Put response.

	This indication allows the application to pend the pushing operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.

	Once the client is allowed to send Put requests without waiting for the
	response, the application won't get OPP_EVT_ASK4_PUT_RESPONSE_IND indication
	for this Put operation any more.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be OPP_EVT_ASK4_PUT_RESPONSE_IND.
    next_op   [in/out] On input, it is set to OPP_OP_CONTINUE. On output,
                   1) Keep it unchanged, the OPP Stack sends Put response to the
                   client immediately after this callback returns. This also
                   instructs the client to send Put requests without waiting for
                   the response if Single Response Mode is enabled.
                   2) Change it to OPP_OP_ABORTING, the OPP Stack sends negative
                   response to the client immediately after this callback returns. 
                   The pushing operation is then terminated.
                   3) Change it to OPP_OP_PENDING, the OPP Stack waits for the
                   application to call OPP_PutResponse to send Put response.
*/
typedef void (tOPP_Ask4PutRspIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_PutContinueFlag *next_op);

/*
Description:
    Callback function to handle indication asking for preparing the business
    card to send to the client.

	The application shall call OPP_PutResponse to reply to this indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be OPP_EVT_OPEN_DEFAULT_VCARD_IND.
    param     [in] Shall be NULL.
*/
typedef void (tOPP_OpenvCardIndCbk)(void *handle, void *context, tOPP_Event_Type event, void *param);

/*
Description:
    Callback function to handle indication asking for Get response.

	This indication allows the application to pend the pulling operation so that
	it can prepare the object data in another task.

 	In platforms that reading from local storage with a lower speed, using a
 	separate task for reading may help to improve the performace of internal
 	schedule task of BRT's Bluetooth Stack.

	If the reading operation is quick enough, the application can just ignore
	this indication.
Parameters:
    handle    [in] Handle identify the local OPP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback. It also identify the message object being pulled.
    event     [in] Shall be OPP_EVT_ASK4_GET_RESPONSE_IND.
    param     [in/out]
                   param->next_op:
                       On input it is set to OPP_OP_CONTINUE.
                       On output,
                       1) Keep it unchanged, the OPP Stack asks for the message
                       object data using OPP_EVT_ASK4_MESSAGE_DATA_IND event and
                       then sends Get response to the client immediately after this
                       callback returns.
                       2) Change it to OPP_OP_ABORTING, the OPP Stack sends negative
                       response to the client immediately after this callback returns.
                       The pulling operation is then terminated.
                       3) Change it to OPP_OP_PENDING, the OPP Stack waits for the
                       application to prepare the data to be pushed. The application
                       shall then call OPP_GetResponse to notify the OPP Stack
                       when the data is ready to send, or to command the OPP Stack
                       to terminate the pushing operation.

                   param->max_data_size:
                       On input it is set to the MAX size of object content that
                       could be sent in the following Get response. The value is
                       for reference only. The application can ignore it.
                       On output,
                       1) If param->next_op is kept OPP_OP_CONTINUE, the value
                       of max_data_size shall specify the size of object content
                       ready for the OPP Stack to send. The OPP Stack will then
                       try to send as much of these data as possible until the
                       OBEX MTU limit is reached.
                       The value 0 specifies that no more content to read and the
                       Final Get response will be sent.
                       2) If param->next_op is changed to either OPP_OP_ABORT or
                       OPP_OP_PENDING, the param->max_data_size value is ignored.
*/
typedef void (tOPP_Ask4GetRspIndCbk)(void *handle, void *context, tOPP_Event_Type event, tOPP_Ask4GetResponseStru *param);


/*---------------------------------------------------------------------------
							Interfaces
----------------------------------------------------------------------------*/
/* Common */
void OPP_Init(void);
void OPP_Done(void);
void OPP_DisconnectReq(void *con_handle);

/* Client */
#ifdef CONFIG_OPP_CLIENT
void OPP_ClientStartReq(void *context, UINT8 *bd, tOPP_ServerAttributesStru *svr_info, tOPP_EventCbk *cbk);
void OPP_PushObjectReq(void *cli_handle, void *context, tOPP_ObjectInformationStru *obj_info, tOPP_EventCbk *cbk);
void OPP_PutContinueReq(void *cli_handle, UINT32 data_size);
void OPP_PullvCardReq(void *cli_handle, void *context, tOPP_EventCbk *cbk);
void OPP_GetContinueReq(void *cli_handle);
void OPP_AbortReq(void *cli_handle);
#endif

/* Server */
#ifdef CONFIG_OPP_SERVER
void OPP_ServerStartReq(void *context, tOPP_ServerAttributesStru *ops_info, tOPP_EventCbk *cbk);
void OPP_ServerStopReq(void *svr_handle);
void OPP_PutResponse(void *con_handle, UINT16 result, tOPP_SRMPWaitFlag rsp_needed);
void OPP_GetResponse(void *con_handle, UINT16 result, UINT32 data_size);
#endif

#endif
