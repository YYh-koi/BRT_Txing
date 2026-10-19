/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_pbap.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _HLP_PBAP_H_
#define _HLP_PBAP_H_

#include "global.h"
#include "bt_event.h"
#include "vcf_parse.h"

#define PBAP_INFO_MASK_FN    (1 << 0)
#define PBAP_INFO_MASK_N     (1 << 1)
#define PBAP_INFO_MASK_TEL   (1 << 2)
#define PBAP_INFO_MASK_EMAIL (1 << 3)
#define PBAP_INFO_MASK_PHOTO (1 << 4)

enum {
    ST_PBAP_UNINIT = 0x00,
    ST_PBAP_INIT,
    ST_PBAP_READY,
    ST_PBAP_CONNECTING,
    ST_PBAP_CONNECTED,
    ST_PBAP_GETTING,
    ST_PBAP_DONE,
    ST_PBAP_END,
};

typedef struct {
	UCHAR	*pse_userid;
	UCHAR	*pse_password;
	WCHAR	path[20];
	UINT16	offset;
	UINT16	pb_size;
	UINT8	auth_failed;
} tApp_GetPBContextStru;

struct PBAPUserDataStru {
	module_event_cbk_t event_cbk;
	vcf_parser_t *p_vcf_parser;
	void *pb_handle;
	UINT16 offset;
	UINT16 max_list_count;
	UINT8 storage;			//-1 means not set
	UINT8 flag_auto_connect;

	UINT8 state;
	UINT8 supported_repos;

	INT32 pbap_info_mask;
};

#ifdef CONFIG_PBAP_PHOTO
struct PBAPUserDataStruP {
	photo_parser_t *p_photo_parser;
	void *pb_handle;
	UINT8 storage;			//-1 means not set
};
#endif
#define PBAP_USER_DATA				(s_pbap_user_data)
#define PBAP_STATE					(s_pbap_user_data->state)
#define PBAP_VCF_PARSER_POINTRT		(s_pbap_user_data->p_vcf_parser)
#ifdef CONFIG_PBAP_PHOTO
// photo parser
#define PBAP_PHOTO_USER_DATA				(s_pbap_user_data_p)
#define PBAP_PHOTO_PARSER_POINTRT           (s_pbap_user_data_p->p_photo_parser)
#endif

void hlp_pbap_init(module_event_cbk_t event_cbk);
void hlp_pbap_server_start(void);
void hlp_pbap_server_stop(void);
void hlp_pbap_done(void);
void hlp_pbap_connect(UINT8 *bd, UCHAR *pse_userid, UCHAR *pse_password);
void hlp_pbap_set_phonebook_param(UINT8 store_type, UINT16 offset, UINT16 count);
void hlp_pbap_get_phonebook(void);

void hlp_pbap_set_status(UINT8 status);
UINT8 hlp_pbap_get_status(void);
UINT8 hlp_pbap_get_supported_repos();
void *hlp_pbap_get_handle(void);
BOOL hlp_pbap_get_flag_auto_connect(void);
void hlp_pbap_set_flag_auto_connect(BOOL flag);
void hlp_pbap_set_req_contact_info(INT32 mask);
void hlp_pbap_disconnect();
void hlp_pbap_abort_phonebook();
void event_pbap_indication(UINT8 event, void *param);
int hlp_pbap_get_storage();
#endif

