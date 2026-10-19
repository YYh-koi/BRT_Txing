/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2017 BRT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	bip_hlp.h
Abstract:
	This file defines the internal data structure, constants and function
	declarations.
Author:
    Y.D.X
Revision History:
	1.	2016.11.04	Created.
------------------------------------------------------------------------*/
#ifndef _BIP_HLP_H
#define _BIP_HLP_H

#include "bip_ui.h"
//TODO BARROT #include "xmlparse.h"
#include "expat.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* Common*/
/* BIP Specific OBEX Headers */
#define OBEX_HI_BIP_IMG_HANDLE		0x30 /* the image handle : UTF-16 Unicode text */
#define OBEX_HI_BIP_IMG_DESCRIPTON	0x71 /* the image descriptor : byte sequence, length prefixed with a UINT16 */

/* Initiator */
/* Operation type */
typedef enum {
	BIP_OP_GET_IMAGE_PROPERTIES,
	BIP_OP_GET_IMAGE,
	BIP_OP_GET_LINKED_THUMBNAIL,

	BIP_OP_UNKNOWN = 0xFF
} tBIPH_OPType;

/* Local Initiator instance status */
typedef enum {
	BIP_IMGC_ST_DISCOVERING,		/* 0 */
	BIP_IMGC_ST_CONNECTING,			/* 1 */
	BIP_IMGC_ST_CONNECTED,			/* 2 */
	BIP_IMGC_ST_GET_OBJECT,			/* 3 */
	BIP_IMGC_ST_PUT_OBJECT			/* 4 */
} tBIPH_ImgClntStatus;

/* Additional status masks */
#define BIP_CLNT_STM_DISCONNECT	0x01
#define BIP_CLNT_STM_GET_BODY	0x02

#define MAX_BIP_RECORD_COUNT	6

/* Function ID */
typedef enum {
	BIP_FUNCTION_GET_CAPABILITIES,
	BIP_FUNCTION_PUT_IMAGE,
	BIP_FUNCTION_PUT_LINKED_ATTACHMENT,
	BIP_FUNCTION_PUT_LINKED_THUMBNAIL,
	BIP_FUNCTION_REMOTE_DISPLAY,
	BIP_FUNCTION_GET_IMAGE_LIST,
	BIP_FUNCTION_GET_IMAGE_PROPERTIES,
	BIP_FUNCTION_GET_IMAGE,
	BIP_FUNCTION_GET_LINKED_THUMBNAIL,
	BIP_FUNCTION_GET_LINKED_ATTACHMENT,
	BIP_FUNCTION_DELETE_IMAGE,
	BIP_FUNCTION_START_PRINT,
	BIP_FUNCTION_GET_PARTIAL_IMAGE,
	BIP_FUNCTION_START_ARCHIVE,
	BIP_FUNCTION_GET_MONITORING_IMAGE,
	BIP_FUNCTION_GET_STATUS,

	BIP_FUNCTION_UNKNOWN
} tBIPH_FunctionID;

/* AppParameters Tag ID */
typedef enum {
	BIP_TAG_NBRETHDLS = 1,
	BIP_TAG_LISTOFF,
	BIP_TAG_LATEST,
	BIP_TAG_PARTIALLEN,
	BIP_TAG_PARTIALOFF,
	BIP_TAG_TOTALSIZE,
	BIP_TAG_ENDFLAG,
	BIP_TAG_REMDISP,
	BIP_TAG_SVCID,
	BIP_TAG_STOREFLAG,
	
	BIP_TAG_UNKNOWN
} tBIPH_TagID;

