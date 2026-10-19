/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2018 Barrot Technology Limited
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
	bip_ui.h
Abstract:
	This file defines the data structure, constants and function declarations
	of API.
Author:
    Y.D.X
Revision History:
	1.	2016.11.04	Created.
------------------------------------------------------------------------*/
#ifndef _BIP_UI_H
#define _BIP_UI_H

#include "global.h"
#include "hci_ui.h"
#include "obex_ui.h"
#include "gap_ui.h"
#include "sdap_ui.h"

/*---------------------------------------------------------------------------
							Constants
----------------------------------------------------------------------------*/
/* Possible error code group values */
#define BIP_ER_GROUP_BIP		  0x0000
#define BIP_ER_GROUP_HCI		  (ER_TYPE_HCI << 8)
#define BIP_ER_GROUP_L2CAP		  (ER_TYPE_L2CAP << 8)
#define BIP_ER_GROUP_RFCOMM		  (ER_TYPE_RFCOMM << 8)
#define BIP_ER_GROUP_OBEX		  (ER_TYPE_OBEX << 8)

#define BIP_ER_GROUP_MASK		  0xFE00
#define BIP_GET_ER_GROUP(_res)	  (_res & BIP_ER_GROUP_MASK)
		  
/* Possible result codes */
/* BIP private result code, OBEX standard response code - BIP_ER_GROUP_BIP */
typedef enum {
	/* BIP private result code */
	ER_BIP_SUCCESS,				/* Success */

	ER_BIP_REPEATED_CONNECTION,	/* A connection to the same PSE service exists */

	ER_BIP_SDP_CONNECTION_FAILURE,	/* Fail to create SDP connection */
	ER_BIP_SERVICE_NOT_AVAILABLE,	/* Can't find PSE service record or the PSE service is labeled as unavailable */
	ER_BIP_REMOTE_SDP_TRANSACTION_ERROR,	/* Error occur during SDP transaction */

	ER_BIP_INVALID_HANDLE,			/* The specified handle is not a valid BIP instance handle */
	ER_BIP_NO_CONNECTION,			/* The connection is not ready to execute the operation requested */
	ER_BIP_LOCAL_BUSY,				/* Another operation is incomplete */
	ER_BIP_LOCAL_ABORT,				/* Operation aborted by local APP */
	ER_BIP_LOCAL_CLOSE,				/* The BIP Stack is being closed. */

    ER_BIP_TOO_MORE_SERVICES,		/* Local services of specified type have reached the limit. */

	/* OBEX standard response code, only those listed in the BIP Spec is defined explicitly */
	ER_BIP_OBEX_RSP_BAD_REQUEST = OBEX_RSP_BAD_REQUEST,
	ER_BIP_OBEX_RSP_UNAUTHORIZED = OBEX_RSP_UNAUTHORIZED,
	ER_BIP_OBEX_RSP_FORBIDDEN = OBEX_RSP_FORBIDDEN,
	ER_BIP_OBEX_RSP_NOT_FOUND = OBEX_RSP_NOT_FOUND,
	ER_BIP_OBEX_RSP_NOT_ACCEPTABLE = OBEX_RSP_NOT_ACCEPTABLE,
	ER_BIP_OBEX_RSP_PRECONDITION_FAILED = OBEX_RSP_PRECONDITION_FAILED,
	ER_BIP_OBEX_RSP_NOT_IMPLEMENTED = OBEX_RSP_NOT_IMPLEMENTED,
	ER_BIP_OBEX_RSP_SERVICE_UNAVAILABLE = OBEX_RSP_SERVICE_UNAVAILABLE
	
} tBIP_PbapResult;
/* 
	OBEX standard response code - BIP_ER_GROUP_BIP
	e.g. (BIP_ER_GROUP_BIP + OBEX_RSP_BAD_REQUEST)
*/
/* 
	HCI error code - BIP_ER_GROUP_HCI
	e.g. (BIP_ER_GROUP_HCI + HCI_STATUS_PAGE_TIMEOUT)
*/
/* 
	L2CAP error code - BIP_ER_GROUP_L2CAP
	e.g. (BIP_ER_GROUP_L2CAP + ER_L2CAP_RTXTIMEOUT)
*/
/*
	RFCOMM error code - BIP_ER_GROUP_RFCOMM
	e.g. (BIP_ER_GROUP_RFCOMM + ER_RFCOMM_UA_IND_DISC)
*/
/*
	OBEX private error code - BIP_ER_GROUP_OBEX
	e.g. (BIP_ER_GROUP_OBEX + ER_OBEX_DISCONNECT_REQ_BEFORE_OBEXCONNECT)
*/


