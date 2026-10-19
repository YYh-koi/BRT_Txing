/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 BRT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	map_ui.h
Abstract:
	This file defines the data structure, constants and function declarations
	of API.
Author:
    Y.D.X
Revision History:
	1.	2013.12.05	Created.
------------------------------------------------------------------------*/
#ifndef _MAP_UI_H
#define _MAP_UI_H

#include "global.h"
#include "hci_ui.h"
#include "obex_ui.h"
#include "gap_ui.h"
#include "sdap_ui.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* UNICODE identify the parent folder */
static const WCHAR g_map_parent_folder[] = {'.', '.', 0};

/* Possible error code group values */
#define MAP_ER_GROUP_MAP            0x0000
#define MAP_ER_GROUP_HCI            (ER_TYPE_HCI << 8)
#define MAP_ER_GROUP_L2CAP          (ER_TYPE_L2CAP << 8)
#define MAP_ER_GROUP_RFCOMM         (ER_TYPE_RFCOMM << 8)
#define MAP_ER_GROUP_OBEX           (ER_TYPE_OBEX << 8)

#define MAP_ER_GROUP_MASK           0xFE00
#define MAP_GET_ER_GROUP(_res)      (_res & MAP_ER_GROUP_MASK)
		  
/* Possible result codes */
/* MAP private result code, OBEX standard response code - MAP_ER_GROUP_MAP */
typedef enum {
	/* MAP private result code */
	ER_MAP_SUCCESS,				    /* Success */

	ER_MAP_REPEATED_CONNECTION,	    /* A connection to the same service exists */

	ER_MAP_SDP_CONNECTION_FAILURE,	/* Fail to create SDP connection */
	ER_MAP_SERVICE_NOT_AVAILABLE,	/* Can't find the specified service record or the service record is labeled as unavailable */
	ER_MAP_REMOTE_SDP_TRANSACTION_ERROR,	/* Error occur during SDP transaction */

	ER_MAP_INVALID_HANDLE,			/* The specified handle is not a valid MAP instance handle */
	ER_MAP_NO_CONNECTION,			/* The connection is not ready to execute the operation requested */
	ER_MAP_LOCAL_BUSY,				/* Another operation is incomplete */
	ER_MAP_LOCAL_ABORT,			    /* Operation aborted by local APP */
	ER_MAP_LOCAL_CLOSE,			    /* The MAP Stack is being closed. */

    ER_MAP_TOO_MORE_SERVICES,       /* Local services of specified type have reached the limit. */

	/* OBEX standard response code, only those listed in the MAP Spec is defined explicitly */
	ER_MAP_OBEX_RSP_BAD_REQUEST = OBEX_RSP_BAD_REQUEST,
	ER_MAP_OBEX_RSP_UNAUTHORIZED = OBEX_RSP_UNAUTHORIZED,
	ER_MAP_OBEX_RSP_FORBIDDEN = OBEX_RSP_FORBIDDEN,
	ER_MAP_OBEX_RSP_NOT_FOUND = OBEX_RSP_NOT_FOUND,
	ER_MAP_OBEX_RSP_NOT_ACCEPTABLE = OBEX_RSP_NOT_ACCEPTABLE,
	ER_MAP_OBEX_RSP_PRECONDITION_FAILED = OBEX_RSP_PRECONDITION_FAILED,
	ER_MAP_OBEX_RSP_NOT_IMPLEMENTED = OBEX_RSP_NOT_IMPLEMENTED,
	ER_MAP_OBEX_RSP_SERVICE_UNAVAILABLE = OBEX_RSP_SERVICE_UNAVAILABLE
	
} tMAP_MapResult;
/* 
	OBEX standard response code - MAP_ER_GROUP_MAP
	e.g. (MAP_ER_GROUP_MAP + OBEX_RSP_BAD_REQUEST)
*/
/* 
	HCI error code - MAP_ER_GROUP_HCI
	e.g. (MAP_ER_GROUP_HCI + HCI_STATUS_PAGE_TIMEOUT)
*/
/* 
	L2CAP error code - MAP_ER_GROUP_L2CAP
	e.g. (MAP_ER_GROUP_L2CAP + ER_L2CAP_RTXTIMEOUT)
*/
/*
	RFCOMM error code - MAP_ER_GROUP_RFCOMM
	e.g. (MAP_ER_GROUP_RFCOMM + ER_RFCOMM_UA_IND_DISC)
*/
/*
	OBEX private error code - MAP_ER_GROUP_OBEX
	e.g. (MAP_ER_GROUP_OBEX + ER_OBEX_DISCONNECT_REQ_BEFORE_OBEXCONNECT)
*/


/* Type of event */
typedef enum {
    /* Common */
	MAP_EVT_SERVER_START_COMPLETE,
	MAP_EVT_SERVER_STOP_COMPLETE,
	MAP_EVT_CLIENT_START_CFM,
	MAP_EVT_CONNECT_COMPLETE,
	MAP_EVT_DISCONNECT_COMPLETE,

    /* MCE */
	MAP_EVT_MAS_SERVICE_IND,
	MAP_EVT_DISCOVER_MAS_COMPLETE,

	MAP_EVT_SET_FOLDER_COMPLETE,
	MAP_EVT_SET_NOTIFICATION_REGISTRATION_COMPLETE,
	MAP_EVT_UPDATE_INBOX_COMPLETE,
	MAP_EVT_SET_MESSAGE_STATUS_COMPLETE,
	MAP_EVT_SET_OWNER_STATUS_COMPLETE,
	MAP_EVT_SET_NOTIFICATION_FILTER_COMPLETE,

	MAP_EVT_OBJECT_DATA_IND,
	MAP_EVT_ASK4_GET_REQUEST_IND,
	MAP_EVT_GET_FOLDERLIST_COMPLETE,
	MAP_EVT_GET_MESSAGELIST_COMPLETE,
	MAP_EVT_GET_MESSAGE_COMPLETE,
	MAP_EVT_GET_MAS_INSTANCE_INFORMATION_COMPLETE,
	MAP_EVT_GET_OWNER_STATUS_COMPLETE,
	MAP_EVT_GET_CONVERSATIONLIST_COMPLETE,

	MAP_EVT_ASK4_PUT_REQUEST_IND,
	MAP_EVT_ASK4_MESSAGE_DATA_IND,
    MAP_EVT_PUSH_MESSAGE_COMPLETE,

    MAP_EVT_EVENT_REPORT_IND,

    /* MSE */
    
} tMAP_Event_Type;

/* Possible values of ApplicatinParameters Header using ON/OFF */
typedef enum {
    MAP_MASK_OFF,
    MAP_MASK_ON
} tMAP_OnOffMask;