/* bit mask for <structure>::req_mask, rsp_mask, par_mask */
/* The Application Parameters */
#define BIP_HD_PAR_NBRETHDLS		0x0001
#define BIP_HD_PAR_LISTOFF			0x0002
#define BIP_HD_PAR_LATEST			0x0004
#define BIP_HD_PAR_PARTIALLEN		0x0008
#define BIP_HD_PAR_PARTIALOFF		0x0010
#define BIP_HD_PAR_TOTALSIZE		0x0020
#define BIP_HD_PAR_ENDFLAG			0x0040
#define BIP_HD_PAR_REMDISP			0x0080
#define BIP_HD_PAR_SVCID			0x0100
#define BIP_HD_PAR_STOREFLAG		0x0200
/* Headers */
#define BIP_HD_APPPARAM				(BIP_HD_PAR_NBRETHDLS|BIP_HD_PAR_LISTOFF|BIP_HD_PAR_LATEST|BIP_HD_PAR_PARTIALLEN\
									|BIP_HD_PAR_PARTIALOFF|BIP_HD_PAR_TOTALSIZE|BIP_HD_PAR_ENDFLAG|BIP_HD_PAR_REMDISP\
									|BIP_HD_PAR_SVCID|BIP_HD_PAR_STOREFLAG)
#define BIP_HD_NAME					0x0400
#define BIP_HD_IMGDESC				0x0800
#define BIP_HD_IMGHDL				0x1000
#define BIP_HD_LENGTH				0x2000
#define BIP_HD_BODY					0x4000

/* Request masks for functions */
#define BIP_REQM_GET_CAPABILITIES			(0)
#define BIP_REQM_PUT_IMAGE					(BIP_HD_NAME|BIP_HD_IMGDESC|BIP_HD_BODY)
#define BIP_REQM_PUT_LINKED_ATTACHMENT		(BIP_HD_IMGDESC|BIP_HD_IMGHDL|BIP_HD_BODY)
#define BIP_REQM_PUT_LINKED_THUMBNAIL		(BIP_HD_IMGHDL|BIP_HD_BODY)
#define BIP_REQM_REMOTE_DISPLAY				(BIP_HD_IMGHDL|BIP_HD_PAR_REMDISP)
#define BIP_REQM_GET_IMAGE_LIST				(BIP_HD_PAR_NBRETHDLS|BIP_HD_PAR_LISTOFF|BIP_HD_PAR_LATEST|BIP_HD_IMGDESC)
#define BIP_REQM_GET_IMAGE_PROPERTIES		(BIP_HD_IMGHDL)
#define BIP_REQM_GET_IMAGE					(BIP_HD_IMGHDL|BIP_HD_IMGDESC)
#define BIP_REQM_GET_LINKED_THUMBNAIL		(BIP_HD_IMGHDL)
#define BIP_REQM_GET_LINKED_ATTACHMENT		(BIP_HD_IMGHDL|BIP_HD_NAME)
#define BIP_REQM_DELETE_IMAGE				(BIP_HD_IMGHDL)
#define BIP_REQM_START_PRINT				(BIP_HD_PAR_SVCID|BIP_HD_BODY)
#define BIP_REQM_GET_PARTIAL_IMAGE			(BIP_HD_NAME|BIP_HD_PAR_PARTIALLEN|BIP_HD_PAR_PARTIALOFF)
#define BIP_REQM_START_ARCHIVE				(BIP_HD_PAR_SVCID)
#define BIP_REQM_GET_MONITORING_IMAGE		(BIP_HD_PAR_STOREFLAG)
#define BIP_REQM_GET_STATUS					(0)
/* Response masks for functions */
#define BIP_RSPM_GET_CAPABILITIES			(0)
#define BIP_RSPM_PUT_IMAGE					(BIP_HD_IMGHDL)
#define BIP_RSPM_PUT_LINKED_ATTACHMENT		(0)
#define BIP_RSPM_PUT_LINKED_THUMBNAIL		(0)
#define BIP_RSPM_REMOTE_DISPLAY				(BIP_HD_IMGHDL)
#define BIP_RSPM_GET_IMAGE_LIST				(BIP_HD_PAR_NBRETHDLS|BIP_HD_IMGDESC|BIP_HD_BODY)
#define BIP_RSPM_GET_IMAGE_PROPERTIES		(BIP_HD_BODY)
#define BIP_RSPM_GET_IMAGE					(BIP_HD_LENGTH|BIP_HD_BODY)
#define BIP_RSPM_GET_LINKED_THUMBNAIL		(BIP_HD_BODY)
#define BIP_RSPM_GET_LINKED_ATTACHMENT		(BIP_HD_BODY)
#define BIP_RSPM_DELETE_IMAGE				(0)
#define BIP_RSPM_START_PRINT				(0)
#define BIP_RSPM_GET_PARTIAL_IMAGE			(BIP_HD_LENGTH|BIP_HD_PAR_TOTALSIZE|BIP_HD_PAR_ENDFLAG|BIP_HD_BODY)
#define BIP_RSPM_START_ARCHIVE				(0)
#define BIP_RSPM_GET_MONITORING_IMAGE		(BIP_HD_IMGHDL|BIP_HD_BODY)
#define BIP_RSPM_GET_STATUS					(0)

