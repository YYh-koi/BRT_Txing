/*************************************************************************
    > File Name: app_dun.h
    >
    >
    > Created Time: Fri 07 Aug 2015 11:37:12 AM CST
 ************************************************************************/
#include "btdatatype.h"
#include "profile.h"

enum {
    EV_DUN_CONNECTING,
    EV_DUN_CONNECTED,
    EV_DUN_DISCONNECTING,
    EV_DUN_DISCONNECTED,
};

extern profile_t g_dun_profile;

void app_dun_init(module_event_cbk_t event_cbk);
void app_dun_done();

void app_dun_connect(UINT8 *bd);

void app_dun_disconnect();

UINT8 app_dun_spp_event_cbk(UINT8 event, void *param);

void app_dun_service_start();

void app_dun_service_stop();