/* Possible bit mask values of MAP_DiscoverMAS::conditions */
#define MAP_CONDITION_EMAIL_SUPPORT                 0x01
#define MAP_CONDITION_SMS_GSM_SUPPORT               0x02
#define MAP_CONDITION_SMS_CDMA_SUPPORT              0x04
#define MAP_CONDITION_MMS_SUPPORT                   0x08
#define MAP_CONDITION_ANY_MSG_TYPE					0xFF
#define MAP_CONDITION_SERVICE_NAME_REQUIRED			0x8000	/* Return name of the service(s) */

/* Possible bit mask values of <structure>::flag */
#define MAP_FLAG_MAXLISTCOUNT						0x00000001
#define MAP_FLAG_LISTSTARTOFFSET					0x00000002
#define MAP_FLAG_FILTER_MESSAGETYPE					0x00000004
#define MAP_FLAG_FILTER_PERIODBEGIN					0x00000008
#define MAP_FLAG_FILTER_PERIODEND					0x00000010
#define MAP_FLAG_FILTER_READSTATUS					0x00000020
#define MAP_FLAG_FILTER_RECIPIENT					0x00000040
#define MAP_FLAG_FILTER_ORIGINATOR					0x00000080
#define MAP_FLAG_FILTER_PRIORITY					0x00000100
#define MAP_FLAG_ATTACHMENT     					0x00000200
#define MAP_FLAG_TRANSPARENT     					0x00000400
#define MAP_FLAG_RETRY                              0x00000800
#define MAP_FLAG_NEWMESSAGE                         0x00001000
#define MAP_FLAG_NOTIFICATIONSTATUS                 0x00002000
#define MAP_FLAG_MASINSTANCEID                      0x00004000
#define MAP_FLAG_PARAMETERMASK                      0x00008000
#define MAP_FLAG_FOLDERLISTINGSIZE                  0x00010000
#define MAP_FLAG_MESSAGELISTINGSIZE                 0x00020000
#define MAP_FLAG_LISTINGSIZE						0x00020000	/* MessageListingSize is renamed as ListingSize since MAP V1.3 */
#define MAP_FLAG_SUBJECTLENGTH                      0x00040000
#define MAP_FLAG_CHARSET                            0x00080000
#define MAP_FLAG_FRACTIONREQUEST                    0x00100000
#define MAP_FLAG_FRACTIONDELIVER                    0x00200000
#define MAP_FLAG_STATUSINDICATOR                    0x00400000
#define MAP_FLAG_STATUSINVALUE                      0x00800000
#define MAP_FLAG_MSETIME                            0x01000000
#define MAP_FLAG_DATABASEID							0x02000000
#define MAP_FLAG_CONVLISTING_VERSIONCOUNTER			0x04000000
#define MAP_FLAG_PRESENCEAVAILABILITY				0x08000000
#define MAP_FLAG_PRESENCETEXT						0x10000000
#define MAP_FLAG_LASTACTIVITY						0x20000000
#define MAP_FLAG_FILTER_LASTACTIVITYBEGIN			0x40000000
#define MAP_FLAG_FILTER_LASTACTIVITYEND				0x80000000
/* Possible bit mask values of <structure>::flag_b1 */
#define MAP_FLAG_B1_CHATSTATE						0x00000001
#define MAP_FLAG_B1_CONVERSATIONID					0x00000002
#define MAP_FLAG_B1_FOLDERVERSIONCOUNTER			0x00000004
#define MAP_FLAG_B1_FILTER_MESSAGEHANDLE			0x00000008
#define MAP_FLAG_B1_NOTIFICATIONFILTERMASK			0x00000010
#define MAP_FLAG_B1_CONVPARAMETERMASK				0x00000020
#define MAP_FLAG_B1_OWNERUCI						0x00000040
#define MAP_FLAG_B1_EXTENDEDDATA					0x00000080
#define MAP_FLAG_B1_MAPSUPPORTEDFEATURES			0x00000100
#define MAP_FLAG_B1_MESSAGEHANDLE					0x00000200
#define MAP_FLAG_B1_MODIFYTEXT						0x00000400

/* Possible bit mask values of <structure>::parameter_mask */
#define MAP_PARMASK_SUBJECT						    0x00000001
#define MAP_PARMASK_DATE_TIME						0x00000002
#define MAP_PARMASK_SENDER_NAME						0x00000004
#define MAP_PARMASK_SENDER_ADDRESSING				0x00000008
#define MAP_PARMASK_RECIPIENT_NAME   				0x00000010
#define MAP_PARMASK_RECIPIENT_ADDRESSING  			0x00000020
#define MAP_PARMASK_TYPE                  			0x00000040
#define MAP_PARMASK_SIZE                  			0x00000080
#define MAP_PARMASK_RECEPTION_STATUS                0x00000100
#define MAP_PARMASK_TEXT                            0x00000200
#define MAP_PARMASK_ATTACHMENT_SIZE                 0x00000400
#define MAP_PARMASK_PRIORITY                        0x00000800
#define MAP_PARMASK_READ                            0x00001000
#define MAP_PARMASK_SENT                            0x00002000
#define MAP_PARMASK_PROTECTED                       0x00004000
#define MAP_PARMASK_REPLYTO_ADDRESSING              0x00008000
#define MAP_PARMASK_DELIVERY_STATUS		            0x00010000
#define MAP_PARMASK_CONVERSATION_ID		            0x00020000
#define MAP_PARMASK_CONVERSATION_NAME	            0x00040000
#define MAP_PARMASK_DIRECTION			            0x00080000
#define MAP_PARMASK_ATTACHMENT_MIME					0x00100000

/* Possible bit mask values of <structure>::filter_message_type */
#define MAP_FILTER_MSGTYPE_SMS_GSM                  0x01
#define MAP_FILTER_MSGTYPE_SMS_CDMA                 0x02
#define MAP_FILTER_MSGTYPE_EMAIL                    0x04
#define MAP_FILTER_MSGTYPE_MMS                      0x08
#define MAP_FILTER_MSGTYPE_IM                      	0x10

/* Possible bit mask values of <structure>::supported_message_types */
#define MAP_SUPPORTED_MSGTYPE_EMAIL                 0x01
#define MAP_SUPPORTED_MSGTYPE_SMS_GSM               0x02
#define MAP_SUPPORTED_MSGTYPE_SMS_CDMA              0x04
#define MAP_SUPPORTED_MSGTYPE_MMS                   0x08
#define MAP_SUPPORTED_MSGTYPE_IM                   	0x10

