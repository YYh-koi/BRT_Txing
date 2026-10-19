/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_cb_screen.h
*
*	Description:
*
*
*	Create Date:		2014/05/29
***********************************************************************/

#ifndef _HLP_CB_SCREEN_H_
#define _HLP_CB_SCREEN_H_

#include "global.h"
#include "auto_cb_screen.h"

#define POINTOFFSET				20/100
#define ACCURACY_OF_CALIBRATE	10

enum {
    ST_CB_SCREEN_UNINIT,
    ST_CB_SCREEN_INIT = 0x01,
    ST_CB_SCREEN_READY,
    ST_CB_SCREEN_STARTING,
    ST_CB_SCREEN_SUCCESS,
};

enum {
    CB_SCREEN_OS_TYPE_ANDROID,
    CB_SCREEN_OS_TYPE_IOS
};

//The OS version of iPhone
enum {
    CB_SCREEN_OS_VERSION_8P4M,
    CB_SCREEN_OS_VERSION_8P4S
};

#define CB_SCREEN_IPHONE_VERSION_UNKNOW			0xFF
#define CB_SCREEN_IPHONE_VERSION_5				0x00
#define CB_SCREEN_IPHONE_VERSION_5S				0x00
#define CB_SCREEN_IPHONE_VERSION_6				0x01
#define CB_SCREEN_IPHONE_VERSION_6PLUS			0x02

struct cb_screen_data_stru {
	UINT16 phone_width;
	UINT16 phone_height;

	UINT16 resolution_w;
	UINT16 resolution_h;

	UINT8 phone_os_type;
	UINT8 os_version;
	UINT8 iphone_version;

	UINT32 state;

	cb_screen_spp_connect spp_connect;
	cb_screen_spp_disconnect spp_disconnect;
	cb_screen_event_cbk event_cbk;
};

typedef struct _cb_screen_spp_data_stru {
	UINT8 sync[2];
	UINT8 type;
	UINT8 length[2];
	UINT8 data[1];
} cb_screen_spp_data_t;

#define PACKET_CONTROL_MASK					(3 << 6)

#define PACKET_TYPE_SCREEN_PIXEL_INFO		(0x10 | PACKET_CONTROL_MASK)
#define PACKET_TYPE_POINT_SPEED_INFO		(0x11 | PACKET_CONTROL_MASK)
#define PACKET_TYPE_SCREEN_ROTATION_INFO	(0x13 | PACKET_CONTROL_MASK)

void hlp_cb_screen_init(struct user_cb_screen_func *p_user_func);
void hlp_cb_screen_done(void);
void hlp_cb_screen_start(UINT8 *bd, UINT16 resolution_w, UINT16 resolution_h);
void hlp_cb_screen_stop(void);
void hlp_cb_screen_clear(void);
void hlp_cb_screen_set_local_info(UINT16 phone_width, UINT16 phone_height);
void hlp_cb_screen_coordinate_data(UINT8 button, UINT16 *x, UINT16 *y);
void hlp_cb_screen_spp_data_ind(UINT8 *p_data, UINT32 len);
UINT8 hlp_cb_screen_get_status(void);
void hlp_cb_screen_set_status(UINT8 state);
#ifdef CONFIG_IAP2
UINT8 hlp_cb_screen_set_os_type(UINT8 *p_os);
UINT8 hlp_cb_screen_get_os_type();
UINT8 hlp_cb_screen_set_os_version(UINT8 *p_version);
UINT8 hlp_cb_screen_get_os_version(void);
void hlp_cb_screen_get_iphone_pixel(UINT16 *width, UINT16 *height);
void hlp_cb_screen_iap_event_cbk(UINT8 event, void *param);
#endif
#endif /*_HLP_CB_SCREEN_H_*/

