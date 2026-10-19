/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2015 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_iap.h
*
*	Description:		Internal Functions For IAP
*
*
*	Create Date:		2015/07/28
***********************************************************************/

#ifndef __HLP_IAP_H__
#define __HLP_IAP_H__

#include "global.h"
#include "bt_event.h"

#define IAP_MOUSE_STEP_LENGTH	127

//The register address define for apple coprocesser
#define CP_AUTH_PROTOCOL_MAJOR_VERSION_REG				0x02
#define CP_AUTH_PROTOCOL_MINOR_VERSION_REG				0x03

#define CP_CERTIFICATE_DATA_PAGE_SIZE					0x80
#define CP_CERTIFICATE_DATA_PAGE_TOTAL					0x0a
#define CP_CERTIFICATE_DATA_SIZE_LIMIT					(CP_CERTIFICATE_DATA_PAGE_SIZE*CP_CERTIFICATE_DATA_PAGE_TOTAL)

#define CP_SIGNATURE_DATA_PAGE_SIZE						0x80
#define CP_SIGNATURE_DATA_PAGE_TOTAL					0x01
#define CP_SIGNATURE_DATA_SIZE_LIMIT					(CP_SIGNATURE_DATA_PAGE_SIZE*CP_SIGNATURE_DATA_PAGE_TOTAL)

#define CP_AUTH_CTL_AND_STATUS_REG					0x10
#define CP_AUTH_CTL_AND_STATUS_GET_CTL_BIT(_x)				(_x & 0x0F)
#define CP_AUTH_CTL_AND_STATUS_GET_STATUS_BIT(_x)			((_x >> 4) & 0x07)
#define CP_AUTH_CTL_AND_STATUS_GET_ERR_BIT(_x)				(_x >> 7)

#define CP_SIGNATURE_DATA_LENGTH_REG					0x11
#define CP_SIGNATURE_DATA_REG							0x12

#define CP_CHALLENGR_DATA_LENGTH_REG					0x20
#define CP_CHALLENGR_DATA_LENGTH_REG_SIZE					0x02
#define CP_CHALLENGR_DATA_REG							0x21

#define CP_CERTIFICATE_DATA_LENGTH_REG					0x30
#define CP_CERTIFICATE_DATA_PAGE_START_REG				0x31

typedef struct _iap_cp_data_stru {
	UINT8 *p_buffer;
	UINT16 data_cur;
	UINT16 data_len;
} iap_cp_data_t;

typedef struct _iap_data_stru {
	module_event_cbk_t event_cbk;
	UINT32 svr_hdl;
	UINT32 conn_hdl;
	UINT32 local_srv_chl;

	UINT8 conn_bd[BD_ADDR_LEN];
	UINT8 flag_out_index;
	UINT8 out_index;
	UINT8 flag_fc;
	UINT8 state;
	UINT16 out_size;
	iap_cp_data_t p_cp_data;
} iap_data_t;

typedef struct iap_hid_report_stru {
	UINT8	ctrl;
	CHAR	x;
	CHAR	y;

	union {							/* Control Button List */
		struct {
			UINT8 ac_menu : 1;
			UINT8 ac_search : 1;
			UINT8 ac_screen_saver : 1;
			UINT8 ac_keyboard_layout : 1;
			UINT8 scan_pre_track : 1;
			UINT8 play_pause : 1;
			UINT8 scan_next_track : 1;
			UINT8 mute : 1;
		} control_bit_1;
		UINT8 control_uint8_1;
	} u1;
	union {
		struct {
			UINT8 volume_down : 1;
			UINT8 volume_up : 1;
			UINT8 power : 1;
			UINT8 reserve : 5;
		} control_bit_2;
		UINT8 control_uint8_2;
	} u2;
} iap_hid_report_t;

typedef struct _iap_tl_event_report_stru {
	UINT8 event;
	UINT8 bd[BD_ADDR_LEN];
} iap_tl_event_report_t;

//Interface
void hlp_iap_init(module_event_cbk_t event_cbk);
void hlp_iap_done();
void hlp_iap_start(void);
void hlp_iap_connect(UINT8 *bd);
void hlp_iap_disconnect();
void hlp_iap_hid_start(void);
void hlp_iap_hid_stop(void);
void hlp_iap_send_hid_mouse(UINT8 button, CHAR x, CHAR y);
void hlp_iap_mouse_coord(UINT8 ctrl, INT16 x, INT16 y);
void hlp_iap_send_key_value(UINT8 key_value_1, UINT8 key_value_2);
void hlp_iap_set_status(UINT8 status);
UINT8 hlp_iap_get_status(void);
void hlp_iap_rfcomm_event_cbk(UINT8 event, void *param);

#endif	/* __HLP_IAP_H__ */


