/*-----------------------------------------------------------------------------
Module Name:
	obex_ui.h
Abstract:
	This file includes all the OBEX UI interface Macros/Values/Structures and function definations.
Author:   
	Wang Yao
-----------------------------------------------------------------------------*/

#ifndef OBEX_UIH
#define OBEX_UIH

extern const WCHAR g_obex_parent_folder[3];
extern const char g_obex_http_length[16];

typedef void (OBEX_CbkFuc)(UINT8 ev, void *arg);/* callback to upper profile */

#define OBEX_FINALBIT 								0x80
#define OBEX_SESSION_ID_LEN							16
#define OBEX_DECODE_NOTEXIST						0xFFFFFFFE

/* All Unicode headers are encoded using UTF-16 format with the bytes sent in network order */
enum {
	OBEX_HI_COUNT = 0xC0,/* Number of objects (used by Connect) */
	OBEX_HI_NAME = 0x01,/* Name of the object (often a file name) */
	OBEX_HI_TYPE = 0x42,/* Type of object - e.g. text, html, binary, manufacturer specific */
	OBEX_HI_LENGTH = 0xC3,/* The length of the object in bytes */
	OBEX_HI_TIME_ISO8601 = 0x44,/* Date/time stamp - ISO 8601 version - preferred */
	OBEX_HI_TIME_4BYTES = 0xC4,/* Date/time stamp - 4 byte version (for compatibility only) */
	OBEX_HI_DESCRIPTION = 0x05,/* Text description of the object */
	OBEX_HI_TARGET = 0x46,/* Name of service that operation is targeted to */
	OBEX_HI_HTTP = 0x47,/* An HTTP 1.x header */
	OBEX_HI_BODY = 0x48,/* A chunk of the object body. */
	OBEX_HI_ENDOF_BODY = 0x49,/* the final chunk of the object body */
	OBEX_HI_WHO = 0x4A,/* Identifies the OBEX application, used to tell if talking to a peer */
	OBEX_HI_CONNECTION_ID = 0xCB,/* An identifier used for OBEX connection multiplexing */
	OBEX_HI_APP_PARAMETERS = 0x4C,/* Extended application request & response information */
	OBEX_HI_AUTH_CHALLENGE = 0x4D,/* Authentication digest-challenge */
	OBEX_HI_AUTH_RESPONSE = 0x4E,/* Authentication digest-response */
	OBEX_HI_CREATOR_ID = 0xCF,/* Indicates the creator of an object */
	OBEX_HI_WAN_UUID = 0x50,/* Uniquely identifies the network client (OBEX server) */
	OBEX_HI_OBJECT_CLASS = 0x51,/* OBEX Object class of object */
	OBEX_HI_SESSION_PARAMETERS = 0x52,/* Parameters used in session commands/responses */
	OBEX_HI_SESSION_SEQUENCE_NUMBER = 0x93,/* Sequence number used in each OBEX packet for reliability, The Session-Sequence-Number must be the first header. */
	OBEX_HI_ACTION_ID = 0x94,/* Specifies the action to be performed (used in ACTION operation) */
	OBEX_HI_DESTNAME = 0x15,/* The destination object name (used in certain ACTION operations) */
	OBEX_HI_PERMISSIONS = 0xD6,/* 4-byte bit mask for setting permissions */
	OBEX_HI_SINGLE_RESPONSE_MODE = 0x97,/* 1-byte value to setup Single Response Mode (SRM) */
	OBEX_HI_SINGLE_RESPONSE_MODE_PARAMETERS = 0x98,/* 1-byte value for setting parameters used during Single Response Mode (SRM) */
	/* 0x19 to 0x2F Reserved for future use */
	/* 0x30 to 0x3F User defined */
	/* This range includes all combinations of the upper 2 bits */
};

/* Request Code values */
enum {
	OBEX_OP_CONNECT = 0x80,
	OBEX_OP_DISCONNECT = 0x81,
	OBEX_OP_PUT = 0x02,
	OBEX_OP_PUT_FINAL = 0x82,/* 0x82 when FINALBIT set */
	OBEX_OP_GET = 0x03,
	OBEX_OP_GET_FINAL = 0x83,/* 0x83 when FINALBIT set */
	OBEX_OP_ABORT = 0xFF,
	OBEX_OP_SETPATH = 0x85,
	OBEX_OP_SESSION = 0x87,
	OBEX_OP_ACTION = 0x06,
	OBEX_OP_ACTION_FINAL = 0x86,
};