/* Type of event */
typedef enum {
    /* Common */
	BIP_EVT_CONNECT_COMPLETE,
	BIP_EVT_DISCONNECT_COMPLETE,

    /* Initiator */
    BIP_EVT_RESPONDER_SERVICE_IND,
	BIP_EVT_CLIENT_START_CFM,
	BIP_EVT_AUTHENTICATION_IND,
	BIP_EVT_GET_OBJECT_COMPLETE,

	BIP_EVT_LIST_IMAGE_IND,
	BIP_EVT_GET_IMAGES_LIST_COMPLETE,

	BIP_EVT_IMAGE_PROPERTY_IND,

	BIP_EVT_OBJECT_SIZE_IND,
	BIP_EVT_OBJECT_DATA_IND,
	BIP_EVT_ASK4_GET_REQUEST_IND,

    /* Responder */

	BIP_EVT_END = 0xFF
} tBIP_Event_Type;

/* Possible values of tBIP_Ask4GetReqIndCbk::next_op */
typedef enum {
	BIP_OP_CONTINUE,
	BIP_OP_ABORTING,
	BIP_OP_PENDING
} tBIP_GetContinueFlag, tBIP_PutContinueFlag;

/* Possible values of tMAP_ObjectDataStru::is_final */
typedef enum {
	BIP_DATA_PARTIAL,
	BIP_DATA_COMPLETE,
	BIP_DATA_INCOMPLETE
} tBIP_Completeness;

/* Possible bit mask values of Digest Challenge options */
#define BIP_DCOPT_USERID_REQUIRED		1
#define BIP_DCOPT_READ_ONLY				2

/* Possible bit mask values of <structure>::supported_capabilities */
#define BIP_SUPPORTED_CAPABILITY_GENERIC_IMAGING		(1<<0)
#define BIP_SUPPORTED_CAPABILITY_CAPTURING				(1<<1)
#define BIP_SUPPORTED_CAPABILITY_PRINTING				(1<<2)
#define BIP_SUPPORTED_CAPABILITY_DISPLAYING				(1<<3)

/* Possible bit mask values of <structure>::supported_features */
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH				(1<<0)
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH_STORE			(1<<1)
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH_PRINT			(1<<2)
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH_DISPLAY		(1<<3)
#define BIP_SUPPORTED_FEATURE_IMAGE_PULL				(1<<4)
#define BIP_SUPPORTED_FEATURE_ADVANCED_IMAGE_PRINTING	(1<<5)
#define BIP_SUPPORTED_FEATURE_AUTOMATIVE_ARCHIVE		(1<<6)
#define BIP_SUPPORTED_FEATURE_REMOTE_CAMERA				(1<<7)
#define BIP_SUPPORTED_FEATURE_REMOTE_DISPLAY			(1<<8)

/* Possible bit mask values of <structure>::supported_functions */
#define BIP_SUPPORTED_FUNCTION_GET_CAPABILITIES			(1<<0)
#define BIP_SUPPORTED_FUNCTION_PUT_IMAGE				(1<<1)
#define BIP_SUPPORTED_FUNCTION_PUT_LINKED_ATTACHMENT	(1<<2)
#define BIP_SUPPORTED_FUNCTION_PUT_LINKED_THUMBNAIL		(1<<3)
#define BIP_SUPPORTED_FUNCTION_REMOTE_DISPLAY			(1<<4)
#define BIP_SUPPORTED_FUNCTION_GET_IMAGE_LIST			(1<<5)
#define BIP_SUPPORTED_FUNCTION_GET_IMAGE_PROPERTIES		(1<<6)
#define BIP_SUPPORTED_FUNCTION_GET_IMAGE				(1<<7)
#define BIP_SUPPORTED_FUNCTION_GET_LINKED_THUMBNAIL		(1<<8)
#define BIP_SUPPORTED_FUNCTION_GET_LINKED_ATTACHMENT	(1<<9)
#define BIP_SUPPORTED_FUNCTION_DELETE_IMAGE				(1<<10)
#define BIP_SUPPORTED_FUNCTION_START_PRINT				(1<<11)
#define BIP_SUPPORTED_FUNCTION_GET_PARTIAL_IMAGE		(1<<12)
#define BIP_SUPPORTED_FUNCTION_START_ARCHIVE			(1<<13)
#define BIP_SUPPORTED_FUNCTION_GET_MONITORING_IMAGE		(1<<14)
#define BIP_SUPPORTED_FUNCTION_GET_STATUS				(1<<16)

