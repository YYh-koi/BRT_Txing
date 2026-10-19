/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	ftp_ui.h
Abstract:
	This file defines the data structure, constants and function declarations
	of API.
Author:
    Y.D.X
Revision History:
	1.	2015.04.20	Created.
------------------------------------------------------------------------*/
#ifndef _FTP_UI_H
#define _FTP_UI_H

#include "global.h"
#include "hci_ui.h"
#include "obex_ui.h"
#include "gap_ui.h"
#include "sdap_ui.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* Possible error code group values */
#define FTP_ER_GROUP_FTP			  0x0000
#define FTP_ER_GROUP_HCI			  (ER_TYPE_HCI << 8)
#define FTP_ER_GROUP_L2CAP			  (ER_TYPE_L2CAP << 8)
#define FTP_ER_GROUP_RFCOMM		  	  (ER_TYPE_RFCOMM << 8)
#define FTP_ER_GROUP_OBEX			  (ER_TYPE_OBEX << 8)

#define FTP_ER_GROUP_MASK			  0xFE00
#define FTP_GET_ER_GROUP(_res)		  (_res & FTP_ER_GROUP_MASK)
		  
/* Possible result codes */
/* FTP private result code, OBEX standard response code - FTP_ER_GROUP_FTP */
typedef enum {
	/* FTP private result code */
	ER_FTP_SUCCESS,				/* Success */

	ER_FTP_REPEATED_CONNECTION,	/* A connection to the same FTP service exists */

	ER_FTP_SDP_CONNECTION_FAILURE,	/* Fail to create SDP connection */
	ER_FTP_SERVICE_NOT_AVAILABLE,	/* Can't find FTP service record or the FTP service is labeled as unavailable */
	ER_FTP_REMOTE_SDP_TRANSACTION_ERROR,	/* Error occur during SDP transaction */

	ER_FTP_INVALID_HANDLE,			/* The specified handle is not a valid FTP instance handle */
	ER_FTP_NO_CONNECTION,			/* The connection is not ready to execute the operation requested */
	ER_FTP_LOCAL_BUSY,				/* Another operation is incomplete */
	ER_FTP_LOCAL_ABORT,				/* Operation aborted by local APP */
	ER_FTP_LOCAL_CLOSE,				/* The FTP Stack is being closed. */

    ER_FTP_TOO_MORE_SERVICES,       /* Local services of specified type have reached the limit. */

	/* OBEX standard response code, only those often used is defined explicitly */
	ER_FTP_OBEX_RSP_BAD_REQUEST = OBEX_RSP_BAD_REQUEST,
	ER_FTP_OBEX_RSP_UNAUTHORIZED = OBEX_RSP_UNAUTHORIZED,
	ER_FTP_OBEX_RSP_FORBIDDEN = OBEX_RSP_FORBIDDEN,
	ER_FTP_OBEX_RSP_NOT_FOUND = OBEX_RSP_NOT_FOUND,
	ER_FTP_OBEX_RSP_NOT_ACCEPTABLE = OBEX_RSP_NOT_ACCEPTABLE,
	ER_FTP_OBEX_RSP_PRECONDITION_FAILED = OBEX_RSP_PRECONDITION_FAILED,
	ER_FTP_OBEX_RSP_REQUESTED_ENTITY_TOOLARGE = OBEX_RSP_REQUESTED_ENTITY_TOOLARGE,
	ER_FTP_OBEX_RSP_INTERNAL_SERVER_ERROR = OBEX_RSP_INTERNAL_SERVER_ERROR,
	ER_FTP_OBEX_RSP_NOT_IMPLEMENTED = OBEX_RSP_NOT_IMPLEMENTED,
	ER_FTP_OBEX_RSP_SERVICE_UNAVAILABLE = OBEX_RSP_SERVICE_UNAVAILABLE
	
} tFTP_FTPResult;
/* 
	OBEX standard response code - FTP_ER_GROUP_FTP
	e.g. (FTP_ER_GROUP_FTP + OBEX_RSP_BAD_REQUEST)
*/
/* 
	HCI error code - FTP_ER_GROUP_HCI
	e.g. (FTP_ER_GROUP_HCI + HCI_STATUS_PAGE_TIMEOUT)
*/
/* 
	L2CAP error code - FTP_ER_GROUP_L2CAP
	e.g. (FTP_ER_GROUP_L2CAP + ER_L2CAP_RTXTIMEOUT)
*/
/*
	RFCOMM error code - FTP_ER_GROUP_RFCOMM
	e.g. (FTP_ER_GROUP_RFCOMM + ER_RFCOMM_UA_IND_DISC)
*/
/*
	OBEX private error code - FTP_ER_GROUP_OBEX
	e.g. (FTP_ER_GROUP_OBEX + ER_OBEX_DISCONNECT_REQ_BEFORE_OBEXCONNECT)
*/