/* Constants for XML Parser */
#define BIP_XML_PARSER_LINE_LENGTH  16


/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
typedef struct {
#ifdef CONFIG_BIP_CLIENT
	struct ListStru imgc_list;		/* tBIPH_IMGCInstStru */
#endif
} tBIPH_UserDataStru;

typedef struct {
	UINT8           *header_src;    /* Pointer to the data source of header being stuffed to the OBEX packet */
	UINT16          src_size;       /* Size of the header_src */
	UINT16          src_offset;     /* Start of the header_src that has not been stuffed, 0 based */
	UINT8           src_allocated;  /* 1 - header_src shall be freed. 0 - header_src shall not be freed. */
} tBIPH_ComposeHeaderStru;

typedef struct {
	const char *type;				/* Type Header */
	UINT16		req_mask;			/* Headers and AppParameters tags specific to the Request packet. */
	UINT16		rsp_mask;			/* Headers and AppParameters tags specific to the Response packet. */
} tBIPH_FunctionInfoStru;

typedef union {
	void					*general;       /* Unspecified parameter */
	tBIP_ObjectDataStru		*obj_data;      /* Parameter of BIP_EVT_OBJECT_DATA_IND etc. */
	tBIP_GetContinueFlag    *continue_flag; /* Parameter of BIP_EVT_ASK4_GET_REQUEST_IND. */
	UINT16                  *result;        /* Parameter of BIP_EVT_GET_OBJECT_COMPLETE, etc. */
} tBIPH_EventParameters;

typedef struct {
	tBIP_XMLElement	element_type;		/* The element type got */
	union {
	tBIP_ImageDescriptorStru image;	/* Image-Descriptor Header */
	tBIP_ImageHandlesDescriptorStru handles;	/* Image-Handles-Descriptor Header */
	tBIP_AttachmentDescriptorStru attachment;	/* Attachment-Descriptor Header */
	} u;
} tBIPH_DescriptorParserStru;

typedef struct {
	tBIP_XMLElement	element_type;		/* The element type got */
	union {
	tBIP_ImageDescriptorStru *image;	/* Image-Descriptor Header */
	tBIP_ImageHandlesDescriptorStru *handles;	/* Image-Handles-Descriptor Header */
	tBIP_AttachmentDescriptorStru *attachment;	/* Attachment-Descriptor Header */
	} u;
} tBIPH_DescriptorStru;

/* Initiator */
typedef struct {
	tBIP_EventCbk	*cbk;			/* callback to handle connection confirmation and link lost indication */
	void			*context;		/* Application defined data structure used to identify the request */
	UINT32			channel_psm;	/* server channel or PSM of the service. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM). */
	UINT16			feature;		/* Feature to use. E.g. BIP_FEATURE_IMAGE_PUSH. */
	UINT8			bd[BD_ADDR_LEN];/* Device address of the service */
} tBIPH_ClntStartStru;