/* Possible values of <structure>::feature. */
typedef enum {
	BIP_FEATURE_IMAGE_PUSH,
	BIP_FEATURE_IMAGE_PUSH_STORE,
	BIP_FEATURE_IMAGE_PUSH_PRINT,
	BIP_FEATURE_IMAGE_PUSH_DISPLAY,
	BIP_FEATURE_IMAGE_PULL,
	BIP_FEATURE_ADVANCED_IMAGE_PRINTING,
	BIP_FEATURE_AUTOMATIVE_ARCHIVE,
	BIP_FEATURE_REMOTE_CAMERA,
	BIP_FEATURE_REMOTE_DISPLAY,
	BIP_FEATURE_AVRCP_COVER_ART,

	BIP_FEATURE_UNKNOWN = 0xFF
} tBIP_Feature;

/* Possible values of <structure>::element_type. */
typedef enum {
	/* Images-Listing Object */
	BIP_ELEMENT_IMAGE,

	/* Image-Properties Object */
	BIP_ELEMENT_IMAGE_PROPERTIES,
	BIP_ELEMENT_NATIVE,
	BIP_ELEMENT_VARIANT,
	BIP_ELEMENT_ATTACHMENT,

	/* Image-Capabilities Object */
	/* BIP_ELEMENT_VARIANT */
	/* BIP_ELEMENT_NATIVE */
	BIP_ELEMENT_ATTACHMENT_FORMATS,
	BIP_ELEMENT_FILTERING_PARAMETERS,
	BIP_ELEMENT_DPOF_OPTIONS,

	/* Image-Descriptor */
	/* BIP_ELEMENT_FILTERING_PARAMETERS */
	/* BIP_ELEMENT_IMAGE */
	/* BIP_ELEMENT_ATTACHMENT */

	BIP_ELEMENT_INVALID = 0xFF
} tBIP_XMLElement;

/* Possible values of <structure>::encoding. */
typedef enum {
	BIP_ENCODING_JPEG,
	BIP_ENCODING_GIF,
	BIP_ENCODING_WBMP,
	BIP_ENCODING_PNG,
	BIP_ENCODING_JPEG2000,
	BIP_ENCODING_BMP,
	BIP_ENCODING_USER_TAG,

	BIP_ENCODING_ANY = 0xFF
} tBIP_Encoding;

/* Possible bit masks of <structure>::transformation. */
#define BIP_TRANSFORMATION_STRETCH		(1<<0)
#define BIP_TRANSFORMATION_FILL			(1<<1)
#define BIP_TRANSFORMATION_CROP			(1<<2)

/* Possible bit masks of tImageFilters. */
#define BIP_FILTER_CREATED				(1 << 0)
#define BIP_FILTER_MODIFIED				(1 << 1)
#define BIP_FILTER_ENCODING				(1 << 2)
#define BIP_FILTER_PIXEL				(1 << 3)

/* Possible bit masks of tDPOFOptions. */
#define BIP_DPOF_OPTION_STANDARD_PRINT			(1 << 0)
#define BIP_DPOF_OPTION_INDEX_PRINT				(1 << 1)
#define BIP_DPOF_OPTION_MULTIPLE_IMAGE_PRINT	(1 << 2)
#define BIP_DPOF_OPTION_SPECIFIC_SIZE_PRINT		(1 << 3)
#define BIP_DPOF_OPTION_NUMBER_SETS				(1 << 4)
#define BIP_DPOF_OPTION_CHARACTER_STAMP			(1 << 5)
#define BIP_DPOF_OPTION_TRIMMING				(1 << 6)

/*---------------------------------------------------------------------------
							Structures
----------------------------------------------------------------------------*/
/* Common */
typedef long	tImageHandle;