/* Type of event */
typedef enum {
    /* Common */
	FTP_EVT_CONNECT_COMPLETE,
	FTP_EVT_DISCONNECT_COMPLETE,

	FTP_EVT_ASK4_OBJECT_DATA_IND,
	FTP_EVT_OBJECT_DATA_IND,

    FTP_EVT_PUSH_OBJECT_COMPLETE,
    FTP_EVT_DELETE_OBJECT_COMPLETE = FTP_EVT_PUSH_OBJECT_COMPLETE,
	FTP_EVT_PULL_OBJECT_COMPLETE,

    /* Client */
	FTP_EVT_CLIENT_START_CFM,
	FTP_EVT_AUTHENTICATION_IND,

	FTP_EVT_SET_FOLDER_COMPLETE,

	FTP_EVT_ACTION_COMPLETE,
	FTP_EVT_COPY_OBJECT_COMPLETE = FTP_EVT_ACTION_COMPLETE,
	FTP_EVT_MOVE_RENAME_OBJECT_COMPLETE = FTP_EVT_ACTION_COMPLETE,
	FTP_EVT_SET_OBJECTS_PERMISSIONS_COMPLETE = FTP_EVT_ACTION_COMPLETE,

	/*FTP_EVT_OBJECT_DATA_IND*/
	FTP_EVT_ASK4_GET_REQUEST_IND,
	/*FTP_EVT_PULL_OBJECT_COMPLETE*/

	/*FTP_EVT_ASK4_GET_REQUEST_IND*/
	FTP_EVT_OBJECT_INFORMATION_IND,
	/*FTP_EVT_PULL_OBJECT_COMPLETE*/

	FTP_EVT_ASK4_PUT_REQUEST_IND,
	/*FTP_EVT_ASK4_OBJECT_DATA_IND*/
    /*FTP_EVT_PUSH_OBJECT_COMPLETE*/

    /* Server */
	FTP_EVT_SERVER_START_COMPLETE,
	FTP_EVT_SERVER_STOP_COMPLETE,

	FTP_EVT_CREATE_FOLDER_IND,
	FTP_EVT_SET_ROOT_FOLDER_IND,
	FTP_EVT_SET_CHILD_PARENT_IND,
	FTP_EVT_SET_PARENT_FOLDER_IND,

	FTP_EVT_DELETE_OBJECT_IND,
	FTP_EVT_COPY_OBJECT_IND,
	FTP_EVT_MOVE_RENAME_OBJECT_IND,
	FTP_EVT_SET_PERMISSION_IND,

	FTP_EVT_FIND_FIRST_OBJECT_IND,
	FTP_EVT_FIND_NEXT_OBJECT_IND,


	FTP_EVT_CREATE_FILE_IND,
	FTP_EVT_ASK4_PUT_RESPONSE_IND,
	/*FTP_EVT_OBJECT_DATA_IND*/
	/*FTP_EVT_PUSH_OBJECT_COMPLETE*/

	FTP_EVT_OPEN_FILE_IND,
	FTP_EVT_ASK4_GET_RESPONSE_IND
	/*FTP_EVT_ASK4_OBJECT_DATA_IND*/
	/*FTP_EVT_PULL_OBJECT_COMPLETE*/

} tFTP_Event_Type;