/* Response Code values, with final bit set is | 0x80, OBEX 1.x always set final_bit */
enum {
	/* 0x00 to 0x0F None, Reserved */
	OBEX_RSP_CONTINUE = 0x90,/* 0x10  (0x90)  100  Continue */
	OBEX_RSP_OK = 0xA0,/* 0x20  (0xA0)     200  OK, Success */	
	OBEX_RSP_CREATED = 0xA1,/* 0x21  (0xA1)  201  Created */	
	OBEX_RSP_ACCEPTED = 0xA2,/* 0x22  (0xA2)  202  Accepted */	
	OBEX_RSP_NON_AUTHORITATIVE_INFO = 0xA3,/* 0x23  (0xA3)  203  Non-Authoritative Information */	
	OBEX_RSP_NO_CONTENT = 0xA4,/* 0x24  (0xA4)  204  No Content */
	OBEX_RSP_RESET_CONTENT = 0xA5,/* 0x25  (0xA5)  205  Reset Content */	
	OBEX_RSP_PARTIAL_CONTENT = 0xA6,/* 0x26  (0xA6)  206  Partial Content  */	
	OBEX_RSP_MULTIPLE_CHOICES = 0xB0,/* 0x30  (0xB0)  300  Multiple Choices  */	
	OBEX_RSP_MOVED_PERMANENTLY = 0xB1,/* 0x31  (0xB1)  301  Moved Permanently */	
	OBEX_RSP_MOVED_TEMPORARILY = 0xB2,/* 0x32  (0xB2)  302  Moved temporarily */	
	OBEX_RSP_SEE_OTHER = 0xB3,/* 0x33  (0xB3)  303  See Other */	
	OBEX_RSP_NOT_MODIFIED = 0xB4,/* 0x34  (0xB4)  304  Not modified */	
	OBEX_RSP_USE_PROXY = 0xB5,/* 0x35  (0xB5)  305  Use Proxy */	
	OBEX_RSP_BAD_REQUEST = 0xC0,/* 0x40  (0xC0)  400  Bad Request - server couldn`t understand request */	
	OBEX_RSP_UNAUTHORIZED = 0xC1,/* 0x41  (0xC1)  401  Unauthorized */	
	OBEX_RSP_PAYMENT_REQUIRED = 0xC2,/* 0x42  (0xC2)  402  Payment required */	
	OBEX_RSP_FORBIDDEN = 0xC3,/* 0x43  (0xC3)  403  Forbidden - operation is understood but refused */	
	OBEX_RSP_NOT_FOUND = 0xC4,/* 0x44  (0xC4)  404  Not Found */	
	OBEX_RSP_METHOD_NOTALLOWED = 0xC5,/* 0x45  (0xC5)  405  Method not allowed */	
	OBEX_RSP_NOT_ACCEPTABLE = 0xC6,/* 0x46  (0xC6)  406  Not Acceptable */	
	OBEX_RSP_PROXY_AUTHENTICATION_REQUIRED = 0xC7,/* 0x47  (0xC7)  407  Proxy Authentication required */		
	OBEX_RSP_REQUEST_TIMEOUT = 0xC8,/* 0x48  (0xC8)  408  Request Time Out */	
	OBEX_RSP_CONFLICT = 0xC9,/* 0x49  (0xC9)  409  Conflict */	
	OBEX_RSP_GONE = 0xCA,/* 0x4A  (0xCA)  410  Gone */	
	OBEX_RSP_LENGTH_REQUIRED = 0xCB,/* 0x4B  (0xCB)  411  Length Required */	
	OBEX_RSP_PRECONDITION_FAILED = 0xCC,/* 0x4C  (0xCC)  412  Precondition failed */		
	OBEX_RSP_REQUESTED_ENTITY_TOOLARGE = 0xCD,/* 0x4D  (0xCD)  413  Requested entity too large */	
	OBEX_RSP_REQUEST_URL_TOOLARGE = 0xCE,/* 0x4E  (0xCE)  414  Request URL too large */	
	OBEX_RSP_UNSUPPORTED_MEDIATYPE = 0xCF,/* 0x4F  (0xCF)  415  Unsupported media type */	
	OBEX_RSP_INTERNAL_SERVER_ERROR = 0xD0,/* 0x50  (0xD0)  500  Internal Server Error */
	OBEX_RSP_NOT_IMPLEMENTED = 0xD1,/* 0x51  (0xD1)  501  Not Implemented */	
	OBEX_RSP_BAD_GATEWAY = 0xD2,/* 0x52  (0xD2)  502  Bad Gateway */		
	OBEX_RSP_SERVICE_UNAVAILABLE = 0xD3,/* 0x53  (0xD3)  503  Service Unavailable */	
	OBEX_RSP_GATEWAY_TIMEOUT = 0xD4,/* 0x54  (0xD4)  504  Gateway Timeout */	
	OBEX_RSP_HTTP_VERSION_NOTSUPPORTED = 0xD5,/* 0x55  (0xD5)  505  HTTP version not supported */
	OBEX_RSP_DATABASE_FULL = 0xE0,/* 0x60 (0xE0)  - - -  Database Full */	
	OBEX_RSP_DATABASE_LOCKED = 0xE1,/* 0x61 (0xE1)  - - -  Database Locked */
};