/* Possible bit mask values of <structure>::supported_features */
#define MAP_SUPPORTED_FEATURE_NOTIFICATION_REGISTRATION						0x00000001
#define MAP_SUPPORTED_FEATURE_NOTIFICATION           						0x00000002
#define MAP_SUPPORTED_FEATURE_BROWSING	        							0x00000004
#define MAP_SUPPORTED_FEATURE_UPLOADING			 							0x00000008
#define MAP_SUPPORTED_FEATURE_DELETE										0x00000010
#define MAP_SUPPORTED_FEATURE_INSTANCE_INFORMATION							0x00000020
#define MAP_SUPPORTED_FEATURE_EXTENDED_EVENT_REPORT_V1_1					0x00000040
#define MAP_SUPPORTED_FEATURE_EVENT_REPORT_V1_2								0x00000080
#define MAP_SUPPORTED_FEATURE_MESSAGE_FORMAT_V1_1							0x00000100
#define MAP_SUPPORTED_FEATURE_MESSAGE_LISTING_FORMAT_V1_1					0x00000200
#define MAP_SUPPORTED_FEATURE_PERSISTENT_MESSAGE_HANDLES					0x00000400
#define MAP_SUPPORTED_FEATURE_DATABASE_ID									0x00000800
#define MAP_SUPPORTED_FEATURE_FOLDER_VERSION_COUNTER						0x00001000
#define MAP_SUPPORTED_FEATURE_CONVERSATION_VERSION_COUNTERS					0x00002000
#define MAP_SUPPORTED_FEATURE_PARTICIPANT_PRESENCE_CHANGE_NOTIFICATION		0x00004000
#define MAP_SUPPORTED_FEATURE_PARTICIPANT_CHAT_STATE_CHANGE_NOTIFICATION	0x00008000
#define MAP_SUPPORTED_FEATURE_PBAP_CONTACT_CROSS_REFERENCE					0x00010000
#define MAP_SUPPORTED_FEATURE_NOTIFICATION_FILTERING						0x00020000
#define MAP_SUPPORTED_FEATURE_UTC_OFFSET_TIMESTAMP_FORMAT					0x00040000
#define MAP_SUPPORTED_FEATURE_IN_CONNECT_REQUEST							0x00080000	/* MSE only */
#define MAP_SUPPORTED_FEATURE_CONVERSATION_LISTING							0x00100000
#define MAP_SUPPORTED_FEATURE_OWNER_STATUS									0x00200000
#define MAP_SUPPORTED_FEATURE_MESSAGE_FORWARDING							0x00400000

/* Possible bit mask values of <structure>::conv_parameter_mask */
#define MAP_CONV_PARMASK_CONVERSATION_NAME					0x00000001
#define MAP_CONV_PARMASK_CONVERSATION_LAST_ACTIVITY			0x00000002
#define MAP_CONV_PARMASK_CONVERSATION_READ_STATUS			0x00000004
#define MAP_CONV_PARMASK_CONVERSATION_VERSION_COUNTER		0x00000008
#define MAP_CONV_PARMASK_CONVERSATION_SUMMARY   			0x00000010
#define MAP_CONV_PARMASK_PARTICIPANTS  						0x00000020
#define MAP_CONV_PARMASK_PARTICIPANT_UCI             		0x00000040
#define MAP_CONV_PARMASK_PARTICIPANT_DISPLAY_NAME      		0x00000080
#define MAP_CONV_PARMASK_PARTICIPANT_CHAT_STATE				0x00000100
#define MAP_CONV_PARMASK_PARTICIPANT_LAST_ACTIVITY			0x00000200
#define MAP_CONV_PARMASK_PARTICIPANT_XBT_UID				0x00000400
#define MAP_CONV_PARMASK_PARTICIPANT_NAME					0x00000800
#define MAP_CONV_PARMASK_PARTICIPANT_PRESENCE_AVAILABILITY	0x00001000
#define MAP_CONV_PARMASK_PARTICIPANT_PRESENCE_TEXT			0x00002000
#define MAP_CONV_PARMASK_PARTICIPANT_PRIORITY				0x00004000

/* Possible bit mask values of NotificationFilterMask */
#define MAP_NOTIFICATION_MASK_NEW_MESSAGE						0x00000001
#define MAP_NOTIFICATION_MASK_MESSAGE_DELETED           		0x00000002
#define MAP_NOTIFICATION_MASK_MESSAGE_SHIFT	        			0x00000004
#define MAP_NOTIFICATION_MASK_SENDING_SUCCESS			 		0x00000008
#define MAP_NOTIFICATION_MASK_SENDING_FAILURE					0x00000010
#define MAP_NOTIFICATION_MASK_DELIVERY_SUCCESS					0x00000020
#define MAP_NOTIFICATION_MASK_DELIVERY_FAILURE					0x00000040
#define MAP_NOTIFICATION_MASK_MEMORY_FULL						0x00000080
#define MAP_NOTIFICATION_MASK_MEMORY_AVAILABLE					0x00000100
#define MAP_NOTIFICATION_MASK_READ_STATUS_CHANGED				0x00000200
#define MAP_NOTIFICATION_MASK_CONVERSATION_CHANGED				0x00000400
#define MAP_NOTIFICATION_MASK_PARTICIPANT_PRESENCE_CHANGED		0x00000800
#define MAP_NOTIFICATION_MASK_PARTICIPANT_CHAT_STATE_CHANGED	0x00001000
#define MAP_NOTIFICATION_MASK_MESSAGE_EXTENDED_DATA_CHANGED		0x00002000
#define MAP_NOTIFICATION_MASK_MESSAGE_REMOVED					0x00004000

/* Possible values of <structure>::filter_read_status */
typedef enum {
    MAP_FILTER_BOTH_READ_UNREAD,
    MAP_FILTER_UNREAD_ONLY,
    MAP_FILTER_READ_ONLY
} tMAP_FilterReadStatus;

/* Possible values of <structure>::filter_priority */
typedef enum {
    MAP_FILTER_IGNORE_PRIORITY,
    MAP_FILTER_HIGH_PRIORITY_ONLY,
    MAP_FILTER_NONHIGH_PRIORITY_ONLY
} tMAP_FilterPriority;

/* Possible values of <structure>::charset */
typedef enum {
	MAP_CHARSET_NATIVE,
	MAP_CHARSET_UTF8
} tMAP_Charset;

/* Possible values of <structure>::fraction_request */
typedef enum {
	MAP_FRACTION_REQUEST_FIRST,
	MAP_FRACTION_REQUEST_NEXT
} tMAP_FractionRequest;

/* Possible values of <structure>::fraction_deliver */
typedef enum {
	MAP_FRACTION_DELIVER_MORE,
	MAP_FRACTION_DELIVER_LAST
} tMAP_FractionDeliver;

/* Possible values of MessageStatus, it is a combination of StatusIndicator and StautsValue */
typedef enum {
	MAP_MSGSTATUS_UNREAD,
	MAP_MSGSTATUS_READ,
	MAP_MSGSTATUS_UNDELETED,
	MAP_MSGSTATUS_DELETED
} tMAP_MessageStatus;

