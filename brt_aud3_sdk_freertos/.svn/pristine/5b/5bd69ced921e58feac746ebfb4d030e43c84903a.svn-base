/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_pbap.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef APP_PBAP_H
#define APP_PBAP_H

#include "btdatatype.h"
#include "bt_event.h"
#include "internal_pbap.h"
#include "profile.h"

enum {
    EV_PBAP_CONNECT_SUCCESS,
    EV_PBAP_CONNECT_FAIL,
    EV_PBAP_DISCONNECTED,
    EV_PBAP_PHONEBOOK_DATA_IND,
    EV_PBAP_CURR_CALL_PHONEBOOK_IND,
    EV_PBAP_GET_PHONEBOOK_SUCCESS,
    EV_PBAP_GET_PHONEBOOK_FAIL,
    EV_PBAP_GET_PHONEBOOK_SIZE_SUCCESS,
    EV_PBAP_INDEX_DATA_IND,
    EV_PBAP_SUPPORTED_REPOS,
    EV_PBAP_PHONEBOOK_RAWDATA_IND,
    EV_PBAP_PHONEBOOK_PHOTODATA_IND,
    EV_PBAP_STAT_EV,
};

/* Get Phonebook Param Type */
enum {
    PT_PBAP_SIM = 0x00,
    PT_PBAP_TEL,
     PT_PBAP_ICH,
    PT_PBAP_OCH,
    PT_PBAP_MCH,
    //PT_PBAP_ICH,
    PT_PBAP_SIM_IDX,
    PT_PBAP_TEL_IDX,
    PT_PBAP_CCH,
	PT_PBAP_VCARD,
    PT_PBAP_IDLE = 0xFF,
};

extern profile_t g_pbap_profile;

void app_pbap_init(module_event_cbk_t event_cbk);
void app_pbap_server_start(void);
void app_pbap_done(void);
void app_pbap_get_phonebook(UINT8 *bd, UINT8 store_type, UINT16 offset, UINT16 count);
void app_pbap_abort_phonebook(void);
void app_pbap_connect(UINT8 *bd);
void app_pbap_stop(void);
UINT8 app_pbap_get_param_type(void);
void app_pbap_set_req_contact_info(INT32 mask);
void app_pbap_disconnect();
UINT8 app_pbap_get_status(void);
BOOL app_pbap_get_flag_auto_connect(void);

#endif