/* Possible values of tFTP_ObjectDataStru::is_final */
typedef enum {
	FTP_DATA_PARTIAL,
	FTP_DATA_COMPLETE,
	FTP_DATA_INCOMPLETE
} tFTP_Completeness;

/* Possible values of tFTP_Ask4GetReqIndCbk::next_op */
typedef enum {
	FTP_OP_CONTINUE,
	FTP_OP_ABORTING,
	FTP_OP_PENDING
} tFTP_GetContinueFlag, tFTP_PutContinueFlag;

/* Possible values of FTP_PutResponse::rsp_needed */
typedef enum {
	FTP_SRMP_WAIT_NOT_REQUIRED,
	FTP_SRMP_WAIT_REQUIRED
} tFTP_SRMPWaitFlag;

/* Possible bit mask values of Digest Challenge options */
#define FTP_DCOPT_USERID_REQUIRED		1
#define FTP_DCOPT_READ_ONLY				2

/* Possible bit mask values of object attributes */
#define FTP_OBJECT_ATTRIBUTE_PARENT					0x40000000				/* If it is set, the object is a parent folder. The following permission bits are not applicable to the Parent Folder object. */
#define FTP_OBJECT_ATTRIBUTE_FOLDER					0x20000000				/* If it is set, the object is a folder. */
/* User Permissions */
#define FTP_OBJECT_ATTRIBUTE_USER_PERM_VALID		0x00800000				/* If it is not set, all the USER_PERM bits are invalid and shall be ignored. */
#define FTP_OBJECT_ATTRIBUTE_USER_PERM_READ			0x00010000				/* If it is set, user READ is allowed for this object. */
#define FTP_OBJECT_ATTRIBUTE_USER_PERM_WRITE		0x00020000				/* If it is set, user WRITE is allowed for this object. */
#define FTP_OBJECT_ATTRIBUTE_USER_PERM_DELETE		0x00040000				/* If it is set, user DELETE is allowed for this object. */
/* Group Permissions */
#define FTP_OBJECT_ATTRIBUTE_GROUP_PERM_VALID		0x00008000				/* If it is not set, all the GROUP_PERM bits are invalid and shall be ignored. */
#define FTP_OBJECT_ATTRIBUTE_GROUP_PERM_READ		0x00000100				/* If it is set, group READ is allowed for this object. */
#define FTP_OBJECT_ATTRIBUTE_GROUP_PERM_WRITE		0x00000200				/* If it is set, group WRITE is allowed for this object. */
#define FTP_OBJECT_ATTRIBUTE_GROUP_PERM_DELETE		0x00000400				/* If it is set, group DELETE is allowed for this object. */
/* Other Permissions */
#define FTP_OBJECT_ATTRIBUTE_OTHER_PERM_VALID		0x00000080				/* If it is not set, all the OTHER_PERM bits are invalid and shall be ignored. */
#define FTP_OBJECT_ATTRIBUTE_OTHER_PERM_READ		0x00000001				/* If it is set, other READ is allowed for this object. */
#define FTP_OBJECT_ATTRIBUTE_OTHER_PERM_WRITE		0x00000002				/* If it is set, other WRITE is allowed for this object. */
#define FTP_OBJECT_ATTRIBUTE_OTHER_PERM_DELETE		0x00000004				/* If it is set, other DELETE is allowed for this object. */

/*---------------------------------------------------------------------------
							Path string rules

File path used in the UI is a null terminated UNICODE text string. It can be
either an absolute path or a relative path. The backslash "\" or slash "/" 
shall be used to indicate the folder hierarchy. Examples:

	Text.txt (relative to the current folder)
	folder1\Text.txt (relative to the current folder)
	folder1/Text.txt (relative to the current folder)
	c:\folder1\Text.txt (absolute path)
	c:/folder1/Text.txt (absolute path)
	\folder1\Text.txt (relative to the root folder)
	/folder1/Text.txt (relative to the root folder)

if relative path is used, the current folder is determined by the application
according to its operation context.

If UNC (Universal Naming Convention) path is used, a full path shall be given
starting with "\\". Backslash "\" is used to delimit path. Example of UNC path:

	\\server1\folder1\Text.txt
----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
typedef struct {
	UINT16  channel;    		/* server channel of the FTP service. 0 means unspecifed. */
	UINT16  psm;        		/* dynamic PSM of the FTP service. 0 means unspecified. */
	UINT8	*service_name;		/* Service Name, NULL terminated UTF-8 string */
} tFTP_ServerAttributesStru;