enum {/* Authentication Header, Digest Challenge Tag values */
	OBEX_AUTHTAG_CHALLENGE_NONCE = 0x00,/* 16bytes String of bytes representing the request digest. */
	OBEX_AUTHTAG_CHALLENGE_OPTIONS = 0x01,/* N bytes, User ID string of length n. Max size is 20 bytes. */
	OBEX_AUTHTAG_CHALLENGE_REALM = 0x02,/* 16bytes, The nonce sent in the digest challenge string. */
};

enum {/* Authentication Header, Digest Response Tag values */
	OBEX_AUTHTAG_RESPONSE_REQUEST_DIGEST = 0x00,/* 16bytes String of bytes representing the request digest. */
	OBEX_AUTHTAG_RESPONSE_USERID = 0x01,/* N bytes, User ID string of length n. Max size is 20 bytes. */
	OBEX_AUTHTAG_RESPONSE_NONCE = 0x02,/* 16bytes, The nonce sent in the digest challenge string. */
};

enum {/* Session Header Tag values */
	OBEX_SESSIONTAG_DEVICE_ADDRESS = 0x00,/* For Bluetooth this is the 48-bit Bluetooth address. */

	/* The nonce is a value provided by the device, which will be used  in  the 
	creation of the session ID. This number must be unique for  each  session 
	created by  the device. One method for creating the nonce is to start with a 
	random  number  then increment the value each time a new session is 
	created. The Nonce should be at least 4 bytes and at most 16 bytes in size. */
	OBEX_SESSIONTAG_NONCE = 0x01,

	/* Session ID. This is a 16-byte value, which is formed by taking  the  device 
	address and nonce from the client and server and running the MD5 algorithm 
	over  the resulting string. The Session ID is created as follows: MD5(Client 
	Device Address Client Nonce Server Device Address Server Nonce) */
	OBEX_SESSIONTAG_SESSIONID = 0x02,

	/* This is a one-byte value sent by the server, which indicates  the  next 
	sequence number expected when the session is resumed. */
	OBEX_SESSIONTAG_NEXT_SEQ_NUM = 0x03,

	/* This  is a 4-byte value that contains the number of seconds a session can be 
	in  suspend  mode  before it is considered closed. The value of 0xffffffff 
	indicates a timeout of infinity. This is the default timeout. If a device does not 
	send  a  timeout  field then it can be assumed that the desired timeout is 
	infinity. The timeout in affect is the smallest timeout sent  by  the  client  or 
	server.  */
	OBEX_SESSIONTAG_TIMEOUT = 0x04,

	/* The  session  opcode is a 1-byte value sent by the client to indicate the 
	Session command that is to be performed. This tag-length-value is only sent 
	in  SESSION  commands and must be the first tag in the  Session-
	Parameters  header.    The session opcode definitions are defined in the 
	bulleted list below. */
	OBEX_SESSIONTAG_OPCODE = 0x05,

	/* This is a 4-byte unsigned integer value containing the number of bytes of 
	object data that have been successfully received during a portion of object transfer.  
	This value shall be used to determine the point in the object where the data flow 
	shall continue when the session and its subsequent suspended operation are resumed.	
		If a single object exceeds 4 gigabytes -1 in length, its size cannot be represented 
	using this TAG.  Instead, an HTTP content-length header should be used, which is ASCII 
	encoded decimal and can represent arbitrarily large values. However, implementations 
	that cannot handle such large objects are not required to recognize the HTTP header." */
	OBEX_SESSIONTAG_OBJECT_OFFSET = 0x06,
	/* 0x07~0xff are reserved */
};

enum {/* Session Opcode Definitions */
	OBEX_SESSIONOP_CREATE = 0x00,
	OBEX_SESSIONOP_CLOSE = 0x01,
	OBEX_SESSIONOP_SUSPEND = 0x02,
	OBEX_SESSIONOP_RESUME = 0x03,
	OBEX_SESSIONOP_SETTIMEOUT = 0x04,
	/* 0x05-0xFF are reserved */
};

/* Action Identifier */
enum {
	OBEX_ACTIONID_COPY = 0x00,
	OBEX_ACTIONID_MOVE_RENAME = 0x01,
	OBEX_ACTIONID_SET_PERMISSIONS = 0x02,
	/* 0x03 - 0x7F Reserved for assignment by OBEX */
	/* 0x80 - 0xFF An area of the Action Identifier space, which is reserved for use in vendor specific applications. */
};

