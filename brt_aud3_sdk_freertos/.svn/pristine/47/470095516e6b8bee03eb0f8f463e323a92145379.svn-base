/*************************************************************************
    > File Name: auto_app/app_pan.h
    > Author: hualiangyu
    > Mail: sheldon_wy@126.com
    > Created Time: Fri 26 Jun 2015 09:38:13 AM CST
 ************************************************************************/
#include "btdatatype.h"
#include "profile.h"

enum {
    EV_PAN_CONNECTING,
    EV_PAN_CONNECTED,
    EV_PAN_DISCONNECTING,
    EV_PAN_DISCONNECTED,
    EV_PAN_CONTROL_STATE,
};

extern profile_t g_pan_profile;

void app_pan_init(module_event_cbk_t event_cbk);
void app_pan_connect(UINT8 *bd);
void app_pan_disconnect();
void app_pan_done();