/* Parameter of FTP_EVT_CONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  result;             /* Connection result */
	/* The following values shall be ignored in case of connection failure */
	void    *connection_handle; /* Handle identifies the new connection entity. It is usually used by the local service to identify the remote client if it allows multiple connections. */
	UINT32  channel_psm;        /* server channel or PSM of the FTP service connected. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM) */
} tFTP_ConnectCompleteStru;

/* Parameter of FTP_EVT_DISCONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  reason;             /* Reason of disconnection */
} tFTP_DisconnectCompleteStru;

typedef struct {
	WCHAR  *name;    		/* Name of the object. Null-terminated UNICODE string. It shall not be NULL. */
	BtUINT64Stru length;	/* Total length in bytes of the object. 0 may be either 0 bytes or unknown length, so it shall be ignored. */
} tFTP_FileInformationStru;

typedef struct {
	UCHAR  *name;    		/* Name of the object. Null-terminated UTF-8 string. NULL if not available. */
	BtUINT64Stru length;	/* Total length in bytes of the object. The size is an estimate and may not be exact. 0 may be either 0 bytes or unknown length, so it shall be ignored. */
	UINT32 attributes;		/* Permissions and type of the object. Bit masks e.g. FTP_OBJECT_ATTRIBUTE_FOLDER. */
	UCHAR  *modified_time;	/* the last modified time of the object. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time) */
	UCHAR  *created_time;	/* the creation time of the object. The same format as modified_time. */
	UCHAR  *accessed_time;	/* the last accessed time of the object. The same format as modified_time. */
	UCHAR  *owner;			/* Name of the owner of this object. Null-terminated UTF-8 string. NULL if not available. */
	UCHAR  *group;			/* Name of the group if group ownership is specified. Null-terminated UTF-8 string. NULL if not available. */
	UCHAR  *type;			/* MIME type of the file object. Null-terminated ASCII string. NULL if not available. */
} tFTP_ObjectInformationStru;

/* Client */
/* Parameter of FTP_EVT_AUTHENTICATION_IND */
typedef struct {
	UINT8	bd[BD_ADDR_LEN];	/* Bluetooth device address of the Server */
	UINT8	options;			/* Options field of Digest Challenge, combinations of FTP_DCOPT_xxxx */
	UINT8	*realm;				/* Realm field of Digest Challenge. NULL if not present */ 	 
} tFTP_AuthenIndStru;

/* Parameter of FTP_EVT_OBJECT_DATA_IND and FTP_EVT_ASK4_OBJECT_DATA_IND */
typedef struct {	
	UINT8				*data;				/* Data chunk of the Object. It is NULL if the Get operation is terminated abnormally. */
	UINT16				length;				/* Size, in bytes, of the data. It is 0 if the Get operation is terminated abnormally. */
	tFTP_Completeness	is_final;			/* FTP_DATA_PARTIAL - More data to be received. FTP_DATA_COMPLETE - It is the final chunk.
											   FTP_DATA_INCOMPLETE - The object transfer is aborted by the client and no more data will be received. */
} tFTP_ObjectDataStru;

/* Parameter of FTP_EVT_ASK4_PUT_REQUEST_IND */
typedef struct {
	tFTP_PutContinueFlag  next_op;          /* Pend, reject or continue the Put operation */
	UINT32                max_data_size;    /* MAX data size expected or available  */
} tFTP_Ask4PutRequestStru;