enum {/* Permissions header, flag mask. */
	/* Read. When this bit is set to 1, reading permission is granted. For a file object, the OBEX 
		Client may use ACTION/Copy or GET on the file. For a folder object, the OBEX Client may use 
		ACTION/Copy or SETPATH on the folder. To do ACTION/Move client needs to have both 
		Read and Delete permission  to  the source file/folder.  */
	OBEX_PERMISSION_FLAG_READ = 0x01,
	/* Write. When this bit is set to 1, writing permission is granted. For a file object, the OBEX Client 
		may use PUT on the file. To do ACTION/Move client needs to have both Read and Delete 
		permission to the source file/folder. */
	OBEX_PERMISSION_FLAG_WRITE = 0x02,
	/* Delete. When  this  bit is set to 1, deletion permission is granted. For a file/folder object, the 
		OBEX Client may use PUT-delete. */
	OBEX_PERMISSION_FLAG_DELETE = 0x04,
	/* bit 3~6 reserved for future use */
	/* Modify Permissions. When this bit is set to 1 the file access permissions can be changed. For a 
		file/folder object, the OBEX Client may use ACTION/Set Object Permissions */
	OBEX_PERMISSION_FLAG_MODIFY_PERMISSIONS = 0x80,
};

enum {/* Event callback */
	OBEX_EV_CALC_HEADER_LEN = 0x01,
	OBEX_EV_COMPOSE_HEADER,				/* 0x02 */
	OBEX_EV_CONNECT_CFM,				/* 0x03, client, after Connect_Req, this cfm used to tell profile tl_handle for cancel the connection before it has established */
	OBEX_EV_CONNECT_COMPLETE,			/* 0x04, result indicate by specified macros such as ER_OBEX_SUCCESS */
	OBEX_EVA4_CONNECT_REQ,				/* 0x05, only in Auth */
	OBEX_EVA4_CONNECT_RSP,				/* 0x06 */	
	OBEX_EV_DISCONNECT_COMPLETE,		/* 0x07 */
	
	OBEX_EVA4_ABORT_RSP,				/* 0x08 */
	OBEX_EVA4_PUT_REQ,					/* 0x09 */
	OBEX_EV_PUT_IND, 					/* 0x0A, include OBEX_EVA4_PUT_RSP */
	OBEX_EV_GET_CFM_CONTINUE,			/* 0x0B */	
	OBEX_EVA4_GET_REQ,					/* 0x0C */
	OBEX_EVA4_GET_RSP,					/* 0x0D */
	OBEX_EV_OP_COMPLETE,				/* 0x0E, include GET/PUT, both server and client, aborted or completed */
	OBEX_EV_SETPATH_CFM,				/* 0x0F */
	OBEX_EVA4_SETPATH_RSP,				/* 0x10 */
	OBEX_EVA4_ACTION_REQ,				/* 0x11 */
	OBEX_EVA4_ACTION_RSP,				/* 0x12 */
	OBEX_EV_SERVER_START_CFM,			/* 0x13 */
	OBEX_EV_SESSION_CREATE_IND,
	OBEX_EV_SESSION_SUSPEND_COMPLETE,
	OBEX_EV_SESSION_SUSPEND_IND,
	OBEX_EV_SESSION_RESUME_COMPLETE,
	OBEX_EV_SESSION_RESUME_IND,
};

enum {/* Type in struct OBEX_UITLVStru */
	OBEX_TLVTYPE_CONNECT_TARGET,
	OBEX_TLVTYPE_CONNECT_PRIKEY,/* to calculate nonce */
	OBEX_TLVTYPE_CONNECT_OPTION,/* only 1 byte */
	OBEX_TLVTYPE_CONNECT_NONCE,
	OBEX_TLVTYPE_CONNECT_REQUEST_DIGEST,
	OBEX_TLVTYPE_CONNECT_REALM,
	OBEX_TLVTYPE_CONNECT_USERID,
	OBEX_TLVTYPE_CONNECT_PASSWORD_CHALLENGE,/* used to calc Req_Dig when challenge */
	OBEX_TLVTYPE_CONNECT_PASSWORD_RESPONSE,/* used to calc Req_Dig when encode Auth_Rsp header */
	OBEX_TLVTYPE_OBJECT_OFFSET,
	OBEX_TLVTYPE_RECEIVE_OBJECT_OFFSET,
};

enum {/* OBEX Transport type */
	OBEX_TL_RFCOMM = (0x00 << 24),
	OBEX_TL_L2CAP = (0x01 << 24),
};
#define OBEX_TLTYPE_BITS    (0xFF << 24)

struct OBEX_UITLVStru {/* for information exchange between profile and OBEX, variable length */
	UINT16 total_len;
	UINT16 len;/* as offset...new TLV will put on *(data+len) */
	UINT8 data[1];/* Type(1)+Len(1)+Value(Len)... */
};

