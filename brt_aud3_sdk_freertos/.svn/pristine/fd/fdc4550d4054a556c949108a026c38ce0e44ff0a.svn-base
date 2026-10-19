/*************************************************************************
    > File Name: auto_app/photo_parser.h
    >
    >
    > Created Time: Sun 14 Feb 2016 03:02:50 PM CST
 ************************************************************************/
 #include "vcf_parse.h"
#define MAX_PHOTO_SIZE (50 * 1024)

typedef struct _PBITEMP {
	unsigned char sNumber[MAX_VCF_NUMBER_LEN];
	unsigned char sName[MAX_VCF_NAME_LEN];
	unsigned char sTime[MAX_VCF_TIME_LEN];
	unsigned char sLine[MAX_PHOTO_SIZE];
	unsigned char sMsg_length;
	unsigned char *sMsg;
	unsigned char msg_handle[17];
	unsigned short nCount;
	unsigned short pb_index;
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
	unsigned char numberType;
#endif
	unsigned char call_log_type;
} PBITEMP;

typedef struct _photo_parser {
#ifdef VCF_PARSER_DEBUG_ENABLE
	UINT8 parser_name[VCF_PARSER_NAME_LENGTH];
#endif
	PBITEMP *p_pb_item;
	UINT8 *p_item_name_start;
	UINT8 *p_item_name_end;
	UINT8 *p_item_prop_start;
	UINT8 *p_item_prop_end;
	UINT8 *p_item_val_start;
	UINT8 *p_item_val_end;

	BOOL b_continue;
	UINT8 is_encoding_QP; //QP: QUOTED-PRINTABLE
	UINT16 item_type;
	UINT8 store_type;
	UINT8 status;

	/*App set this callback which will be called when vcard item is parsed.
	  For example, a pbap's contact is parsed*/
	vcard_item_ind_callback item_callback;
} photo_parser_t;

photo_parser_t *app_photo_parser_alloc(UINT8 store_type, UINT8 *p_parser_name);
void app_photo_parser_free(photo_parser_t *p_parser);