/* Server */
/* Parameter of FTP_EVT_SERVER_START_COMPLETE */
typedef struct {
	UINT16  result;             /* Server Start result */
	/* The following values shall be ignored in case of a failure */
    UINT32  sdp_record_handle;  /* SDP service record handle of this service. */
	UINT16  channel;    		/* server channel of the service. */
	UINT16  psm;        		/* dynamic PSM of the service. */
} tFTP_ServerStartCompleteStru;

/* Parameter of FTP_EVT_ASK4_GET_RESPONSE_IND */
typedef struct {
	tFTP_GetContinueFlag  next_op;          /* Pend, reject or continue the Get operation */
	UINT32                max_data_size;    /* MAX data size expected or available  */
} tFTP_Ask4GetResponseStru;


/*---------------------------------------------------------------------------
							Callback Prototype
----------------------------------------------------------------------------*/
/*
Description:
    General callback function to handle confirmations or indications.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] The event type.
    param     [in/out] Additional event parameter. Refer to the following
                   specific event callback definition. It is set to NULL if
                   not specified additionally.
*/
typedef void (tFTP_EventCbk)(void *handle, void *context, tFTP_Event_Type event, void *param);

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
    event     [in] Shall be FTP_EVT_CONNECT_COMPLETE.
    param     [in] Result of the connection creation process.
*/
typedef void (tFTP_ConnectCompleteCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_ConnectCompleteStru *param);

/*
Description:
    Callback function to handle disconnect complete event.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be FTP_EVT_DISCONNECT_COMPLETE.
    param     [in] Information of the connection disconnected.
*/
typedef void (tFTP_DisconnectCompleteCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_DisconnectCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the ClientStart request.
	This confirmation is used to return the lower handle identifying the
	FTP connection entity. And the APP can then use this handle to cancel
	the connection creation process.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the
				   event. It is also identify the new connection to create.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart request.
    event     [in] Shall be FTP_EVT_CLIENT_START_CFM.
    bd		  [in] Bluetooth device address of the remote FTP server to connect.
*/
typedef void (tFTP_ClientStartCfmCbk)(void *handle, void *context, tFTP_Event_Type event, UINT8 *bd);

/* 
Description:
    Callback function to handle OBEX authentication initiated by the Server.
	Call FTP_AuthenticationRsp to respond with the proper password.
	NOTE: Only the challenge to the Connection is supported.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the
				   event.
	context	  [in] Application defined data structure used to identify
				   the request being challenged.
    event     [in] Shall be FTP_EVT_AUTHENTICATION_IND.
    param	  [in] Digest Challenge information.
*/
typedef void (tFTP_AuthenIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_AuthenIndStru *param);

/*
Description:
    Callback function to handle the completion of an operation, e.g. PushFile.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] It can be one of:
    			   1) FTP_EVT_SET_FOLDER_COMPLETE,
    			   2) FTP_EVT_PUSH_OBJECT_COMPLETE,
    			   3) FTP_EVT_PULL_OBJECT_COMPLETE,
    			   4) FTP_EVT_ACTION_COMPLETE,
    result    [in] Result of the operation.
*/
typedef void (tFTP_GeneralCompleteCbk)(void *handle, void *context, tFTP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle indication of the object content.

	During the Get operation, the client APP may receive this indication more
	than once. If the remote server rejects the Get request, the APP may not
	receive this indication.

    During the Put operation. the server APP may receive this indication more
	than once. If the server APP rejects the Put request, it may not receive
	this indication.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be FTP_EVT_OBJECT_DATA_IND.
    param     [in] Chunk of the object content.
			  
*/
typedef void (tFTP_ObjectDataIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_ObjectDataStru *param);

/*
Description:
    Callback function to report the information of an object.

    For a PullFile operation, this indication is only created when the Length
    header or HTTP header with content-length is detected in the Get Response
    packet. So only the param->length value is valid in this case. The object
    length may be used to calculate the progress.

    For a BrowseFolder operation, this indication is created repeatedly for
    each folder item described in the Folder Listing object returned.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be FTP_EVT_OBJECT_INFORMATION_IND.
    param     [in] Contains the information of the object.
*/
typedef void (tFTP_ObjectInformationIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_ObjectInformationStru *param);

/*
Description:
    Callback function to handle indication asking for next Get request.
	This indication allows the application to pend the pulling operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be FTP_EVT_ASK4_GET_REQUEST_IND.
    next_op   [in/out] On input, it is set to FTP_OP_CONTINUE. On output,
                   1) Keep it unchanged, the FTP Stack sends Get request to the
                   server immediately after this callback returns.
                   2) Change it to FTP_OP_ABORTING, the FTP Stack sends Abort
                   request to the server immediately after this callback returns. 
                   The pulling operation is then terminated.
                   3) Change it to FTP_OP_PENDING, the FTP Stack waits for the
                   application to call FTP_GetContinueReq to send Get request or
                   to call FTP_AbortReq to terminate the pulling operation.
*/
typedef void (tFTP_Ask4GetReqIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_GetContinueFlag *next_op);


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
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request. It also identify the object being pushed.
    event     [in] Shall be FTP_EVT_ASK4_PUT_REQUEST_IND.
    param     [in/out]
                   param->next_op:
                       On input it is set to FTP_OP_CONTINUE.
                       On output,
                       1) Keep it unchanged, the FTP Stack asks for the message
                       object data using FTP_EVT_ASK4_MESSAGE_DATA_IND event and
                       then sends Put request to the server immediately after this
                       callback returns.
                       2) Change it to FTP_OP_ABORTING, the FTP Stack sends Abort
                       request to the server immediately after this callback returns.
                       The pushing operation is then terminated.
                       3) Change it to FTP_OP_PENDING, the FTP Stack waits for the
                       application to prepare the data to be pushed. The application
                       shall then call FTP_PutContinueReq to notify the FTP Stack
                       when the data is ready to send, or FTP_AbortReq
                       to command the FTP Stack to terminate the pushing operation.

                   param->max_data_size:
                       On input it is set to the MAX size of object content that
                       could be sent in the following Put request. The value is
                       for reference only. The application can ignore it.
                       On output,
                       1) If param->next_op is kept FTP_OP_CONTINUE, the value
                       of max_data_size shall specify the size of object content
                       ready for the FTP Stack to send. The FTP Stack will then
                       try to send as much of these data as possible until the
                       OBEX MTU limit is reached.
                       The value 0 specifies that no more content to read and the
                       Final Put request will be sent.
                       2) If param->next_op is changed to either FTP_OP_ABORT or
                       FTP_OP_PENDING, the param->max_data_size value is ignored.