/* Possible values of tMAP_ObjectDataStru::is_final */
typedef enum {
	MAP_DATA_PARTIAL,
	MAP_DATA_COMPLETE,
	MAP_DATA_INCOMPLETE
} tMAP_Completeness;

/* Possible values of tMAP_Ask4GetReqIndCbk::next_op, tMAP_Ask4PutRequestStru::next_op */
typedef enum {
	MAP_OP_CONTINUE,
	MAP_OP_ABORTING,
	MAP_OP_PENDING
} tMAP_GetContinueFlag, tMAP_PutContinueFlag;

/* Possible values of <structure>::modify_text */
typedef enum {
	MAP_TEXT_REPLACE,
	MAP_TEXT_PREPEND
} tMAP_ModifyText;

/* Possible values of <structure>::presence_availability */
typedef enum {
	MAP_PRESENCE_UNKNOWN,
	MAP_PRESENCE_OFFLINE,
	MAP_PRESENCE_ONLINE,
	MAP_PRESENCE_AWAY,
	MAP_PRESENCE_DONT_DISTURB,
	MAP_PRESENCE_BUSY,
	MAP_PRESENCE_IN_MEETING
} tMAP_Presence;

/* Possible values of <structure>::chat_state */
typedef enum {
	MAP_CHATSTATE_UNKNOWN,
	MAP_CHATSTATE_INACTIVE,
	MAP_CHATSTATE_ACTIVE,
	MAP_CHATSTATE_COMPOSING,
	MAP_CHATSTATE_PAUSED_COMPOSING,
	MAP_CHATSTATE_GONE
} tMAP_ChatState;


/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
typedef struct {
	UINT16  channel;    		/* server channel of the MAS service. 0 means unspecifed. */
	UINT16  psm;        		/* dynamic PSM of the MAS service. 0 means unspecified. */
	UINT8   mas_instance_id;    /* MASInstanceID of the MAS service. */
	UINT8   supported_message_types;/* SupportedMessageTypes of the MAS service, bit mask of MAP_SUPPORTED_MSGTYPE_EMAIL, etc.. */
	UINT8	*service_name;		/* Service Name, NULL terminated UTF-8 string */
    UINT32  sdp_record_handle;  /* SDP service record handle of this MAS service. Only used to return value to the application. The value specified by the application is ignored. */
    UINT32  supported_features;  /* MapSupportedFeatures of this MAS service. bit mask of MAP_SUPPORTED_FEATURE_NOTIFICATION, etc.. */
} tMAP_MASAttributesStru;

typedef struct {
	UINT16  channel;    		/* server channel of the MNS service. 0 means unspecifed. */
	UINT16  psm;        		/* dynamic PSM of the MNS service. 0 means unspecified. */
} tMAP_MNSAttributesStru;

/* Parameter of MAP_EVT_SERVER_START_COMPLETE */
typedef struct {
	UINT16  result;             /* Server Start result */
	/* The following values shall be ignored in case of a failure */
    UINT32  sdp_record_handle;  /* SDP service record handle of this service. */
	UINT16  channel;    		/* server channel of the service. */
	UINT16  psm;        		/* dynamic PSM of the service. */
	UINT8   mas_instance_id;    /* MASInstanceID of the MAS service. Ignored for MNS service */
    UINT32  supported_features; /* MapSupportedFeatures of this service. bit mask of MAP_SUPPORTED_FEATURE_NOTIFICATION, etc.. */
} tMAP_ServerStartCompleteStru;

/* Parameter of MAP_EVT_CONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  result;             /* Connection result */
	/* The following values shall be ignored in case of connection failure */
	void    *connection_handle; /* Handle identifies the new connection entity. It is usually used by the local service to identify the remote client if it allows multiple connections. */
	UINT32  channel_psm;        /* server channel or PSM of the MAP service connected. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM) */
	UINT8   mas_instance_id;    /* MASInstanceID of the MAS service connected. Ignored for both roles of MNS connection */
	UINT8   supported_message_types;/* SupportedMessageTypes of the MAS service connected, bit mask of MAP_SUPPORTED_MSGTYPE_EMAIL, etc.. Ignored for both roles of MNS connection */
    UINT32  supported_features;  /* MapSupportedFeatures of this connection. It specifies the features supported by both sides. bit mask of MAP_SUPPORTED_FEATURE_NOTIFICATION, etc.. */
} tMAP_ConnectCompleteStru;

/* Parameter of MAP_EVT_DISCONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  reason;             /* Reason of disconnection */
} tMAP_DisconnectCompleteStru;

/* Parameter of MAP_EVT_SET_NOTIFICATION_REGISTRATION_COMPLETE */
typedef struct {
	UINT16  		result;		/* Result of the SetNotificationRegistration operation, ER_MAP_SUCCESS etc. */
	tMAP_OnOffMask 	status;	    /* Current Notification tatus */
} tMAP_NotificationRegCompleteStru;

/* Application Parameters header in the GetFolderListing request */
typedef struct {
	UINT32  flag;				/* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT16  max_list_count;		/* MaxListCount parameter. It is ignored if MAP_FLAG_MAXLISTCOUNT is not set in the flag */
	UINT16  list_start_offset;	/* ListStartOffset parameter. It is ignored if MAP_FLAG_LISTSTARTOFFSET is not set in the flag */
} tMAP_GetFolderListReqStru;

/* Application Parameters header in the GetMessageListing request */
typedef struct {
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT16  max_list_count;		    /* MaxListCount parameter. It is ignored if MAP_FLAG_MAXLISTCOUNT is not set in the flag */
	UINT16  list_start_offset;	    /* ListStartOffset parameter. It is ignored if MAP_FLAG_LISTSTARTOFFSET is not set in the flag */
	UINT32  parameter_mask;         /* ParameterMask parameter, bit mask of MAP_PARMASK_SUBJECT, etc.. It is ignored if MAP_FLAG_PARAMETERMASK is not set in the flag */
	UINT8   subject_length;         /* SubjectLength parameter. It is ignored if MAP_FLAG_SUBJECTLENGTH is not set in the flag */
	UINT8   filter_message_type;    /* FilterMessageType parameter, bit mask of MAP_FILTER_MSGTYPE_MMS, etc.. It is ignored if MAP_FLAG_FILTER_MESSAGETYPE is not set in the flag */
	UINT8   filter_read_status;     /* FilterReadStatus parameter, one of tMAP_FilterReadStatus. It is ignored if MAP_FLAG_FILTER_READSTATUS is not set in the flag */
	UINT8   filter_priority;        /* FilterPriority parameter, one of tMAP_FilterPriority. It is ignored if MAP_FLAG_FILTER_PRIORITY is not set in the flag */
	UCHAR   *filter_period_begin;   /* FilterPeriodBegin parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_FILTER_PERIODBEGIN is not set in the flag */
	UCHAR   *filter_period_end;     /* FilterPeriodEnd parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_FILTER_PERIODEND is not set in the flag */
	UCHAR   *filter_originator;     /* FilterOriginator parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_FILTER_ORIGINATOR is not set in the flag */
	UCHAR   *filter_recipient;      /* FilterRecipient parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_FILTER_RECIPIENT is not set in the flag */
} tMAP_GetMessageListReqStru;