struct OBEX_HeaderStru {/* Always located on bottom of a main structure */
	UINT16 header_num;
	UINT8 headers_id[1];/* variable length */
};

struct OBEX_OP_CompleteStru {/* Arg in OBEX_EV_OP_COMPLETE */
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_id;
	UINT8 rsp_code;
	UINT8 is_aborted;
	struct BuffStru *pdata;
};

struct OBEX_CalcHeaderLenStru {/* param of OBEX_EV_CALC_HEADER_LEN */
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	struct OBEX_HeaderStru *headers;
	UINT16 obex_packetlen;/* Profile fill to tell OBEX */
};

struct OBEX_ComposeHeaderStru {
	UINT32 handle;/* connection handle */
	UINT8 op_id;
	UINT8 op_code;/* profile should return FINAL_BIT or rsp_code info */	
	UINT8 type;/* header type(HI) */
	UINT16 index;/* header index */
	UINT8 *pdata;/* header value pointer */
	UINT8 *offset_tag;
	UINT16 tl_remained_len;/* remained TL_packet size, ReadONLY */
	UINT16 obex_remained_len;/* app should return MOP_consumed len, if incomplete, return 0xFFFF or any value larger than what OBEX gave */
};

enum {/* flags in struct OBEX_SetPathStru */
	OBEX_SETPATH_FLAG_BACKUP_LEVEL = 0x01,/* 0  backup a level before applying name (equivalent to ../ on many systems) */
	OBEX_SETPATH_FLAG_DONOT_CREATE_FOLDER = 0x02,/* 1  Don`t create folder if it does not exist, return an error instead. */
};

struct OBEX_SetPathStru {/* arg in OBEX_EVA4_SETPATH_RSP/OBEX_EV_SETPATH_CFM, OBEX_SetPath_Rsp and OBEX_SetPath_Req */
	UINT32 handle;
	UINT8 flags;/*only used in OBEX_SetPath_Ind and OBEX_SetPath_Req */
	UINT8 rsp_code;/* only used in OBEX_SetPath_Rsp */
	UINT8 op_id;/* Send, only used in OBEX_SetPath_Rsp/Req */
	struct BuffStru *packet;/* Receive, lower to upper */
	struct OBEX_HeaderStru headers;/* Send, Upper to lower */
};

struct OBEX_SessionCompleteIndStru {/* arg for OBEX_EV_SESSION_RESUME_COMPLETE, OBEX_EV_SESSION_SUSPEND_COMPLETE
									OBEX_EV_SESSION_REUSME_IND, and OBEX_EV_SESSION_SUSPEND_IND */
	UINT32 handle;
	UINT8 session_id[OBEX_SESSION_ID_LEN];
	UINT8 result;/* for client */
};

struct OBEX_ConnectCompleteStru {/* arg in OBEX_EV_CONNECT_COMPLETE and OBEX_EV_CONNECT_CFM */
	UINT8 bd[BD_ADDR_LEN];
	UINT8 session_id[OBEX_SESSION_ID_LEN];/* valid when reliable connection */
	UINT32 server_handle;/* L2CAP PSM */
	UINT32 handle;/* L2CAP CID */
	UINT16 result;/* e.g: ER_OBEX_SUCCESS */
	struct BuffStru *packet;/* Maybe exist, maybe not */
};

struct OBEX_DisconnectCompleteStru {/* Used in OBEX_ConnectionFI_Done, arg in OBEX_EV_DISCONNECT_COMPLETE */
	UINT32 handle;
	UINT16 reason;
	struct BuffStru *packet;/* Maybe exist, maybe not */
};

#ifdef CONFIG_OBEX_CLIENT
struct OBEX_ConnectReqStru {/* arg in OBEX_Connect_Req */
	UINT32 handle;/* L2CAP CID */
	UINT32 server_handle;/* Server PSM for l2cap, server channel for rfcomm */	
	UINT8 bd[BD_ADDR_LEN];/* TL connection? */
	UINT16 max_packetlen_in; /* Profile specific incoming MAX OBEX packet length. 0 to use the OBEX default value */
	UINT8 enable_srm;/* Connection SRM. GOEP20:SRM headers shall not be sent in CONNECT request or response packets.*/
	void *client_cbk;
	struct OBEX_UITLVStru *param;
	struct OBEX_HeaderStru headers;
};

struct OBEX_ConnectAuthReqStru {/* arg in OBEX_EVA4_CONNECT_REQ, when server want to auth_challenge */
	UINT32 handle;/* L2CAP CID */
	UINT32 server_handle;/* L2CAP PSM */
	UINT8 bd[BD_ADDR_LEN];
	struct OBEX_UITLVStru *param;/* Deliver Auth-Challenge TLV */
	struct BuffStru *packet;
};