/* Context structure for Get operations */
typedef struct {
	/* Headers */
	WCHAR			*name;				/* Name Header. Big Endian internally. System order at UI. */
	tImageHandle	image_handle;		/* Image-Handle Header. */
	tBIPH_DescriptorStru descriptor;
	/* Generic */
	void			*clnt_handle;/* Handle identify the connection */
	tBIP_EventCbk	*cbk;		/* callback to handle object data indication and final confirmation */
	void			*context;	/* Application defined data structure used to identify the request */
	UINT8			function;	/* One of tBIPH_FunctionID except for BIP_FUNCTION_UNKNOWN. */
	/* AppParameters */
	UINT8			store_flag;			/* StoreFlag parameter. */
	UINT8			latest_captured;	/* LatestCapturedImages parameter. */
	UINT8			end_flag;			/* EndFlag parameter. */
	UINT16			nb_return_handles;	/* NbReturnHandles parameter. */
	UINT16			list_start_offset;	/* ListStartOffset parameter. */
	UINT32			partial_file_length;/* PartialFileLength parameter. */
	UINT32			partial_file_offset;/* PartialFileStartOffset parameter. */
	UINT32			total_file_size;	/* TotalFileSize parameter. */
} tBIPH_GetObjStru;

/* Context structure for Put operations */
typedef struct {
	/* Headers */
	WCHAR			*name;				/* Name Header. Big Endian internally. System order at UI. */
	tImageHandle	image_handle;		/* Image-Handle Header. */
	tBIPH_DescriptorStru descriptor;
	/* Generic */
	void			*clnt_handle;/* Handle identify the connection */
	tBIP_EventCbk	*cbk;		/* callback to handle object data indication and final confirmation */
	void			*context;	/* Application defined data structure used to identify the request */
	UINT8			function;	/* One of tBIPH_FunctionID except for BIP_FUNCTION_UNKNOWN. */
	/* The upper shall be the same as tBIPH_GetObjStru */
	/* AppParameters */
	UINT8			remote_display;		/* RemoteDisplay parameter. */
	UINT8			*service_id;		/* ServiceID parameter. 16bytes UUID in big Endian */
} tBIPH_PutObjStru;

/* Authentication Response */
typedef struct {
	void			*clnt_handle;	/* Handle identify the connection */
	UCHAR			*user_id;		/* UserID field of the Digest Response.*/
	UCHAR			*password;		/* Password of the PSE.*/
} tBIPH_AuthenRspStru;

/* Client Instance */
typedef struct {
	UINT8				bd[BD_ADDR_LEN];/* Device address of the Responder */
	UINT8				status;			/* One of tBIPH_ImgClntStatus */
	UINT8				status_mask;	/* Additional status information, e.g. BIP_CLNT_STM_DISCONNECT */
	tBIP_EventCbk		*conn_cbk;		/* callback to handle connection confirmation and link lost indication */
	void				*context;		/* Application defined data structure used to identify the ClientStart request */
	UINT32				channel_psm;	/* server channel or PSM of the Responder. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM). */
	UINT32				obex_tl_handle;	/* OBEX connection handle */
	union {
	tBIPH_ClntStartStru	*clnt_start_p;
	tBIPH_GetObjStru	*get_req_p;
	tBIPH_PutObjStru	*put_req_p;
	}					cur_op;			/* determined by inst_status */
	tBIPH_ComposeHeaderStru cur_header;/* Header being composed */
} tBIPH_IMGCInstStru;