typedef struct {
	UINT16  channel;    		/* server channel of the Imaging Responder service. 0 means unspecifed. */
	UINT16  psm;        		/* dynamic PSM of the Imaging Responder service. 0 means unspecified. */
	UINT16  supported_capabilities;	/* SupportedCapabilities of the Imaging Responder service, bit mask of BIP_SUPPORTED_CAPABILITY_GENERIC_IMAGING, etc.. */
	UINT16  supported_features;/* SupportedFeatures of the Imaging Responder service, bit mask of BIP_SUPPORTED_FEATURE_IMAGE_PUSH, etc.. */
	UINT32  supported_functions;/* SupportedFunctions of the Imaging Responder service, bit mask of BIP_SUPPORTED_FUNCTION_GET_CAPABILITIES, etc.. */
	BtUINT64Stru total_data_capacity; /* Higher 32bit of the TotalImagingDataCapacity of the Imaging Responder service. */
	UINT8	*service_name;		/* Service Name, NULL terminated UTF-8 string */
    UINT32  sdp_record_handle;  /* SDP service record handle of this MAS service. Only used to return value to the application. The value specified by the application is ignored. */
} tBIP_ResponderAttributesStru;

/* Parameter of BIP_EVT_CONNECT_COMPLETE and BIP_EVT_DISCONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  result;             /* Connection result */
	UINT32  channel_psm;        /* server channel or PSM of the PSE connected. In the format: (OBEX_TL_RFCOMM | channel) or (OBEX_TL_L2CAP | PSM) */
	tBIP_Feature feature;		/* Feature of the connection. One of BIP_FEATURE_IMAGE_PUSH. */
} tBIP_ConnectCompleteStru;

/* Parameter of BIP_EVT_DISCONNECT_COMPLETE */
typedef struct {
	UINT8   bd[BD_ADDR_LEN];    /* Bluetooth device address of the remote device */
	UINT16  reason;             /* Reason of disconnection */
} tBIP_DisconnectCompleteStru;

/* Element "image" of the images-listing object */
typedef struct {
	tImageHandle	image_handle;	/* The image's handle. 0 - 9999999 */
	UCHAR			*created;		/* The creation time of the attachment. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time).
						   			It may be NULL. */
	UCHAR			*modified;		/* The last modified time of the attachment. The same format as created. It may be NULL. */
} tBIP_ImageListItemStru;

/* Element "image-properties" of the image-properties object */
typedef struct {
	tImageHandle	image_handle;	/* The image's handle. 0 - 9999999 */
	UCHAR     		*friendly_name;	/* A human readable name of the image. NULL-terminated UTF-8 string. It may be NULL. */
} tBIP_BasicPropertyStru;

/* Element "native" of the image-properties object */
typedef struct {
    tBIP_Encoding	encoding;	/* Encoding method of the image. */
    UCHAR	*user_tag;	/* Proprietary encodinig method. A null-terminated UTF-8 string. It is available only when encoding is set to BIP_ENCODING_USER_TAG. */
	/* Usage of pixel_xx combinations:
	1. Fixed pixel: pixel_w1 and pixel_h1 shall be set to the fixed pixel. pixel_w2 and pixel_h2 shall be set to 0.
	2. Pixel range with a modified aspect ratio: pixel_w1, pixel_h1, pixel_w2 and pixel_h2 shall all be none-0.
	3. Pixel range with a fixed aspect ratio: pixel_h1 shall be set to 0; pixel_w1, pixel_w2 and pixel_h2 shall all be none-0. */
    UINT16	pixel_w1;	/* Width of the smallest image size supported. */
    UINT16	pixel_h1;	/* Height of the smallest image size supported. */
    UINT16	pixel_w2;	/* Width of the largest image size supported. */
    UINT16	pixel_h2;	/* Height of the largest image size supported. */
    BtUINT64Stru size;	/* The image file size in bytes. It is ignored if the value is 0. */
} tBIP_NativePropertyStru;

/* Element "variant" of the image-properties object */
typedef struct {
    tBIP_Encoding	encoding;	/* Encoding method of the image. */
    UCHAR	*user_tag;	/* Proprietary encodinig method. A null-terminated UTF-8 string. It is available only when encoding is set to BIP_ENCODING_USER_TAG. */
    UINT16	pixel_w1;	/* Width of the smallest image size supported. */
    UINT16	pixel_h1;	/* Height of the smallest image size supported. */
    UINT16	pixel_w2;	/* Width of the largest image size supported. */
    UINT16	pixel_h2;	/* Height of the largest image size supported. */
    BtUINT64Stru max_size;	/* The estimated maximum size of the image after conversion. It is ignored if the value is 0. */
	UINT8	transformation;	/* Bit mask specifies the supported transformation methods. E.g. BIP_TRANSFORMATION_STRETCH. It is ignored if the value is 0. */
} tBIP_VariantPropertyStru;