struct OBEX_AbortReqStru {/* arg in OBEX_Abort_Req */
	UINT32 handle;/* Add connection_handle */
	struct OBEX_HeaderStru headers;
};

struct OBEX_Put_ActiveReqStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* only OBEX_OP_PUT or OBEX_OP_PUT_FINAL, when no headers exist, this is useful */
	UINT8 op_id;/* Allocated by profile, mainly used in PUT/GET */
	UINT8 enable_srm;
	struct OBEX_HeaderStru headers;
};

struct OBEX_Put_PassiveReqStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* only OBEX_OP_PUT or OBEX_OP_PUT_FINAL, when no headers exist, this is useful */
	UINT8 op_id;/* equal to the one in struct OBEX_Put_ActiveReqStru */
	UINT8 srmp_00;/* Client request for additional response */
	UINT8 *offset_tag;/* length and value of object offset, in 'tlv' format, used in session resume */	
	struct BuffStru *pdata;/* used in OBEX_EVA4_PUT_REQ */
	struct OBEX_HeaderStru headers;
};

struct OBEX_Get_ActiveReqStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* only OBEX_OP_GET or OBEX_OP_GET_FINAL, when no headers exist, this is useful */
	UINT8 op_id;/* Allocated by profile, mainly used in PUT/GET */
	UINT8 enable_srm;
	UINT8 srmp_01;
	UINT8 no_timer;/* For BPP GetEvent, no op_mem time out control */
	struct OBEX_HeaderStru headers;
};

struct OBEX_Get_PassiveReqStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* only OBEX_OP_GET or OBEX_OP_GET_FINAL, when no headers exist, this is useful */
	UINT8 op_id;/* equal to the one in struct OBEX_Get_ActiveReqStru */
	UINT8 srmp_01;/* Client ask server to wait */
	struct BuffStru *pdata;
	struct OBEX_HeaderStru headers;
};

struct OBEX_Get_CfmStru {/* Arg in OBEX_EV_GET_CFM_CONTINUE */
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_id;
	struct BuffStru *pdata;
};

struct OBEX_DisconnectStru {/* arg in OBEXC_Disconnect_Req */
	UINT32 handle;/* add connection handle to match */
	struct OBEX_HeaderStru headers;
};

struct OBEX_SessionSetTimeOutStru {/* arg in OBEX_Session_SetTimeOut_Req */
	UINT8 session_id[OBEX_SESSION_ID_LEN];
//	UINT8 bd[BD_ADDR_LEN];
//	UINT8 nonce_len;
//	UINT8 nonce[16];/* 4~16 bytes */
	UINT32 timeout;
};

struct OBEX_SessionStru {/* arg in OBEX_ClientSessionCloseReq, and OBEX_ClientSessionSuspendReq */
	UINT8 session_id[16];
};

struct OBEX_SessionResumeStru {/* arg in OBEX_ClientSessionResumeReq */
	UINT32 server_handle;/* can change transport layer */
	BtUINT64Stru obj_offset;
	UINT8 session_id[OBEX_SESSION_ID_LEN];
};

struct OBEX_ActionReqStru {/* arg in OBEX_Action_Req */
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* OBEX_OP_ACTION or OBEX_OP_ACTION_FINAL */
	UINT8 op_id;
	UINT8 action_id;/* e.g: OBEX_ACTIONID_COPY */
	/* Name, DestName, Permissions headers input by what? */
	struct OBEX_HeaderStru headers;
};

struct OBEX_ActionCfmStru {
	UINT32 handle;
	UINT8 rsp_code;
};
#endif

#ifdef CONFIG_OBEX_SERVER
struct OBEX_ConnectIndStru {/* Server used in OBEX_EVA4_CONNECT_RSP */
	UINT32 handle;/* connection handle, L2CAP CID */
	UINT32 server_handle;/* L2CAP PSM */
	UINT8 bd[BD_ADDR_LEN];
	UINT8 enable_session;
	UINT8 auth_challenge;/* Client is challenge us, profile must fill tlv. */
	struct BuffStru *packet;/* Target decode for profile? */
	struct OBEX_UITLVStru *param;/* Target?, Client_UserID, Option/Realm */
};

struct OBEX_ConnectRspStru {/* arg in OBEX_Connect_Rsp */
	UINT32 handle;
	UINT8 rsp_code;
	UINT8 enable_srm;/* Connection SRM . GOEP20:SRM headers shall not be sent in CONNECT request or response packets.*/
	struct OBEX_UITLVStru *param;/* PriKey/Option/Realm, Server_UserID/Server_Password, Client_Password */	
	struct OBEX_HeaderStru headers;
};

