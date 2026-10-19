#ifndef APP_ANCS_H__
#define APP_ANCS_H__

//#include "ancs_client.h"
#include "profile.h"

enum {
    EV_ANCS_IOS_NOTIFICATION,
    EV_ANCS_CONNECTION_STATE,
    EV_ANCS_NOTIFICATION_ATTRIBUTE,
    EV_ANCS_APP_ATTRIBUTE,
};

enum {
    EV_ANCS_IOS_NOTIFICATION_MESSAGE_TYPE_UN,
    EV_ANCS_IOS_NOTIFICATION_MESSAGE_TYPE_PHONE,
    EV_ANCS_IOS_NOTIFICATION_MESSAGE_TYPE_MOILESMS,
    EV_ANCS_IOS_NOTIFICATION_MESSAGE_TYPE_XIN,
    EV_ANCS_IOS_NOTIFICATION_MESSAGE_TYPE_QQ
};


extern profile_t g_ancs_profile;

void app_ancs_init(module_event_cbk_t callback);

void app_ancs_done();

void app_ancs_start();

void app_ancs_stop();


void app_ancs_get_notification_attribute(unsigned char *uid, unsigned char attr_id, int attr_max_len);

void app_ancs_get_app_attribute(unsigned char *app_id, unsigned char attr_id);

void app_ancs_perform_action(unsigned char *uid, unsigned char action);

void ble_ancs_evt_handler_ex(unsigned char *data, int len);

#endif /*APP_ANCS_H__*/