/* Context structure for Service Discovery */
typedef struct {
    UINT8  			bd[BD_ADDR_LEN];/* Bluetooth device address of the Responder */
    void  			*context;       /* Application defined context data */
    tBIP_EventCbk	*cbk;  			/* Application defined callback function */
	UINT16			feature;		/* Feature to use. E.g. BIP_FEATURE_IMAGE_PUSH. */

	SDAP_SearchRsp_Func *sa_func;	/* ServiceAttribute confirmation for different scenario */
	void 			*cmp_func;		/* Internal complete function for different scenario. BIPH_SDPComplete_Func */
	UINT16 			trans_hdl;		/* Transaction handle used to identify the SDP connection. */
    UINT8 			svc_hdl_count;  /* Total number of service record handles */
    UINT8 			svc_hdl_idx;   	/* Service record handle index of current querying record */
    UINT32 			*svc_hdls;     	/* Array of service record handles */
} tBIPH_SDPContextStru;

typedef void (BIPH_SDPComplete_Func)(tBIPH_SDPContextStru *app_context, UINT16 status);

/* Context structures for GetImageProperties Operations */
typedef struct {
	tBIP_XMLElement	element_type;	/* Specifies the type of the property element. */
	union {
		tBIP_BasicPropertyStru		basic;		/* If element_type is BIP_ELEMENT_IMAGE_PROPERTIES */
		tBIP_NativePropertyStru		native;		/* If element_type is BIP_ELEMENT_NATIVE */
		tBIP_VariantPropertyStru	variant;	/* If element_type is BIP_ELEMENT_VARIANT */
		tBIP_AttachmentPropertyStru	attachment;	/* If element_type is BIP_ELEMENT_ATTACHMENT */
	} property;
    /* The upper shall be the same as tBIP_ImagePropertyStru */
	UINT8 found;		/* 1 - Find one property item. */
	UINT8 finished;		/* 1 - Parsing of this item is finished. */
} tBIPH_ImgPropParserStru;

typedef struct {
	tBIP_EventCbk	*cbk;				/* callback to handle GetImageProperties indications and confirmation */
	void			*context;			/* Application defined data structure used to identify the GetImageProperties request */
	XML_Parser      parser;             /* XML parser object */
	tBIPH_ImgPropParserStru  cur_obj;    /* Current object being parsed */
} tBIPH_GetImgPropContextStru;

/* Context structures for GetImagesList Operations */
typedef struct {
	tImageHandle	image_handle;	/* The image's handle. 0 - 9999999 */
	UCHAR			*created;		/* The creation time of the attachment. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time).
						   			It may be NULL. */
	UCHAR			*modified;		/* The last modified time of the attachment. The same format as created. It may be NULL. */
    /* The upper shall be the same as tBIP_ImageListItemStru */
	UINT8 found;		/* 1 - Find one image item. */
	UINT8 finished;		/* 1 - Parsing of this item is finished. */
} tBIPH_ImgListParserStru;

typedef struct {
	tBIP_EventCbk	*cbk;				/* callback to handle GetImageProperties indications and confirmation */
	void			*context;			/* Application defined data structure used to identify the GetImageProperties request */
	XML_Parser      parser;             /* XML parser object */
	tBIPH_ImgListParserStru  cur_obj;    /* Current object being parsed */
} tBIPH_GetImgListContextStru;

/* XML Parser */
typedef tBIP_XMLElement (tBIPH_DecodeElementFunc)(const char **attrs, void *elem_stru);

typedef struct {
	const char	*elem_name;	/* Element name string */
	void		*func;		/* Function to decode the element, tBIPH_DecodeElementFunc */
} tBIPH_ElemParserStru;

/* Responder */


/*---------------------------------------------------------------------------
							Globals
----------------------------------------------------------------------------*/
extern struct FsmInst *g_bip_fsminst;

#define BIP_CUDATA				((tBIPH_UserDataStru *)(g_bip_fsminst->user_data))
#define BIP_IMGC_LIST			(BIP_CUDATA->imgc_list)

#define BIP_FREE(m)				{if (m != NULL) FREE(m);}
#define BIP_BUFFREE(m)			{if (m != NULL) BuffFree(m);}