struct OBEX_AbortRspStru {/* arg in OBEX_Abort_Rsp */
	UINT32 handle;/* Add connection_handle */
	UINT8 op_id;/* Only used in server response */
	struct OBEX_HeaderStru headers;
};

struct OBEX_AbortIndStru {/* arg in OBEX_EVA4_ABORT_RSP */
	UINT32 handle;/* Add connection_handle */
	UINT8 op_id;/* Only used in server response */
	struct BuffStru *packet;
};

struct OBEX_Put_IndStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* Put or Put_Final for IND */
	UINT8 op_id;/* Allocated by OBEX Server, mainly used in PUT/GET */
	UINT8 enable_srm;/* SRM not present, SRM 00disable, SRM 01enable */
	UINT8 ask4_putrsp;/* need to call Put_Rsp or not */
	struct BuffStru *pdata;
};

struct OBEX_Put_RspStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 rsp_code;/* rsp_code for RSP */
	UINT8 op_id;/* equal to the one in struct OBEX_Put_IndStru */
	UINT8 enable_srm;/* SRM not present, SRM 00disable, SRM 01enable */
	UINT8 srmp_01;
	struct OBEX_HeaderStru headers;
};

struct OBEX_Get_IndStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 op_code;/* Get or Get_Final */
	UINT8 op_id;/* Allocated by OBEX Server, mainly used in PUT/GET  */
	UINT8 enable_srm;
	UINT8 srmp_00;/* used to tell profile should set srmp 00 ask for more or not */
	UINT8 *offset_tag;/* length and value of object offset, in 'tlv' format, used in session resume */
	struct BuffStru *pdata;
};

struct OBEX_Get_RspStru {
	UINT32 handle;/* Match to a struct OBEX_ConnectionUserDataStru */
	UINT8 rsp_code;
	UINT8 op_id;/* equal to the one in struct OBEX_Get_IndStru */
	UINT8 enable_srm;
	UINT8 srmp_00;/* Server ask for additional packet */
	struct OBEX_HeaderStru headers;
};

struct OBEX_ServerCbkStru {/* Input arg for OBEX_ServerStart/OBEXC_ServerStop, OBEX_EV_SERVER_START_CFM */
	UINT32 server_handle;/* PSM for l2cap, or server_channel for RFCOMM, high byte is type */
	UINT16 max_packetlen_in;/* Profile specific incoming MAX OBEX packet length, 0 to use OBEX default. */
	void *cbk;
};

struct OBEX_ActionIndStru {/* arg in OBEX_EVA4_ACTION_RSP */
	UINT32 handle;
	UINT8 op_id;
	UINT8 op_code;
	struct BuffStru *packet;
};

struct OBEX_ActionRspStru {/* arg in OBEX_Action_Rsp */
	UINT32 handle;
	UINT8 rsp_code;
	UINT8 op_id;
	struct OBEX_HeaderStru headers;
};
#endif

/* Notice: this is within the same set with Rsp code(0x80 always set) */
enum {/* OBEX_ConnectionFI_Done reason, used in OBEX_EV_DISCONNECT_COMPLETE & OBEX_EV_CONNECT_COMPLETE */
	ER_OBEX_SUCCESS,/* OBEX_EV_CONNECT_COMPLETE OK */
	ER_OBEX_L2CAP_CONNECT_FAILED,
	ER_OBEX_RFCOMM_CONNECT_FAILED,
	ER_OBEX_DISCONNECT_REQ_BEFORE_OBEXCONNECT,
	ER_OBEX_DISCONNECT_REQ_IN_OTHER_STATUS,
	ER_OBEX_DISCONNECT_REQ_BY_SERVER_STOP,
	ER_OBEX_DISCONNECT_COMPLETE,/* Disconnect_Ind */
	ER_OBEX_AUTH_FAILED,
	ER_OBEX_SESSION_CREATE_FAILED,
	ER_OBEX_SESSION_CLOSE_CFM,
	ER_OBEX_SESSION_CLOSE_RSP,
	ER_OBEX_SESSION_RESUME_CFMERROR,
	ER_OBEX_SESSION_SUSPEND_CFMERROR,
	ER_OBEX_SESSION_SUSPEND_INDERROR,
	ER_OBEX_SESSION_SSN_INVAILD,
	ER_OBEX_RTXTIMEOUT,
	ER_OBEX_ERROR_PACKET_RECEIVED,
	ER_OBEX_DONE,
};

/* OBEX is big endian */
#define OBEX_DECODE2BYTE(ptr) 	(UINT16)(*(UINT8 *)((ptr) + 1) | (*(UINT8 *)(ptr) << 8))
#define OBEX_DECODE4BYTE(ptr)\
	(UINT32)(*(UINT8 *)((ptr) + 3) | (*(UINT8 *)((ptr) + 2) << 8) \
	| (*(UINT8 *)((ptr) + 1) << 16) | (*(UINT8 *)(ptr) << 24))