/* Application Parameters header in the GetMessage request */
typedef struct {
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT8   attachment;		        /* Attachment parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_ATTACHMENT is not set in the flag */
	UINT8   charset;	            /* Charset parameter, one of tMAP_Charset. It is ignored if MAP_FLAG_CHARSET is not set in the flag */
	UINT8   fraction_request;       /* FractionRequest parameter, one of tMAP_FractionRequest. It is ignored if MAP_FLAG_FRACTIONREQUEST is not set in the flag */
} tMAP_GetMessageReqStru;

/* Application Parameters header in the PushMessage request */
typedef struct {
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT32  flag_b1;				/* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_B1_FOLDERVERSIONCOUNTER etc. */
	UINT8   transparent;		    /* Transparent parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_TRANSPARENT is not set in the flag */
	UINT8   retry;	                /* Charset parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_RETRY is not set in the flag */
	UINT8   charset;	            /* Charset parameter, one of tMAP_Charset. It is ignored if MAP_FLAG_CHARSET is not set in the flag */
	UCHAR   *conversation_id;      	/* ConversationID parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_B1_CONVERSATIONID is not set in the flag_b1 */
	UCHAR   *message_handle;      	/* MessageHandle parameter, a 64-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_B1_MESSAGEHANDLE is not set in the flag_b1 */
	UINT8   attachment;		        /* Attachment parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_ATTACHMENT is not set in the flag */
	UINT8   modify_text;	        /* ModifyText parameter, one of tMAP_ModifyText. It is ignored if MAP_FLAG_B1_MODIFYTEXT is not set in the flag_b1 */
} tMAP_PushMessageReqStru;

/* Application Parameters header in the SetOwnerStatus request */
typedef struct {
	UINT32  flag;					/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_PRESENCEAVAILABILITY etc. */
	UINT32  flag_b1;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_B1_CHATSTATE etc. */
	UINT8   presence_availability;  /* PresenceAvailability parameter, one of tMAP_Presence. It is ignored if MAP_FLAG_PRESENCEAVAILABILITY is not set in the flag */
	UINT8   chat_state;       		/* ChatState parameter, one of tMAP_ChatState. It is ignored if MAP_FLAG_B1_CHATSTATE is not set in the flag_b1 */
	UCHAR   *presence_text;       	/* PresenceText parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_PRESENCETEXT is not set in the flag */
	UCHAR   *last_activity;       	/* LastActivity parameter, NULL terminated UTF-8 string in format of ¡°YYYYMMDDTHHMMSS¡±. It is ignored if MAP_FLAG_LASTACTIVITY is not set in the flag */
	UCHAR   *conversation_id;      	/* ConversationID parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_B1_CONVERSATIONID is not set in the flag_b1 */
} tMAP_SetOwnerStatusReqStru;

/* Application Parameters header in the GetConversationListing request */
typedef struct {
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, PBAP_FLAG_FILTER etc. */
	UINT32  flag_b1;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_B1_CHATSTATE etc. */
	UINT16  max_list_count;		    /* MaxListCount parameter. It is ignored if MAP_FLAG_MAXLISTCOUNT is not set in the flag */
	UINT16  list_start_offset;	    /* ListStartOffset parameter. It is ignored if MAP_FLAG_LISTSTARTOFFSET is not set in the flag */
	UINT8   filter_read_status;     /* FilterReadStatus parameter, one of tMAP_FilterReadStatus. It is ignored if MAP_FLAG_FILTER_READSTATUS is not set in the flag */
	UCHAR   *filter_last_activity_begin;   /* FilterLastActivityBegin parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_FILTER_LASTACTIVITYBEGIN is not set in the flag */
	UCHAR   *filter_last_activity_end;     /* FilterLastActivityEnd parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_FILTER_LASTACTIVITYEND is not set in the flag */
	UCHAR   *filter_recipient;      /* FilterRecipient parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_FILTER_RECIPIENT is not set in the flag */
	UCHAR   *conversation_id;      	/* ConversationID parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_B1_CONVERSATIONID is not set in the flag_b1 */
	UINT32  conv_parameter_mask;    /* ConvParameterMask parameter, bit mask settings, e.g. MAP_CONV_PARMASK_CONVERSATION_NAME. It is ignored if MAP_FLAG_B1_CONVPARAMETERMASK is not set in the flag_b1 */
} tMAP_GetConversationListReqStru;

/* MCE */
/* Parameter of MAP_EVT_OBJECT_DATA_IND and MAP_EVT_ASK4_MESSAGE_DATA_IND */
typedef struct {	
	UINT8				*data;				/* Data chunk of the Object. It is NULL if the Get operation is terminated abnormally. */
	UINT16				length;				/* Size, in bytes, of the data. It is 0 if the Get operation is terminated abnormally. */
	tMAP_Completeness	is_final;			/* MAP_DATA_PARTIAL - More data to be received. MAP_DATA_COMPLETE - It is the final chunk. 
											   MAP_DATA_INCOMPLETE - The object transfer is broken and no more data will be received. */
} tMAP_ObjectDataStru;

/* Parameter of MAP_EVT_GET_FOLDERLIST_COMPLETE */
typedef struct {
	UINT16  result;				/* Result of the GetFolderListing operation, ER_MAP_SUCCESS etc. */
	UINT32  flag;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_FOLDERLISTINGSIZE etc. */
	UINT16  folder_listing_size;/* FolderListingSize parameter. It is ignored if MAP_FLAG_FOLDERLISTINGSIZE is not set in the flag */
} tMAP_GetFolderListCompleteStru;

/* Parameter of MAP_EVT_GET_MESSAGELIST_COMPLETE */
typedef struct {
	UINT16  result;					/* Result of the GetMessageListing operation, ER_MAP_SUCCESS etc. */
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_NEWMESSAGE etc. */
	UINT32  flag_b1;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_B1_FOLDERVERSIONCOUNTER etc. */
	UINT16  message_listing_size;   /* MessageListingSize parameter. It is ignored if MAP_FLAG_MESSAGELISTINGSIZE is not set in the flag */
	UINT8   new_message;            /* NewMessage parameter, one of tMAP_OnOffMask. It is ignored if MAP_FLAG_NEWMESSAGE is not set in the flag */
	UCHAR   *mse_time;         		/* MSETime parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS+(-)HHMM. It is ignored if MAP_FLAG_MSETIME is not set in the flag */
	UCHAR   *database_id;			/* DatabaseIdentifier parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_DATABASEID is not set in the flag */
	UCHAR 	*folder_version_counter;/* FolderVersionCounter parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_B1_FOLDERVERSIONCOUNTER is not set in the flag_b1 */
} tMAP_GetMessageListCompleteStru;

