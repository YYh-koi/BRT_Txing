#ifndef _VCF_PARSE_H
#define _VCF_PARSE_H

#include "btdatatype.h"

#define VCF_PARSER_DEBUG_ENABLE

#define PBAP_VCF_PARSE_CARE_TEL_TYPE

#define VCF_PARSER_NAME_LENGTH	30

enum {
    PARSE_IDLE,
    PARSE_ITEM_NAME_GOT,
    PARSE_ITEM_PROP_GOT,
    PARSE_ITEM_VAL_GOT
};

enum {
    VCF_PARSE_TYPE_NONE = 0x00,
    VCF_PARSE_TYPE_VCARD,
    VCF_PARSE_TYPE_BMSG = 0x05,
    VCF_PARSE_TYPE_BENV,
    VCF_PARSE_TYPE_BBODY,
    VCF_PARSE_TYPE_MSG = 0x0b,
};

//For Number Type
/*
"home"��ʾ��ͥ�绰
"msg" ��ʾ�������֧������
"work" �����绰
"pref" ��ʾ����绰����ϲ��ʹ�õĵ绰
"voice" �����绰����
"fax"�������
"cell" ��ʾ�ֻ��绰
"video" ��Ƶ�绰
"pager" ������
"bbs" �����Ĺ㲥ϵͳ�ĵ绰
"modem" ���ƽ�����绰
"car"���ص绰
"isdn" ISDN���ӵ绰����
"pcs" ����ͨ�ŷ���绰
ȱʡ�� "voice".
*/
enum {
    NUMBER_TYPE_HOME = 1,
	NUMBER_TYPE_HOME_FAX,
    NUMBER_TYPE_MSG,
    NUMBER_TYPE_WORK,
	NUMBER_TYPE_WORK_FAX,
    NUMBER_TYPE_PREF,
    NUMBER_TYPE_FAX,
    NUMBER_TYPE_CELL,
    NUMBER_TYPE_VIDEO,
    NUMBER_TYPE_PAGER,
    NUMBER_TYPE_BBS,
    NUMBER_TYPE_MODEM,
    NUMBER_TYPE_CAR,
    NUMBER_TYPE_ISDN,
    NUMBER_TYPE_PCS,
    NUMBER_TYPE_VOICE,
};

#define MAX_VCF_NUMBER_LEN	20
#ifdef CONFIG_UNLIMIT_RESOURCE
#define MAX_VCF_NAME_LEN	250//80
#define MAX_VCF_ITEM_LEN	1024
#else
#define MAX_VCF_NAME_LEN	40
#define MAX_VCF_ITEM_LEN    512//255
#endif
#define MAX_VCF_TIME_LEN	40
#define MAX_VCF_MSG_LEN		195

#define VCF_CONTENT_NAME_LEN	255
#define VCF_CONTENT_PROP_LEN	255
#define VCF_CONTENT_VAL_LEN		(1024+6)

enum {
    PBAP_VCF_CALLLOG_DIALED = 0x08,
    PBAP_VCF_CALLLOG_MISSED,
    PBAP_VCF_CALLLOG_RECEIVED,
    PBAP_VCF_CALLLOG_UNKOWN	= 0x0F
};

#if 0
typedef struct _PBITEM {
	unsigned char sNumber[MAX_VCF_NUMBER_LEN];
	unsigned char sName[MAX_VCF_NAME_LEN];
	unsigned char sTime[MAX_VCF_TIME_LEN];
	unsigned char sLine[MAX_VCF_ITEM_LEN];
	unsigned char sMsg_length;
	unsigned char *sMsg;
	unsigned char msg_handle[17];
	unsigned short nCount;
	unsigned short pb_index;
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
	unsigned char numberType;
#endif
	unsigned char call_log_type;
} PBITEM;
#endif

#define PB_BUFF_NUMBER_MAX 15