*/
typedef void (tFTP_Ask4PutReqIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_Ask4PutRequestStru *param);

/*
Description:
    Callback function to get object content from the application.

    Each time after getting possitive confirmation to the FTP_EVT_ASK4_PUT_REQUEST_IND
    from the client APP, the FTP Stack tries to require and push as much data as
    possible until the OBEX MTU limit is reached, or the data ready for this Put
    request have all be sent, or the param->is_final is set to FTP_DATA_COMPLETE.
	If the remote server rejects the Put request, the APP may not receive this
	indication.

    Each time after getting possitive confirmation to the FTP_EVT_ASK4_GET_RESPONSE_IND
    from the server APP, the FTP Stack tries to require and push as much data as
    possible until the OBEX MTU limit is reached, or the data ready for this Get
    response have all be sent, or the param->is_final is set to FTP_DATA_COMPLETE.
	If the server APP rejects the Get request, it may not receive this indication.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request. It also identify the object to read.
    event     [in] Shall be FTP_EVT_ASK4_OBJECT_DATA_IND.
    param     [in/out]Chunk of the object content.
                   param->data: Specifies the buffer to return the object content.
                   param->length: On input, it specifies the size of param->data.
                   On output, it shall be changed to the size of object content,
                   in bytes, copied to param->data. 0 bytes copied means no more
                   content to read.
                   param->is_final: On input, it is set to FTP_DATA_PARTIAL. On
                   output, is shall kept FTP_DATA_PARTIAL until the final chunk.
			  
*/
typedef void (tFTP_Ask4ObjectDataIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_ObjectDataStru *param);