/* Parameter of MAP_EVT_GET_MESSAGE_COMPLETE */
typedef struct {
	UINT16  result;					/* result of the GetMessage operation, ER_MAP_SUCCESS etc. */
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_NEWMESSAGE etc. */
	UINT8   fraction_deliver;       /* FractionDeliver parameter, one of tMAP_FractionDeliver. It is ignored if MAP_FLAG_FRACTIONDELIVER is not set in the flag */
} tMAP_GetMessageCompleteStru;

/* Parameter of MAP_EVT_GET_MAS_INSTANCE_INFORMATION_COMPLETE */
typedef struct {
	UINT16  result;					/* result of the GetMASInstanceInformation operation, ER_MAP_SUCCESS etc. */
	UINT32  flag_b1;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_B1_OWNERUCI etc. */
	UCHAR   *ower_uci;       		/* OwnerUCI parameter, UTF-8 string specifies the UCI of the MAS-instance for which the information is requested. It is ignored if MAP_FLAG_B1_OWERUCI is not set in the flag_b1 */
} tMAP_GetMASInstanceInformationCompleteStru;

/* Parameter of MAP_EVT_ASK4_PUT_REQUEST_IND */
typedef struct {
	tMAP_PutContinueFlag  next_op;          /* Pend, reject or continue the Put operation */
	UINT32                max_data_size;    /* MAX data size expected or available  */
} tMAP_Ask4PutRequestStru;

/* Parameter of MAP_EVT_PUSH_MESSAGE_COMPLETE */
typedef struct {
	UINT16  result;					/* result of the PushMessage operation, ER_MAP_SUCCESS etc. */
	WCHAR   message_handle[16 + 1];	/* Handle assigned by the MSE to the message that was pushed. It is valid only when the result is ER_MAP_SUCCESS. */
} tMAP_PushMessageCompleteStru;

/* Parameter of MAP_EVT_GET_OWNER_STATUS_COMPLETE */
typedef struct {
	UINT16  result;					/* result of the GetOwnerStatus operation, ER_MAP_SUCCESS etc. */
	UINT32  flag;					/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_PRESENCEAVAILABILITY etc. */
	UINT32  flag_b1;				/* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_B1_CHATSTATE etc. */
	UINT8   presence_availability;  /* PresenceAvailability parameter, one of tMAP_Presence. It is ignored if MAP_FLAG_PRESENCEAVAILABILITY is not set in the flag */
	UINT8   chat_state;       		/* ChatState parameter, one of tMAP_ChatState. It is ignored if MAP_FLAG_B1_CHATSTATE is not set in the flag_b1 */
	UCHAR   *presence_text;       	/* PresenceText parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_PRESENCETEXT is not set in the flag */
	UCHAR   *last_activity;       	/* LastActivity parameter, NULL terminated UTF-8 string. It is ignored if MAP_FLAG_LASTACTIVITY is not set in the flag */
} tMAP_GetOwnerStatusCompleteStru;

/* Parameter of MAP_EVT_GET_CONVERSATIONLIST_COMPLETE */
typedef struct {
	UINT16  result;					/* Result of the GetMessageListing operation, ER_MAP_SUCCESS etc. */
	UINT32  flag;				    /* Bit mask specifies the validity of the Application Parameter headers, MAP_FLAG_LISTINGSIZE etc. */
	UINT16  listing_size;   		/* ListingSize parameter. It is ignored if MAP_FLAG_LISTINGSIZE is not set in the flag */
	UCHAR 	*version_counter;		/* ConversationListingVersionCounter parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_CONVLISTING_VERSIONCOUNTER is not set in the flag */
	UCHAR   *database_id;			/* DatabaseIdentifier parameter, a 128-bit value in hex string, NULL terminated. It is ignored if MAP_FLAG_DATABASEID is not set in the flag */
	UCHAR   *mse_time;         		/* MSETime parameter, NULL terminated string, formated in YYYYMMDDTHHMMSS+(-)HHMM or YYYYMMDDTHHMMSS. It is ignored if MAP_FLAG_MSETIME is not set in the flag */
} tMAP_GetConversationListCompleteStru;

/* Parameter of MAP_EVT_EVENT_REPORT_IND */
typedef struct {
    UINT8               mas_instance_id;    /* MASInstanceID of the MAS service sending this object. */
	UINT16				length;				/* Size, in bytes, of the data. It is 0 if the Get operation is terminated abnormally. */
	UINT8				*data;				/* Data chunk of the Object. It is NULL if the Get operation is terminated abnormally. */
	tMAP_Completeness	is_final;			/* MAP_DATA_PARTIAL - More data to be received. MAP_DATA_COMPLETE - It is the final chunk.
											   MAP_DATA_INCOMPLETE - The object transfer is aborted by the client and no more data will be received. */
} tMAP_EventReportStru;

/*---------------------------------------------------------------------------
							Callback Prototype
----------------------------------------------------------------------------*/
/*
Description:
    General callback function to handle confirmations or indications.
Parameters:
    handle    [in] Handle identify the local MAS or MNS entity reporting the
				   event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] The event type.
    param     [in/out] Additional event parameter. Refer to the following
                   specific event callback definition. It is set to NULL if
                   not specified additionally.
*/
typedef void (tMAP_EventCbk)(void *handle, void *context, tMAP_Event_Type event, void *param);

/*
Description:
    Callback function to handle the result of MAS or MNS service initiation.
Parameters:
    handle    [in] Handle identify the local entity reporting the event.
				   It is also identify the new service created.
				   It shall be NULL if the service is failed to initiate.
	context	  [in] Application defined data structure used to identify
				   the ServerStart request.
    event     [in] Shall be MAP_EVT_SERVER_START_COMPLETE.
    param     [in] Result of service initiation.
*/
typedef void (tMAP_ServerStartCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_ServerStartCompleteStru *param);

/*
Description:
    Callback function to handle the result of deregistering MAS or MNS service.
Parameters:
    handle    [in] Handle identify the service being deregistered.
	context	  [in] Application defined data structure used to identify
				   the ServerStart request.
    event     [in] Shall be MAP_EVT_SERVER_STOP_COMPLETE.
    result    [in] Result of service initiation.
*/
typedef void (tMAP_ServerStopCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);

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
    event     [in] Shall be MAP_EVT_CONNECT_COMPLETE.
    param     [in] Result of the connection creation process.
*/
typedef void (tMAP_ConnectCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_ConnectCompleteStru *param);