/* Element "attachment" of the image-properties object */
typedef struct {
    UCHAR	*content_type;	/* MIME content type of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    UCHAR	*charset;	/* MIME character set of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    UCHAR	*name;		/* The file name of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    BtUINT64Stru size;	/* The attachment file size in bytes. It is ignored if the value is 0. */
	UCHAR	*created;	/* The creation time of the attachment. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time).
						   It may be NULL. */
	UCHAR	*modified;	/* The last modified time of the attachment. The same format as created. It may be NULL. */
} tBIP_AttachmentPropertyStru;

/* Element "preferred-format" of the imaging-capabilities object */
typedef struct {
    tBIP_Encoding	encoding;	/* Encoding method of the image. */
    UCHAR	*user_tag;	/* Proprietary encodinig method. A null-terminated UTF-8 string. It is available only when encoding is set to BIP_ENCODING_USER_TAG. */
    UINT16	pixel_w1;	/* Width of the smallest image size supported. */
    UINT16	pixel_h1;	/* Height of the smallest image size supported. */
    UINT16	pixel_w2;	/* Width of the largest image size supported. */
    UINT16	pixel_h2;	/* Height of the largest image size supported. */
    BtUINT64Stru max_size;	/* The maximum file size in bytes acceptable for an image. It is ignored if the value is 0. */
	UINT8	transformation;	/* Bit mask specifies the supported transformation methods. E.g. BIP_TRANSFORMATION_STRETCH. It is ignored if the value is 0. */
} tBIP_PreferredFormatStru;

/* Element "image-formats" of the imaging-capabilities object */
typedef struct {
    tBIP_Encoding	encoding;	/* Encoding method of the image. */
    UCHAR	*user_tag;	/* Proprietary encodinig method. A null-terminated UTF-8 string. It is available only when encoding is set to BIP_ENCODING_USER_TAG. */
    UINT16	pixel_w1;	/* Width of the smallest image size supported. */
    UINT16	pixel_h1;	/* Height of the smallest image size supported. */
    UINT16	pixel_w2;	/* Width of the largest image size supported. */
    UINT16	pixel_h2;	/* Height of the largest image size supported. */
    BtUINT64Stru max_size; /* The maximum file size in bytes acceptable for an image. It is ignored if the value is 0. */
} tBIP_ImageFormatsStru;

/* Element "attachment-formats" of the imaging-capabilities object */
typedef struct {
    UCHAR	*content_type;	/* MIME content type of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    UCHAR	*charset;		/* MIME character set of the attachment. A null-terminated UTF-8 string. It may be NULL. */
} tBIP_AttachmentFormatsStru;

/* Element "filtering-parameters" of the imaging-capabilities object */
typedef unsigned long tImageFilters;	/* Bit mask specifies the supported image attribute filters. E.g. BIP_FILTER_CREATED. */

/* Element "DPOF-options" of the imaging-capabilities object */
typedef unsigned long tDPOFOptions;	/* Bit mask specifies the supported DPOF options. E.g. BIP_DPOF_OPTION_STANDARD_PRINT. */

/* Element "filtering-parameters" of the image-handles-descriptor object */
typedef struct {
    tBIP_Encoding	encoding;	/* Encoding method of the image. */
    UCHAR	*user_tag;	/* Proprietary encodinig method. A null-terminated UTF-8 string. It is available only when encoding is set to BIP_ENCODING_USER_TAG. */
    UINT16	pixel_w1;	/* Width of the smallest image size supported. */
    UINT16	pixel_h1;	/* Height of the smallest image size supported. */
    UINT16	pixel_w2;	/* Width of the largest image size supported. */
    UINT16	pixel_h2;	/* Height of the largest image size supported. */
	UCHAR	*created_start;	/* The start of the range of image creation dates. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time).
						   	A NULL pointer means any images before the created_end. */
	UCHAR	*created_end; 	/* The end of the range of image creation dates. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time).
							A NULL pointer means any images after the created_start. */
	UCHAR	*modified_start;/* The start of the range of image modification dates. Similar to created_start. */
	UCHAR	*modified_end;	/* The end of the range of image modification dates. Similar to created_end. */
} tBIP_ImageHandlesDescriptorStru;