/*
Description:
    Callback function to handle the result of FTP service initiation.
Parameters:
    handle    [in] Handle identify the local entity reporting the event.
				   It is also identify the new service created.
				   It shall be NULL if the service is failed to initiate.
	context	  [in] Application defined data structure used to identify
				   the ServerStart request.
    event     [in] Shall be FTP_EVT_SERVER_START_COMPLETE.
    param     [in] Result of service initiation.
*/
typedef void (tFTP_ServerStartCompleteCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_ServerStartCompleteStru *param);

/*
Description:
    Callback function to handle the result of deregistering FTP service.
Parameters:
    handle    [in] Handle identify the service being deregistered.
	context	  [in] Application defined data structure used to identify
				   the ServerStart request.
    event     [in] Shall be FTP_EVT_SERVER_STOP_COMPLETE.
    result    [in] Result of service initiation.
*/
typedef void (tFTP_ServerStopCompleteCbk)(void *handle, void *context, tFTP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle indication asking for saving the file pushed by
    the remote client.

	This indication allows the application to determine whether to save the
	object or not.

	The application shall call FTP_PutResponse to reply to this indication.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be FTP_EVT_CREATE_FILE_IND.
    param     [in] Contains the information of the file to save.
*/
typedef void (tFTP_CreateFileIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_FileInformationStru *param);

/*
Description:
    Callback function to handle indication asking for Put response.

	This indication allows the application to pend the pushing operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.

	Once the client is allowed to send Put requests without waiting for the
	response, the application won't get FTP_EVT_ASK4_PUT_RESPONSE_IND indication
	for this Put operation any more.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be FTP_EVT_ASK4_PUT_RESPONSE_IND.
    next_op   [in/out] On input, it is set to FTP_OP_CONTINUE. On output,
                   1) Keep it unchanged, the FTP Stack sends Put response to the
                   client immediately after this callback returns. This also
                   instructs the client to send Put requests without waiting for
                   the response if Single Response Mode is enabled.
                   2) Change it to FTP_OP_ABORTING, the FTP Stack sends negative
                   response to the client immediately after this callback returns. 
                   The pushing operation is then terminated.
                   3) Change it to FTP_OP_PENDING, the FTP Stack waits for the
                   application to call FTP_PutResponse to send Put response.
*/
typedef void (tFTP_Ask4PutRspIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_GetContinueFlag *next_op);

/*
Description:
    Callback function to handle indication asking for preparing the file to
    send to the client.

	The application shall call FTP_PutResponse to reply to this indication.
Parameters:
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be FTP_EVT_OPEN_FILE_IND.
    param     [in] The name of the file to be opened for read access.
    			   It is a Null-terminated UNICODE string. It doesn't contain
    			   any path information.
*/
typedef void (tFTP_OpenFileIndCbk)(void *handle, void *context, tFTP_Event_Type event, WCHAR *path);

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
    handle    [in] Handle identify the local FTP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the callback. It also identify the object being pulled.
    event     [in] Shall be FTP_EVT_ASK4_GET_RESPONSE_IND.
    param     [in/out]
                   param->next_op:
                       On input it is set to FTP_OP_CONTINUE.
                       On output,
                       1) Keep it unchanged, the FTP Stack asks for the message
                       object data using FTP_EVT_ASK4_MESSAGE_DATA_IND event and
                       then sends Get response to the client immediately after this
                       callback returns.
                       2) Change it to FTP_OP_ABORTING, the FTP Stack sends negative
                       response to the client immediately after this callback returns.
                       The pulling operation is then terminated.
                       3) Change it to FTP_OP_PENDING, the FTP Stack waits for the
                       application to prepare the data to be pushed. The application
                       shall then call FTP_GetResponse to notify the FTP Stack
                       when the data is ready to send, or to command the FTP Stack
                       to terminate the pushing operation.

                   param->max_data_size:
                       On input it is set to the MAX size of object content that
                       could be sent in the following Get response. The value is
                       for reference only. The application can ignore it.
                       On output,
                       1) If param->next_op is kept FTP_OP_CONTINUE, the value
                       of max_data_size shall specify the size of object content
                       ready for the FTP Stack to send. The FTP Stack will then
                       try to send as much of these data as possible until the
                       OBEX MTU limit is reached.
                       The value 0 specifies that no more content to read and the
                       Final Get response will be sent.
                       2) If param->next_op is changed to either FTP_OP_ABORT or
                       FTP_OP_PENDING, the param->max_data_size value is ignored.