/*
Description:
    Callback function to handle disconnect complete event.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
                   It is always the handle to the connection disconnected.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be MAP_EVT_DISCONNECT_COMPLETE.
    param     [in] Information of the connection disconnected.
*/
typedef void (tMAP_DisconnectCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_DisconnectCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the ClientStart request.
	This confirmation is used to return the lower handle identifying the
	MAP connection entity. And the APP can then use this handle to cancel
	the connection creation process.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the
				   event. It is also identify the new connection to create.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart request.
    event     [in] Shall be MAP_EVT_CLIENT_START_CFM.
    bd		  [in] Bluetooth device address of the remote MAS or MNS to connect.
*/
typedef void (tMAP_ClientStartCfmCbk)(void *handle, void *context, tMAP_Event_Type event, UINT8 *bd);

/*
Description:
    Callback function to report attribute values of a MAS service discovered.
Parameters:
    bd    	  [in] BD_ADDR of the MAS server being discovered.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_MAS_SERVICE_IND.
    param     [in] Attribute values of the MAS service.
*/
typedef void (tMAP_MASServiceInd)(UINT8 *bd, void *context, tMAP_Event_Type event, tMAP_MASAttributesStru *param);

/*
Description:
    Callback function to handle confirmation to the Discover MAS services
    request.
Parameters:
    bd    	  [in] BD_ADDR of the MAS server being discovered.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_DISCOVER_MAS_COMPLETE.
    result    [in] Result of the Discover MAS services operation.
*/
typedef void (tMAP_DiscoverMASCompleteCbk)(UINT8 *bd, void *context, tMAP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle confirmation to the SetNotificationRegistration
    request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_SET_NOTIFICATION_REGISTRATION_COMPLETE.
    param     [in] Result of the SetNotificationRegistration operation.
*/
typedef void (tMAP_SetNotificationRegistrationCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_NotificationRegCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the UpdateInbox request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_UPDATE_INBOX_COMPLETE.
    result    [in] Result of the UpdateInbox operation.
*/
typedef void (tMAP_UpdateInboxCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle confirmation to the SetPhoneBook request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_SET_FOLDER_COMPLETE.
    result    [in] Result of the SetFolder operation.
*/
typedef void (tMAP_SetFolderCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle indication of the object content.
	The object can be a	Folder-Listing, a Message-listing or a bMessage. 
	During the Get operation, the APP may receive this indication more than once.
	If the MAS rejects the Get request, the APP may not receive this indication.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_OBJECT_DATA_IND.
    param     [in] Chunk of the object content.
			  
*/
typedef void (tMAP_ObjectDataIndCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_ObjectDataStru *param);

/*
Description:
    Callback function to handle indication asking for next Get request.
	This indication allows the application to pend the pulling operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_ASK4_GET_REQUEST_IND.
    next_op   [in/out] On input, it is set to MAP_OP_CONTINUE. On output,
                   1) Keep it unchanged, the MAP Stack sends Get request to the
                   MAS immediately after this callback returns.
                   2) Change it to MAP_OP_ABORTING, the MAP Stack sends Abort
                   request to the MAS immediately after this callback returns. 
                   The pulling operation is then terminated.
                   3) Change it to MAP_OP_PENDING, the MAP Stack waits for the
                   application to call MAP_GetContinueReq to send Get request or
                   to call MAP_AbortReq to terminate the pulling operation.
*/
typedef void (tMAP_Ask4GetReqIndCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetContinueFlag *next_op);

/*
Description:
    Callback function to handle confirmation to the GetFolderListing request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_GET_FOLDERLIST_COMPLETE.
    param     [in] Result of the GetFolderListing operation.
*/
typedef void (tMAP_GetFolderListCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetFolderListCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the GetMessageListing request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_GET_MESSAGELIST_COMPLETE.
    param     [in] Result of the GetMessageListing operation.
*/
typedef void (tMAP_GetMessageListCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetMessageListCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the GetMessage request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_GET_MESSAGE_COMPLETE.
    param     [in] Result of the GetMessage operation.
*/
typedef void (tMAP_GetMessageCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetMessageCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the GetMASInstanceInformation request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_GET_MAS_INSTANCE_INFORMATION_COMPLETE.
    param     [in] Result of the GetMASInstanceInformation operation.
*/
typedef void (tMAP_GetMASInstanceInformationCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetMASInstanceInformationCompleteStru *param);

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
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request. It also identify the message object to be pushed.
    event     [in] Shall be MAP_EVT_ASK4_PUT_REQUEST_IND.
    param     [in/out]
                   param->next_op:
                       On input it is set to MAP_OP_CONTINUE. On output,
                       1) Keep it unchanged, the MAP Stack asks for the message
                       object data using MAP_EVT_ASK4_MESSAGE_DATA_IND event and
                       then sends Put request to the MAS immediately after this
                       callback returns.
                       2) Change it to MAP_OP_ABORTING, the MAP Stack sends Abort
                       request to the MAS immediately after this callback returns.
                       The pushing operation is then terminated.
                       3) Change it to MAP_OP_PENDING, the MAP Stack waits for the
                       application to prepare the data to be pushed. The application
                       shall then call MAP_PutContinueReq to notify the MAP Stack
                       when the data is ready to send, or MAP_AbortReq
                       to command the MAP Stack to terminate the pushing operation.

                   param->max_data_size:
                       On input it is set to the MAX size of object content that
                       could be sent in one Put request. The value is for reference
                       only. The application can ignore it.
                       On output,
                       1) If param->next_op is kept MAP_OP_CONTINUE, the value
                       of max_data_size shall specify the size of object content
                       ready for the MAP Stack to send. The MAP Stack will then
                       try to send as much of these data as possible until the
                       OBEX MTU limit is reached.
                       The value 0 specifies that no more content to read and the
                       Final Put request will be sent.
                       2) If param->next_op is changed to either MAP_OP_ABORT or
                       MAP_OP_PENDING, the param->max_data_size value is ignored.
*/
typedef void (tMAP_Ask4PutReqIndCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_Ask4PutRequestStru *param);

/*
Description:
    Callback function to get message content from the application.
    Each time after getting possitive confirmation to the MAP_EVT_ASK4_PUT_REQUEST_IND
    from the APP, the MAP Stack tries to require and push as much data as possible
    until the OBEX MTU limit is reached, or the data ready for this Put request
    have all be sent, or the param->is_final is set to MAP_DATA_COMPLETE.
	If the MAS rejects the Put request, the APP may not receive this indication.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request. It also identify the message object to read.
    event     [in] Shall be MAP_EVT_ASK4_MESSAGE_DATA_IND.
    param     [in/out]Chunk of the message content.
                   param->data: Specifies the buffer to return the message content.
                   param->length: On input, it specifies the size of param->data.
                   On output, it shall be changed to the size of message content,
                   in bytes, copied to param->data. 0 bytes copied means no more
                   content to read.
                   param->is_final: On input, it is set to MAP_DATA_PARTIAL. On
                   output, is shall kept MAP_DATA_PARTIAL until the final chunk.
			  
*/
typedef void (tMAP_Ask4MessageDataIndCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_ObjectDataStru *param);

/*
Description:
    Callback function to handle confirmation to the PushMessage request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_PUSH_MESSAGE_COMPLETE.
    param     [in] Result of the PushMessage operation.
*/
typedef void (tMAP_PushMessageCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_PushMessageCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the SetMessageStatus request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_SET_MESSAGE_STATUS_COMPLETE.
    result    [in] Result of the SetMessageStatus operation.
*/
typedef void (tMAP_SetMessageStatusCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle confirmation to the SetOwnerStatus request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_SET_OWNER_STATUS_COMPLETE.
    result    [in] Result of the SetOwnerStatus operation.
*/
typedef void (tMAP_SetOwnerStatusCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle confirmation to the GetOwnerStatus request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_GET_OWNER_STATUS_COMPLETE.
    param     [in] Result of the GetOwnerStatus operation.
*/
typedef void (tMAP_GetOwnerStatusCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetOwnerStatusCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the GetConversationListing request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_GET_CONVERSATIONLIST_COMPLETE.
    param     [in] Result of the GetConversationListing operation.
*/
typedef void (tMAP_GetCoversationListCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_GetConversationListCompleteStru *param);

/*
Description:
    Callback function to handle confirmation to the SetNotificationFilter request.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be MAP_EVT_SET_NOTIFICATION_FILTER_COMPLETE.
    result    [in] Result of the SetNotificationFilter operation.
*/
typedef void (tMAP_SetNotificationFilterCompleteCbk)(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);

/*
Description:
    Callback function to handle indication of the MAP-Event-Report object content.
	During the SendEvent operation, the APP may receive this indication more than
	once. This event is sent to both the MNS service application and the MAS client
	application that controls the connection with the MAS service sending this
	MAP-Event-Report object.
Parameters:
    handle    [in] Handle identify the local MAP entity reporting the event. 
              For the MNS service application, it is the handle identifing the MNS
              connection receiving the object.
              For the MAS client application, it is the handle identifying the
              connection to the MAS service sending the object.
	context	  [in] Application defined data structure used to identify
				   the callback.
    event     [in] Shall be MAP_EVT_EVENT_REPORT_IND.
    param     [in] Chunk of the MAP-Event-Report object content.
			  
*/
typedef void (tMAP_EventReportIndCbk)(void *handle, void *context, tMAP_Event_Type event, tMAP_EventReportStru *param);


/*---------------------------------------------------------------------------
							Interfaces
----------------------------------------------------------------------------*/
/* Common */
void MAP_Init(void);
void MAP_Done(void);
void MAP_DisconnectReq(void *con_handle);

/* MCE */
#ifdef CONFIG_MAP_MCE
void MAP_DiscoverMAS(UINT8 *bd, void *context, UINT16 conditions, tMAP_EventCbk *cbk);
void MAP_MNSServerStartReq(void *context, tMAP_MNSAttributesStru *mns_info, tMAP_EventCbk *cbk);
void MAP_MNSServerStopReq(void);
void MAP_MASClientStartReq(void *context, UINT8 *bd, tMAP_MASAttributesStru *mas_info, tMAP_EventCbk *cbk);
#define MAP_ClientStopReq   MAP_DisconnectReq
void MAP_SetNotificationRegistrationReq(void *cli_handle, void *context, tMAP_SetNotificationRegistrationCompleteCbk *cbk);
void MAP_UpdateInboxReq(void *cli_handle, void *context, tMAP_UpdateInboxCompleteCbk *cbk);
void MAP_SetFolderReq(void *cli_handle, void *context, WCHAR *folder, tMAP_SetFolderCompleteCbk *cbk);
/* Set root folder */
#define MAP_SetRootFolderReq(_hdl, _cont, _cbk)	MAP_SetFolderReq(_hdl, _cont, NULL, _cbk)
/* Set parent folder */
#define MAP_SetParentFolderReq(_hdl, _cont, _cbk) MAP_SetFolderReq(_hdl, _cont, (WCHAR *)g_map_parent_folder, _cbk)
/* Set child folder */
#define MAP_SetChildFolderReq				MAP_SetFolderReq
void MAP_GetFolderListReq(void *cli_handle, void *context, tMAP_GetFolderListReqStru *req_param, tMAP_EventCbk *cbk);
void MAP_GetMessageListReq(void *cli_handle, void *context, WCHAR *folder, tMAP_GetMessageListReqStru *req_param, tMAP_EventCbk *cbk);
void MAP_GetConversationMessageListReq(void *cli_handle, void *context, UCHAR *conversation_id, tMAP_GetMessageListReqStru *req_param, tMAP_EventCbk *cbk);
void MAP_GetMessageListByHandleReq(void *cli_handle, void *context, UCHAR *msg_handle, tMAP_GetMessageListReqStru *req_param, tMAP_EventCbk *cbk);
void MAP_GetMessageReq(void *cli_handle, void *context, WCHAR *msg_handle, tMAP_GetMessageReqStru *req_param, tMAP_EventCbk *cbk);
void MAP_GetMasInstanceInformationReq(void *cli_handle, void *context, UINT8 mas_instance_id, tMAP_EventCbk *cbk);
void MAP_GetContinueReq(void *cli_handle);
void MAP_SetMessageStatusReq(void *cli_handle, void *context, WCHAR *msg_handle, tMAP_MessageStatus msg_status, tMAP_SetMessageStatusCompleteCbk *cbk);
void MAP_SetExtendedDataReq(void *cli_handle, void *context, WCHAR *msg_handle, UINT8 *ext_data_value, tMAP_SetMessageStatusCompleteCbk *cbk);
void MAP_PushMessageReq(void *cli_handle, void *context, WCHAR *folder, tMAP_PushMessageReqStru *req_param, tMAP_EventCbk *cbk);
void MAP_PutContinueReq(void *cli_handle, UINT32 max_data_size);
void MAP_AbortReq(void *cli_handle);
void MAP_SetOwnerStatusReq(void *cli_handle, void *context, tMAP_SetOwnerStatusReqStru *req_param, tMAP_SetOwnerStatusCompleteCbk *cbk);
void MAP_GetOwnerStatusReq(void *cli_handle, void *context, UCHAR *conversation_id, tMAP_GetOwnerStatusCompleteCbk *cbk);
void MAP_SetNotificationFilterReq(void *cli_handle, void *context, UINT32 filter_mask, tMAP_SetNotificationFilterCompleteCbk *cbk);
#endif

/* MSE */
#ifdef CONFIG_MAP_MSE
#endif

#endif
