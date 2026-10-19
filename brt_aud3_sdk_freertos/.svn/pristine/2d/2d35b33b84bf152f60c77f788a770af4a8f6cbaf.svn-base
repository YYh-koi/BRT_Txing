/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2015 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_iap.h
*
*	Description:		Internal Functions For IAP
*
*
*	Create Date:		2015/07/28
***********************************************************************/

#ifndef __APP_IAP_H__
#define __APP_IAP_H__

enum {
    EV_IAP_STARTED,
    EV_IAP_STOPPED
};

//Interface
void app_iap_init(module_event_cbk_t event_cbk);
void app_iap_done();
void app_iap_start(void);
void app_iap_connect(UINT8 *bd);
void app_iap_disconnect(void);
void app_iap_hid_start(void);
void app_iap_hid_stop(void);
void app_iap_rfcomm_cbk(UINT8 event, void *param);
void app_iap_send_mouse_data(UINT8 button, INT16 x, INT16 y);
void app_iap_send_mouse_test(UINT8 button, INT16 x, INT16 y);
void app_iap_send_key_value(UINT8 key_value_1, UINT8 key_value_2);

#endif	/* __APP_IAP_H__ */