/*---------------------------------------------------------------------------
							Functions
----------------------------------------------------------------------------*/
/* Operation Macro */
#ifdef CONFIG_LITTLE_ENDIAN
#define BIPH_LE2_TO_BE2(_le)	((_le >> 8) + ((_le & 0xFF) << 8))
#define BIPH_LE4_TO_BE4(_le)	((_le >> 24) + ((_le & 0x00FF0000) >> 8) + ((_le & 0x0000FF00) << 8)+ ((_le & 0xFF) << 24))
#else
#define BIPH_LE2_TO_BE2(_le)	(_le)
#define BIPH_LE4_TO_BE4(_le)	(_le)
#endif

/* Common */
void BIP_Fsm_Done(struct FsmInst *fi, UINT8 event, void *arg);
void BIP_Fsm_Disconnect(struct FsmInst *fi, UINT8 event, void **arg);

void BIPH_SendConnectCompleteEv(tBIP_EventCbk *cbk, void *handle, tBIPH_ClntStartStru *arg, UINT16 result);
void BIPH_SendDisconnectCompleteEv(tBIP_EventCbk *cbk, void *handle, void *context, UINT8 *bd, UINT16 reason);

UINT16 BIPH_GetNbHeaders(int fid, UINT16 exclude_mask, int is_req);
void BIPH_SetHeaderIDs(int fid, UINT16 exclude_mask, int is_req, UINT8 *hids);
void BIPH_ComposeShortHeader(tBIPH_ComposeHeaderStru *cur_header, struct OBEX_ComposeHeaderStru *info);
UINT16 BIPH_ConvertOBEXError(UINT32 obex_err);

tBIP_ImageHandlesDescriptorStru *BIPH_CopyImgHandlesDescriptor(tBIP_ImageHandlesDescriptorStru *in_par);

tBIP_XMLElement BIPH_ElemImageParser(const char **attrs, tBIP_ImageListItemStru *elem_stru);
tBIP_XMLElement BIPH_ElemBasicPropParser(const char **attrs, tBIP_BasicPropertyStru *elem_stru);
tBIP_XMLElement BIPH_ElemNativePropParser(const char **attrs, tBIP_NativePropertyStru *elem_stru);
tBIP_XMLElement BIPH_ElemVariantPropParser(const char **attrs, tBIP_VariantPropertyStru *elem_stru);
tBIP_XMLElement BIPH_ElemAttachmentPropParser(const char **attrs, tBIP_AttachmentPropertyStru *elem_stru);
tBIP_XMLElement BIPH_ElemAttachmentFmtParser(const char **attrs, tBIP_AttachmentFormatsStru *elem_stru);
tBIP_XMLElement BIPH_ElemCapFilterParamParser(const char **attrs, tImageFilters *elem_stru);
tBIP_XMLElement BIPH_DPOFOptParser(const char **attrs, tDPOFOptions *elem_stru);
tBIP_XMLElement BIPH_ElemDescFilterParamParser(const char **attrs, tBIP_ImageHandlesDescriptorStru *elem_stru);
tBIP_XMLElement BIPH_ElemImgDescriptorParser(const char **attrs, tBIP_ImageDescriptorStru *elem_stru);
tBIP_XMLElement BIPH_ElemAttachmentDescriptorParser(const char **attrs, tBIP_AttachmentDescriptorStru *elem_stru);

/* Initiator */
#ifdef CONFIG_BIP_CLIENT
tBIPH_IMGCInstStru *IMGC_FindInstByBDChannel(void *ignored_item, UINT8 *bd, UINT32 channel_psm);
void IMGC_InstDone(tBIPH_IMGCInstStru *cli_inst, UINT16 reason);