/* Element "image" of the image-descriptor object */
typedef struct {
    tBIP_Encoding	encoding;	/* Encoding method of the image. */
    UCHAR	*user_tag;	/* Proprietary encodinig method. A null-terminated UTF-8 string. It is available only when encoding is set to BIP_ENCODING_USER_TAG. */
    UINT16	pixel_w1;	/* Width of the smallest image size supported. */
    UINT16	pixel_h1;	/* Height of the smallest image size supported. */
    UINT16	pixel_w2;	/* Width of the largest image size supported. */
    UINT16	pixel_h2;	/* Height of the largest image size supported. */
    BtUINT64Stru size;	/* The size/maxsize attribute value. For GetImageRequest, it is set to the maxsize attribute value.
    						 For PutImageRequest, it is set to the size attribute value. It is ignored if the value is 0. */
	UINT8	transformation;	/* Specifies the transformation method applied or to be applied to the image. E.g. BIP_TRANSFORMATION_STRETCH. It is ignored if the value is 0. */
} tBIP_ImageDescriptorStru;

/* Element "attachment" of the attachment-descriptor object */
typedef struct {
    UCHAR	*content_type;	/* MIME content type of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    UCHAR	*charset;	/* MIME character set of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    UCHAR	*name;		/* The file name of the attachment. A null-terminated UTF-8 string. It may be NULL. */
    BtUINT64Stru size;	/* The attachment file size in bytes. It is ignored if the value is 0. */
	UCHAR	*created;	/* The creation time of the attachment. NULL terminated ASCII string, formated in YYYYMMDDTHHMMSS or YYYYMMDDTHHMMSSZ (UTC time).
						   It may be NULL. */
} tBIP_AttachmentDescriptorStru;

/* Parameters of GetImagesList request */
typedef struct {
    UINT16	nb_returned_handles;	/* Maximum number of image handles to be returned. 0 to return only the total numbers. 0xFFFF to return all images. */
	UINT16	list_start_offset;		/* Zero-based offset from the beginning of the images-listing object. */
	UINT8	latest_captured_images;	/* Shall be 0 or 1. 1 indicates that only the most recently captured images shall be returned in descending time order.
									0 indicates any images can be returned in any order. The list_start_offset is ignored if latest_captured_images is set to 1. */
    tBIP_ImageHandlesDescriptorStru *filter_parameter;	/* Specify the filtering parameters to apply. It can be NULL. */
} tBIP_GetImagesListReqStru;

/* Parameters of GetImagesList response */
typedef struct {
	UINT16  result;             	/* Result of the GetImagesList operation. */
	/* The following values shall be ignored in case of a failure */
    UINT16	nb_returned_handles;	/* Number of image items returned or to be returned. */
    tBIP_ImageHandlesDescriptorStru *filter_parameter;	/* Specify the filtering parameters applied */
} tBIP_GetImagesListCompleteStru;

/* Initiator */
/* Parameter of BIP_EVT_AUTHENTICATION_IND */
typedef struct {
	UINT8	bd[BD_ADDR_LEN];	/* Bluetooth device address of the Responder */
	UINT8	options;			/* Options field of Digest Challenge, combinations of BIP_DCOPT_xxxx */
	UINT8	*realm;				/* Realm field of Digest Challenge. NULL if not present */ 	 
} tBIP_AuthenIndStru;

/* Parameter of BIP_EVT_OBJECT_DATA_IND */
typedef struct {	
	UINT8				*data;				/* Data chunk of the Object. It is NULL if the Get operation is terminated abnormally. */
	UINT16				length;				/* Size, in bytes, of the data. It is 0 if the Get operation is terminated abnormally. */
	tBIP_Completeness	is_final;			/* BIP_DATA_PARTIAL - More data to be received. BIP_DATA_COMPLETE - It is the final chunk.
											   BIP_DATA_INCOMPLETE - The object transfer is aborted by the client and no more data will be received. */
} tBIP_ObjectDataStru;