*/
typedef void (tFTP_Ask4GetRspIndCbk)(void *handle, void *context, tFTP_Event_Type event, tFTP_Ask4GetResponseStru *param);


/*---------------------------------------------------------------------------
							Interfaces
----------------------------------------------------------------------------*/
/* Common */
void FTP_Init(void);
void FTP_Done(void);
void FTP_DisconnectReq(void *con_handle);

/* Client */
#ifdef CONFIG_FTP_CLIENT
void FTP_ClientStartReq(void *context, UINT8 *bd, tFTP_ServerAttributesStru *svr_info, tFTP_EventCbk *cbk);
void FTP_SetFolderReq(void *cli_handle, void *context, WCHAR *folder, UINT8 create_new, tFTP_GeneralCompleteCbk *cbk);
/* Set root folder */
#define FTP_SetRootFolderReq(_hdl, _cont, _cbk)	FTP_SetFolderReq(_hdl, _cont, NULL, 0, _cbk)
/* Set parent folder */
#define FTP_SetParentFolderReq(_hdl, _cont, _cbk) FTP_SetFolderReq(_hdl, _cont, (WCHAR *)g_obex_parent_folder, 0, _cbk)
/* Set child folder */
#define FTP_SetChildFolderReq(_hdl, _cont, folder, _cbk)	FTP_SetFolderReq(_hdl, _cont, folder, 0, _cbk)
/* Create new folder */
#define FTP_CreateFolderReq(_hdl, _cont, folder, _cbk)	FTP_SetFolderReq(_hdl, _cont, folder, 1, _cbk)
void FTP_PushFileReq(void *cli_handle, void *context, tFTP_FileInformationStru *file_info, tFTP_EventCbk *cbk);
void FTP_PutContinueReq(void *cli_handle, UINT32 data_size);
void FTP_PullFileReq(void *cli_handle, void *context, WCHAR *file_name, tFTP_EventCbk *cbk);
void FTP_BrowseFolderReq(void *cli_handle, void *context, tFTP_EventCbk *cbk);
void FTP_GetContinueReq(void *cli_handle);
void FTP_PullFolderReq(void *cli_handle, void *context, WCHAR *folder_name, tFTP_EventCbk *cbk);
void FTP_PushFolderReq(void *cli_handle, void *context, WCHAR *path, tFTP_EventCbk *cbk);
void FTP_DeleteObjectReq(void *cli_handle, void *context, WCHAR *name, tFTP_GeneralCompleteCbk *cbk);
void FTP_CopyObjectReq(void *cli_handle, void *context, WCHAR *src_name, WCHAR *dest_name, tFTP_GeneralCompleteCbk *cbk);
void FTP_MoveRenameObjectReq(void *cli_handle, void *context, WCHAR *src_name, WCHAR *dest_name, tFTP_GeneralCompleteCbk *cbk);
void FTP_SetObjectPermissionsReq(void *cli_handle, void *context, WCHAR *obj_name, UINT32 flags, tFTP_GeneralCompleteCbk *cbk);
void FTP_AbortReq(void *cli_handle);
void FTP_AuthenticationRsp(void *cli_handle, UCHAR *user_id, UCHAR *password);
#endif

/* Server */
#ifdef CONFIG_FTP_SERVER
void FTP_ServerStartReq(void *context, tFTP_ServerAttributesStru *ops_info, tFTP_EventCbk *cbk);
void FTP_ServerStopReq(void *svr_handle);
void FTP_PutResponse(void *con_handle, UINT16 result, tFTP_SRMPWaitFlag rsp_needed);
void FTP_GetResponse(void *con_handle, UINT16 result, UINT32 data_size);
#endif

#endif