void IMGC_DiscoverService4ClientStartComplete(tBIPH_SDPContextStru *in_context, UINT16 status);
void IMGC_ClientStartSACfm(tBIPH_SDPContextStru *in_context, UINT16 status, struct SDP_SearchCfmStru *sa_cfm);
void IMGC_DiscoverService(UINT8 *bd, void *context, UINT16 feature, void *sa_func, void *cmp_func, void *app_cbk);
void IMGC_SendClntStartCfmEv(tBIPH_IMGCInstStru *cli_inst);
void IMGC_SendAuthenIndEv(tBIPH_IMGCInstStru *cli_inst, struct OBEX_ConnectAuthReqStru *arg);
void IMGC_OBEXConnect(tBIPH_IMGCInstStru *cli_inst, tBIPH_AuthenRspStru *auth_rsp);
void IMGC_OBEXAbort(tBIPH_IMGCInstStru *cli_inst);
void IMGC_ComposeNameHeader(tBIPH_IMGCInstStru *cli_inst);
void IMGC_ComposeTypeHeader(tBIPH_IMGCInstStru *cli_inst);
void IMGC_ComposeAppParametersHeader(tBIPH_IMGCInstStru *cli_inst);
void IMGC_ComposeHeaderInd(struct OBEX_ComposeHeaderStru *info);
void IMGC_DecodeAppParameters(tBIPH_GetObjStru *param, struct BuffStru *pdata);

void IMGC_XML_ImgPropElementBegin(tBIPH_ImgPropParserStru *obj_info, const char *name, const char **attrs);
void IMGC_XML_ImgPropElementEnd(tBIPH_ImgPropParserStru *obj_info, const char *name);
void IMGC_GetImagePropertiesCbk(tBIPH_IMGCInstStru *clnt_inst, tBIPH_GetImgPropContextStru *context, tBIP_Event_Type event, void *param);

void IMGC_XML_ImgListElementBegin(tBIPH_ImgListParserStru *obj_info, const char *name, const char **attrs);
void IMGC_XML_ImgListElementEnd(tBIPH_ImgListParserStru *obj_info, const char *name);
void IMGC_GetImagesListCbk(tBIPH_IMGCInstStru *clnt_inst, tBIPH_GetImgListContextStru *context, tBIP_Event_Type event, void *param);

void IMGC_SendGetCompleteEv(tBIPH_GetObjStru *arg, UINT16 result);
void IMGC_SendObjDataIndEv(tBIPH_IMGCInstStru *cli_inst, struct BuffStru *pdata, tBIP_Completeness is_final);
tBIP_GetContinueFlag IMGC_SendAsk4GetReqIndEv(tBIPH_IMGCInstStru *cli_inst);

void BIP_Fsm_ClientStart(struct FsmInst *fi, UINT8 event, tBIPH_ClntStartStru *arg);
void BIP_Fsm_AuthenRsp(struct FsmInst *fi, UINT8 event, tBIPH_AuthenRspStru *arg);
void BIP_Fsm_GetStart(struct FsmInst *fi, UINT8 event, tBIPH_GetObjStru *arg);
void BIP_Fsm_GetContinue(struct FsmInst *fi, UINT8 event, void **arg);
void BIP_Fsm_Abort(struct FsmInst *fi, UINT8 event, void **arg);

void BIP_Fsm_OBEXConnectCfm(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectCompleteStru *arg);
void BIP_Fsm_OBEXAuthenInd(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectAuthReqStru *arg);
void BIP_Fsm_IMGCOBEXConnectComplete(struct FsmInst *fi, UINT8 event, struct OBEX_ConnectCompleteStru *arg);
void BIP_Fsm_IMGCOBEXDisconnectComplete(struct FsmInst *fi, UINT8 event, struct OBEX_DisconnectCompleteStru *arg);
void BIP_Fsm_OBEXGetContinue(struct FsmInst *fi, UINT8 event, struct OBEX_Get_CfmStru *arg);
void BIP_Fsm_OBEXA4GetReq(struct FsmInst *fi, UINT8 event, struct OBEX_Get_PassiveReqStru *arg);
void BIP_Fsm_OBEXGetComplete(struct FsmInst *fi, UINT8 event, struct OBEX_OP_CompleteStru *arg);
#endif

/* Responder */


#endif
