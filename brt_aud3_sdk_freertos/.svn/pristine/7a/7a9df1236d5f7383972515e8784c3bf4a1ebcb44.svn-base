#ifndef _HLP_MAP_H_
#define _HLP_MAP_H_

#include "btdatatype.h"
#include "btlist.h"
#include "auto_map.h"
#include "bt_event.h"
#include "brt_trace.h"
#include "vcf_parse.h"
#include "map_ui.h"


enum hlp_map_state {
    STATE_UNINIT,
    STATE_INIT,
    STATE_READY,
    STATE_CONNECTING,
    STATE_REGISTERING_NOTIF,
    STATE_SETTING_FOLDER,
    STATE_CONNECTED,
    STATE_GETTING
};

#define MAX_BUF_LENGTH          16
#define EVENT_ITEM_LEN          80
#define FOLDER_ITEM_LEN         100
#define MAX_MAP_STR_LEN             256

#define MAX_MAP_MSG_LEN (1024)
#define MAX_MAP_HANDLE_LEN (16)

enum hlp_map_message_location {
    HLP_MAP_MESSAGE_LOCATION_INBOX,
    HLP_MAP_MESSAGE_LOCATION_OUTBOX,
    HLP_MAP_MESSAGE_LOCATION_SENT,
    HLP_MAP_MESSAGE_LOCATION_DELETED,
    HLP_MAP_MESSAGE_LOCATION_DRAFT,
    HLP_MAP_MESSAGE_LOCATION_IDLE = 0xFF,
};

typedef struct tag_map_fold_list_item {
	UINT32  size;
	UINT8   attrib;  /*object type: dir, file, read-only file or hidden file*/
	UCHAR   *name;    /*required, others are optional*/
	UCHAR   *create_time;   /* the created time of the file */
	UCHAR   *access_time;   /* the access time of the file */
	UCHAR   *write_time;        /* the written time of the file */
} map_folder_list_item_t;

typedef struct tag_hlp_map_context {
	int         state;
	int         dir_depth;
	app_map_event_callback callback;
	struct ListStru conn_list;
	vcf_parser_t *p_vcf_parser;
	struct BtList *get_msg_list;
	module_event_cbk_t event_cbk;
	int store_type;
	int offset;
	int max_list_counts;
	UINT8 get_list_flag;
	struct BtList *msg_list;
} hlp_map_context_t;

typedef union {
	void								*param;
	tMAP_ServerStartCompleteStru		*svr_start_cmp;
	tMAP_MASAttributesStru				*mas_inst;
	UINT8								*bd;
	tMAP_ConnectCompleteStru			*conn_cmp;
	tMAP_DisconnectCompleteStru			*disc_cmp;
	tMAP_ObjectDataStru					*obj_data;
	tMAP_GetFolderListCompleteStru		*get_fdlist_cmp;
	tMAP_GetMessageListCompleteStru		*get_msglist_cmp;
	tMAP_GetMessageCompleteStru			*get_msg_cmp;
	tMAP_Ask4PutRequestStru				*ask4_put_req;
	tMAP_PushMessageCompleteStru		*put_msg_cmp;
	UINT16								*result;
	tMAP_GetContinueFlag				*get_continue;
	tMAP_EventReportStru				*evt_report;
} tAppMACEvtParameters;

typedef struct {
	char message_handle[MAX_MAP_HANDLE_LEN+1];
	char message_subject[MAX_MAP_MSG_LEN];//< 256
	char message_content[64];

	char message_sender_name[40];//the name of the sender of the message, when it is known by the MSE device
	//the addressing information of the sender. In case of an email this is the sender's email address,
	//In case of a SMS this is the sender's phone number in canonical form
	char message_sender_addressing[40];

	char message_copyto_addressing[40];//the addressing information for copy to (only used in email)

	char message_recipient_name[40];//the name of the recipient of the message, when it is known by the MSE device

	char message_recipient_addressing[40];//the addressing information of the recipient (to)

	char message_data_time[20];//yyyy-mm-dd hh:mm:ss

	unsigned char  message_folder_type;
	unsigned char message_property;

	unsigned char message_priority;//yes indicates that the message is of high priority, no indicates that the message is not of high priority
	unsigned char message_read;// 1->is read, 0 -> unread
	unsigned char message_sent;// 1->is sent,0 -> unsent
	unsigned char message_protected;
} map_message_info_t;

typedef struct tag_map_message_list_item {
	UINT32 mask;
	UCHAR *msg_hdl;
	UCHAR *subject;
	UCHAR *date_time;
	UCHAR *sender_name;
	UCHAR *sender_addr;
	UCHAR *replyto_addr;
	UCHAR *recipient_name;
	UCHAR *recipient_addr;
	UCHAR *msg_type; /*EMAIL, SMS_GSM, SMS_CDMA, MMS*/
	UINT32 msg_size;
	UCHAR *reception_status; /*complete, fractioned, notification*/
	UINT32 attach_size;  /*Value = 0   indicates that the message has no attachments*/
	UCHAR *msg_priority; /*"yes" or "no"*/
	UCHAR *is_text;  /*"yes" or "no"*/
	UCHAR *is_read;
	UCHAR *is_send;
	UCHAR *is_protected;
} map_message_list_item_t;

typedef struct tag_map_event_object_ {
	UCHAR *evt_type; /*such as:NewMessage, DeliverySuccess, SendingSuccess etc..*/
	UCHAR *handle;   /*handle is the handle that was assigned by the MSE*/
	UCHAR *folder;
	UCHAR *older_folder;
	UCHAR *msg_type;  /*EMAIL, SMS_GSM, SMS_CDMA, MMS*/
} map_event_object_t;