/* Parameter of BIP_EVT_OBJECT_DATA_IND */
typedef struct {
	tBIP_XMLElement	element_type;	/* Specifies the type of the property element. */
	union {
		tBIP_BasicPropertyStru		basic;		/* If element_type is BIP_ELEMENT_IMAGE_PROPERTIES */
		tBIP_NativePropertyStru		native;		/* If element_type is BIP_ELEMENT_NATIVE */
		tBIP_VariantPropertyStru	variant;	/* If element_type is BIP_ELEMENT_VARIANT */
		tBIP_AttachmentPropertyStru	attachment;	/* If element_type is BIP_ELEMENT_ATTACHMENT */
	} property;
} tBIP_ImagePropertyStru;

/* Responder */

/*---------------------------------------------------------------------------
							Callback Prototype
----------------------------------------------------------------------------*/
/* 
Description:
    General callback function to handle confirmations or indications.
Parameters:
    handle    [in] Handle identify the local client or server entity reporting the
				   event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] The event type.
    param     [in/out] Additional event parameter. Refer to the following
                   specific event callback definition. It is set to NULL if
                   not specified additionally.
*/
typedef void (tBIP_EventCbk)(void *handle, void *context, tBIP_Event_Type event, void *param);

/*
Description:
    Callback function to report attribute values of an imaging responder service discovered.
    It is also used to report attribute values of the AVRCP TG service that supports cover
    art. This service is selected for cover art connection. 
Parameters:
    bd    	  [in] BD_ADDR of the server being discovered.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_RESPONDER_SERVICE_IND.
    param     [in] Attribute values of the service discovered. For an 
*/
typedef void (tBIP_ResponderServiceInd)(UINT8 *bd, void *context, tBIP_Event_Type event, tBIP_ResponderAttributesStru *param);

/* 
Description:
    Callback function to handle confirmation to the ClientStart request.
	This confirmation is used to return the lower handle identifying the
	client entity. And the APP can then use this handle to cancel the connection
	creation process.
Parameters:
    handle    [in] Handle identify the local client entity reporting the
				   event. It is also identify the new connection to create.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart request.
    event     [in] Shall be BIP_EVT_CLIENT_START_CFM.
    bd		  [in] Bluetooth device address of the remote Responder to connect.
*/
typedef void (tBIP_ClientStartCfmCbk)(void *handle, void *context, tBIP_Event_Type event, UINT8 *bd);

/* 
Description:
    Callback function to handle OBEX authentication initiated by the Responder.
	Call BIP_AuthenticationRsp to respond with the proper password.
	NOTE: Only the challenge to the Connection is supported.
Parameters:
    handle    [in] Handle identify the local client entity reporting the
				   event.
	context	  [in] Application defined data structure used to identify
				   the request being challenged.
    event     [in] Shall be BIP_EVT_AUTHENTICATION_IND.
    param	  [in] Digest Challenge information.
*/
typedef void (tBIP_AuthenIndCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_AuthenIndStru *param);

/*
Description:
    Callback function to report the size of an object got.

    For a Get operation, this indication is only created when the none-empty
    Length header or HTTP header with content-length is detected in the Get Response
    packet. The object size may be used to calculate the progress.

Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_OBJECT_SIZE_IND.
    param     [in] Contains the size of the image.
*/
typedef void (tBIP_ObjectSizeIndCbk)(void *handle, void *context, tBIP_Event_Type event, BtUINT64Stru *param);

/* 
Description:
    Callback function to handle indication of the object content.
	During the Get operation, the APP may receive this indication more than once.
	If the peer device rejects the Get request, the APP may not receive this indication.
Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_OBJECT_DATA_IND.
    param     [in] Chunk of the object content.
			  
*/
typedef void (tBIP_ObjectDataIndCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_ObjectDataStru *param);

/* 
Description:
    Callback function to handle indication asking for next Get request.
	This indication allows the application to pend the pulling operation so that
	it can handle the object data got in time without overflowing the storage.
	If the storage resource is large enough, the application can just ignore this
	indication.
Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_ASK4_GET_REQUEST_IND.
    next_op   [in/out] On input, it is set to BIP_OP_CONTINUE. On output,
                   1) Keep it unchanged, the BIP Stack sends Get request to the
                   peer device immediately after this callback returns.
                   2) Change it to BIP_OP_ABORTING, the BIP Stack sends Abort
                   request to the peer device immediately after this callback returns.
                   The pulling operation is then terminated.
                   3) Change it to BIP_OP_PENDING, the BIP Stack waits for the
                   application to call BIP_GetContinueReq to send Get request or
                   to call BIP_CancelPullObjectReq to terminate the pulling operation.
			  
*/
typedef void (tBIP_Ask4GetReqIndCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_GetContinueFlag *next_op);