#define OBEX_ENCODE2BYTE(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)(((data) & 0xFF00) >> 8);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) & 0x00FF);
#define OBEX_ENCODE4BYTE(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)(((data) & 0xFF000000) >> 24);\
	*(UINT8 *)((ptr) + 1) = (UINT8)(((data) & 0x00FF0000) >> 16);\
	*(UINT8 *)((ptr) + 2) = (UINT8)(((data) & 0x0000FF00) >> 8);\
	*(UINT8 *)((ptr) + 3) = (UINT8)((data) & 0x000000FF);

/*---------------------------------------------------------------------------*/
/* 								OBEX UI Interface Functions							     */
/*---------------------------------------------------------------------------*/
FUNC_EXPORT void OBEX_Init(void);
FUNC_EXPORT void OBEX_Done(void);
FUNC_EXPORT void OBEX_Disconnect(struct OBEX_DisconnectStru *in);

#ifdef CONFIG_OBEX_CLIENT
FUNC_EXPORT void OBEX_ClientStart(struct OBEX_ConnectReqStru *in);
#define OBEX_ClientStop OBEX_Disconnect
FUNC_EXPORT void OBEX_ClientAbort(struct OBEX_AbortReqStru *in);
FUNC_EXPORT void OBEX_ClientPush(struct OBEX_Put_ActiveReqStru *in);
FUNC_EXPORT void OBEX_ClientA4Push(struct OBEX_Put_PassiveReqStru *in);
FUNC_EXPORT void OBEX_ClientPull(struct OBEX_Get_ActiveReqStru *in);
FUNC_EXPORT void OBEX_ClientA4Pull(struct OBEX_Get_PassiveReqStru *in);
FUNC_EXPORT void OBEX_ClientSetPath(struct OBEX_SetPathStru *in);
FUNC_EXPORT void OBEX_ClientAction(struct OBEX_ActionReqStru *in);

FUNC_EXPORT void OBEX_ClientSessionCreateReq(struct OBEX_ConnectReqStru *in);
FUNC_EXPORT void OBEX_ClientSessionCloseReq(struct OBEX_SessionStru *in);
FUNC_EXPORT void OBEX_ClientSessionSuspendReq(struct OBEX_SessionStru *in);
FUNC_EXPORT void OBEX_ClientSessionResumeReq(struct OBEX_SessionResumeStru *in);
#endif

#ifdef CONFIG_OBEX_SERVER
FUNC_EXPORT void OBEX_ServerStart(struct OBEX_ServerCbkStru *in);
FUNC_EXPORT void OBEX_ServerStop(struct OBEX_ServerCbkStru *in);
FUNC_EXPORT void OBEX_ServerConnect_Rsp(struct OBEX_ConnectRspStru *in);
FUNC_EXPORT void OBEX_ServerPush_Rsp(struct OBEX_Put_RspStru *in);
FUNC_EXPORT void OBEX_ServerPull_Rsp(struct OBEX_Get_RspStru *in);
FUNC_EXPORT void OBEX_ServerAbort_Rsp(struct OBEX_AbortRspStru *in);
FUNC_EXPORT void OBEX_ServerSetPath_Rsp(struct OBEX_SetPathStru *in);
FUNC_EXPORT void OBEX_ServerAction_Rsp(struct OBEX_ActionRspStru *in);
#endif

FUNC_EXPORT UINT8 *OBEX_Decode_TagLenValueHeader(struct BuffStru *packet, UINT16 offset, UINT8 type, UINT8 tag);
FUNC_EXPORT UINT8 *OBEX_Decode_ByteSeq(struct BuffStru *packet, UINT8 type, UINT16 *len);
FUNC_EXPORT UINT8 *OBEX_Decode_ByteSequenceHeader(struct BuffStru *packet, UINT16 offset, UINT8 type);/* This is same to OBEX_Decode_ByteSeq, but return simple pointer to the 2bytes len field */
FUNC_EXPORT UINT32 OBEX_Decode_QuantityHeader(struct BuffStru *packet, UINT16 offset, UINT8 type);
FUNC_EXPORT UINT8 OBEX_Decode_ObjectLength(struct BuffStru *pdata, BtUINT64Stru *obj_len);
FUNC_EXPORT struct OBEX_UITLVStru *OBEX_Encode_UITLV(struct OBEX_UITLVStru *dest, UINT8 type, UINT16 total_len, UINT8 len, UINT8 *data);
FUNC_EXPORT UINT8 *OBEX_Decode_UITLV(struct OBEX_UITLVStru *src, UINT8 type);
FUNC_EXPORT void OBEX_Remove_UITLV(struct OBEX_UITLVStru *dest, UINT8 type);

#endif