typedef struct MAPXMLFindDataStru {
	UINT8   file_type;  /* Type of the element */
	UINT8   flag;       /* reserved */
	struct ListStru data;      /* Element attribute values */
} MAPXMLFINDDATA, *PMAPXMLFINDDATA;

/*FolderListObject Type*/
#define MAP_ATTRIB_DIR          0x01    /* directory   */
#define MAP_ATTRIB_FILE         0x02
#define MAP_ATTRIB_READ         0x04    /* read only   */
#define MAP_ATTRIB_SYS          0x08    /* system file */
#define MAP_ATTRIB_HIDDEN       0x10    /* hidden file */
#define MAP_ATTRIB_NORMAL       0x20    /* normal file */
#define MAP_ATTRIB_SIZE         0x40
#define MAP_ATTRIB_PROCESSED    0x80

/*Left shit to form ParameterMask when GetMessageList*/
#define IDX_SUBJECT             0x00
#define IDX_DATETIME            0x01
#define IDX_SENDER_NAME         0x02
#define IDX_SENDER_ADDR         0x03
#define IDX_RECIPIENT_NAME      0x04
#define IDX_RECIPIENT_ADDR      0x05
#define IDX_MSG_TYPE            0x06
#define IDX_MSG_SIZE            0x07
#define IDX_RECEPTION_STATUS    0x08
#define IDX_IS_TEXT             0x09
#define IDX_ATTACHMENT_SIZE     0x0A
#define IDX_PRIORITY            0x0B
#define IDX_IS_READ             0x0C
#define IDX_IS_SEND             0x0D
#define IDX_IS_PROTECTED        0x0E
#define IDX_REPLYTO_ADDR        0x0F

/*Filter Message type*/
#define MASK_SMS_GSM            0x01
#define MASK_MSK_SMS_CDMA       0x02
#define MASK_MSK_EMAIL          0x04
#define MASK_MSK_MMS            0x08

/*Filter Message priority*/
#define  MASK_NOFILTERING       0x00
#define  MASK_HIGH_PRIORITY     0x01
#define  MASK_NORMAL_PRIORITY   0x02

/*filter ReadStatus*/
#define MASK_FILTER_UNREAD      0x01
#define MASK_FILTER_READ        0x02


/*Item mask when GetMessageListing*/
#define MASK_ITEM_SUBJECT           (1<<IDX_SUBJECT)
#define MASK_ITEM_DATETIME          (1<<IDX_DATETIME)
#define MASK_ITEM_SENDER_NAME       (1<<IDX_SENDER_NAME)
#define MASK_ITEM_SENDER_ADDR       (1<<IDX_SENDER_ADDR)
#define MASK_ITEM_RECIPIENT_NAME    (1<<IDX_RECIPIENT_NAME)
#define MASK_ITEM_RECIPIENT_ADDR    (1<<IDX_RECIPIENT_ADDR)
#define MASK_ITEM_MSG_TYPE          (1<<IDX_MSG_TYPE)
#define MASK_ITEM_MSG_SIZE          (1<<IDX_MSG_SIZE)
#define MASK_ITEM_RECEPTION_STATUS  (1<<IDX_RECEPTION_STATUS)
#define MASK_ITEM_IS_TEXT           (1<<IDX_IS_TEXT)
#define MASK_ITEM_ATTACHMENT_SIZE   (1<<IDX_ATTACHMENT_SIZE)
#define MASK_ITEM_PRIORITY          (1<<IDX_PRIORITY)
#define MASK_ITEM_IS_READ           (1<<IDX_IS_READ)
#define MASK_ITEM_IS_SEND           (1<<IDX_IS_SEND)
#define MASK_ITEM_IS_PROTECTED      (1<<IDX_IS_PROTECTED)
#define MASK_ITEM_REPLYTO_ADDR      (1<<IDX_REPLYTO_ADDR)


#define IDX_MAP_FILE_FOLDER_LISTING  0x01
#define IDX_MAP_FILE_EVENT_OBJECT   0x02
#define IDX_MAP_FILE_MSG_LISTING    0x03
#define IDX_MAP_FILE_BMESSAGE       0x04

#define GET_STR_LEN(pStr)  ((pStr!=NULL)?strlen(pStr):0)
#define GET_WSTR_LEN(pStr)  ((pStr!=NULL)?wcslen(pStr):0)

#define MAP_ALLOC_AND_COPY_STR(pDes, pSrc) {\
            UINT16 len = GET_STR_LEN(pSrc);\
            if (len>0) {\
                            len = (len<MAX_MAP_STR_LEN)?len:MAX_MAP_STR_LEN;\
                            pDes = (UINT8*)NEW(len+1);\
                            memset(pDes, 0, len+1);\
                            memcpy(pDes, pSrc, len);\
                        }\
        }

void hlp_map_init(module_event_cbk_t event_cbk);
void hlp_map_done(void);
void hlp_map_register_callback(app_map_event_callback callback);
void hlp_map_connect(UINT8 *bd);
void hlp_map_disconnect(void);
void hlp_map_get_message_listing(int location, UINT16 start_offset, UINT16 max_count);
void hlp_map_get_folder_listing(void);
void hlp_map_send_message(UINT8 *content, UINT16 length);
void hlp_map_get_message(UINT8 *msg_handle);
void hlp_map_delete_message(UINT8 *msg_handle);
UINT8 hlp_map_get_status(void);

#endif // _HLP_MAP_H_