typedef struct _vcf_content_buff{
	unsigned char sName[MAX_VCF_NAME_LEN];
	unsigned char sNumber[PB_BUFF_NUMBER_MAX][MAX_VCF_NUMBER_LEN];
	unsigned int numberType[PB_BUFF_NUMBER_MAX];
	unsigned short pb_index;
	//unsigned int curr_index;
}vcf_content_buff;

typedef struct _PBITEM {
	unsigned char sNumber[MAX_VCF_NUMBER_LEN];
	unsigned char sName[MAX_VCF_NAME_LEN];
	unsigned char sTime[MAX_VCF_TIME_LEN];
	unsigned char sLine[MAX_VCF_ITEM_LEN];
	unsigned char sRawName[MAX_VCF_NAME_LEN + 4];
	unsigned short sMsg_length;
	unsigned char *sMsg;
	unsigned char msg_handle[17];
	unsigned char msg_status;
	unsigned char msg_type[32];
	unsigned char msg_folder[32];
	unsigned short nCount;
	unsigned short pb_index;
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
	unsigned char numberType;
#endif
	unsigned char call_log_type;
} PBITEM;

typedef struct _PBNAMESTRU {
	unsigned char lastName[MAX_VCF_NAME_LEN];
	unsigned char firstName[MAX_VCF_NAME_LEN];
	unsigned char middleName[MAX_VCF_NAME_LEN];
	unsigned char prefix[MAX_VCF_NAME_LEN];
	unsigned char suffix[MAX_VCF_NAME_LEN];
} PBNAMESTRU;

typedef void (*vcard_item_ind_callback)(int event, void *param);

#if 0
typedef struct _vcf_parser {
#ifdef VCF_PARSER_DEBUG_ENABLE
	UINT8 parser_name[VCF_PARSER_NAME_LENGTH];
#endif
	PBITEM *p_pb_item;
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
} vcf_parser_t;
#endif
typedef struct _vcf_content_t{    
	UINT8 name[VCF_CONTENT_NAME_LEN];
	UINT8 prop[VCF_CONTENT_PROP_LEN];
	UINT8 val[VCF_CONTENT_VAL_LEN];
} vcf_content_t;

typedef struct _vcf_parser {
#ifdef VCF_PARSER_DEBUG_ENABLE
	UINT8 parser_name[VCF_PARSER_NAME_LENGTH];
#endif
	UINT8 bd_addr[BD_ADDR_LEN];
	PBITEM *p_pb_item;
	vcf_content_t content;//store vcf data:item,prop,val 
	//UINT16 p_item_name_start;
	UINT16 p_item_name_end;//name len
	//UINT16 p_item_prop_start;
	UINT16 p_item_prop_end;//prop len
	//UINT16 p_item_val_start;
	UINT16 p_item_val_end;//val len

	BOOL b_continue;
	UINT8 is_encoding_QP; //QP: QUOTED-PRINTABLE
	UINT16 item_type;
	UINT8 store_type;
	UINT8 status;

	/*App set this callback which will be called when vcard item is parsed.
	  For example, a pbap's contact is parsed*/
	vcard_item_ind_callback item_callback;
} vcf_parser_t;

vcf_parser_t *app_vcfparser_alloc(UINT8 store_type, UINT8 *p_parser_name);
void app_vcfparser_data_ind(vcf_parser_t *p_parser, UINT8 *p_input, UINT16 data_len, UINT16 offset);
void app_vcfparser_stor(vcf_parser_t *p_parser);
void app_vcfparser_free(vcf_parser_t *p_parser, int is_done);
int hlp_event_last_day_pbap_packet(int pbap_c_flag);
void app_vcfparser_create();

typedef struct {
    UINT8 handle[32];
    UINT8 name[MAX_VCF_NAME_LEN];//32
    UCHAR number[32];
}vcard_xml_entry_t;
void app_process_obex_xml_vcard_listing(UINT8 *obex_data, UINT16 data_len, vcard_xml_entry_t *first_entry);
#endif	/* _VCF_PARSE_H */