/*
Description:
    Callback function to report one image item of the images-listing object.

    For a GetImagesList operation, this indication is created repeatedly for
    each image element found in the Images-Listing object returned.
Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_LIST_IMAGE_IND.
    param     [in] Contains information of one image item in the images-listing object.
*/
typedef void (tBIP_ListingImageIndCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_ImageListItemStru *param);

/*
Description:
    Callback function to report the property of an image.

    For a GetImageProperties operation, this indication is created repeatedly for
    each property element found in the Image-Properties object returned.
Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_IMAGE_PROPERTY_IND.
    param     [in] Contains a property element of the image.
*/
typedef void (tBIP_ImagePropertyIndCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_ImagePropertyStru *param);

/* 
Description:
    Callback function to handle confirmation to all the Get request if not specified additionally.
Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_GET_OBJECT_COMPLETE.
    result    [in] Result of the Get operation.
*/
typedef void (tBIP_GetObjectCompleteCbk)(void *handle, void *context, tBIP_Event_Type event, UINT16 *result);

/* 
Description:
    Callback function to handle confirmation to GetImagesList request.
Parameters:
    handle    [in] Handle identify the local client entity reporting the event.
	context	  [in] Application defined data structure used to identify
				   the request.
    event     [in] Shall be BIP_EVT_GET_IMAGES_LIST_COMPLETE.
    param     [in] Result of the GetImagesList operation.
*/
typedef void (tBIP_GetImagesListCompleteCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_GetImagesListCompleteStru *param);

/* 
Description:
    Callback function to handle the result of connection creation process.
    If local device acts as a server, this event is created only when the
    connection initiated by the remote client is created successfully.
    If local device acts as a client, this event is created for both success and
    failure situations.
Parameters:
    handle    [in] Handle identify the local client or server entity reporting the
				   event. It is also identify the new connection created.
				   It shall be NULL if the connection is failed to create.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be BIP_EVT_CONNECT_COMPLETE.
    param     [in] Result of the connection creation process.
*/
typedef void (tBIP_ConnectCompleteCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_ConnectCompleteStru *param);

/* 
Description:
    Callback function to handle disconnect complete event.
Parameters:
    handle    [in] Handle identify the local client or server entity reporting the
                   event.
	context	  [in] Application defined data structure used to identify
				   the ClientStart or ServerStart request.
    event     [in] Shall be BIP_EVT_DISCONNECT_COMPLETE.
    param     [in] Information of the connection disconnected.
*/
typedef void (tBIP_DisconnectCompleteCbk)(void *handle, void *context, tBIP_Event_Type event, tBIP_DisconnectCompleteStru *param);


/*---------------------------------------------------------------------------
							Interfaces
----------------------------------------------------------------------------*/
/* Common */
void BIP_Init(void);
void BIP_Done(void);
void BIP_DisconnectReq(void *con_handle);

/* Initiator */
#ifdef CONFIG_BIP_CLIENT
void BIP_ClientStartReq(void *context, UINT8 *bd, tBIP_Feature feature, UINT32 channel_psm, tBIP_EventCbk *cbk);
#define BIP_ClientStopReq  BIP_DisconnectReq

void BIP_GetImagesListReq(void *cli_handle, void *context, tBIP_GetImagesListReqStru *req_par, tBIP_EventCbk *cbk);
void BIP_GetImagePropertiesReq(void *cli_handle, void *context, tImageHandle img_handle, tBIP_EventCbk *cbk);
void BIP_GetImageReq(void *cli_handle, void *context, tImageHandle img_handle, tBIP_ImageDescriptorStru *req_param, tBIP_EventCbk *cbk);
void BIP_GetLinkedThumbnailReq(void *cli_handle, void *context, tImageHandle img_handle, tBIP_EventCbk *cbk);
void BIP_GetContinueReq(void *cli_handle);
void BIP_AbortReq(void *cli_handle);
void BIP_AuthenticationRsp(void *cli_handle, UCHAR *user_id, UCHAR *password);
#endif

/* Responder */
#ifdef CONFIG_BIP_SERVER
#endif

#endif
