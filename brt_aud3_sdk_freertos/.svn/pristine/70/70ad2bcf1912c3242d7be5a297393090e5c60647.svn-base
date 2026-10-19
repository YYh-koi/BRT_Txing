#ifndef _APP_OPP_H_
#define _APP_OPP_H_

#include "btdatatype.h"
#include "profile.h"

#define OPP_PATH_MAX_LEN (256)
#define BD_NAME_LEN (20)

enum app_opp_event_type {
    EV_OPP_PUSH_STARTED,
    EV_OPP_PUSH_COMPLETED,
    EV_OPP_PUSH_IN_PROGRESS,
    EV_OPP_PUSH_ABORTED,
    EV_OPP_PULL_STARTED,
    EV_OPP_PULL_COMPLETED,
    EV_OPP_PULL_IN_PROGRESS,
    EV_OPP_PULL_ABORTED,
};

enum app_opp_error_type {
    OPP_RESULT_OK = 0,
    OPP_RESULT_ERROR_INVALID_PARAMETER,
    OPP_RESULT_ERROR_MEMORY_UNAVAILABLE,
    OPP_RESULT_ERROR_NOT_PERMISSION,
    OPP_RESULT_ERROR_CONNECT_FAILED,
    OPP_RESULT_ERROR_UNKNOWN,
};

extern profile_t g_opp_profile;

void app_opp_init(module_event_cbk_t p_event_cbk);
void app_opp_set_save_path(const char *save_path);
void app_opp_push_file(const char *path, UINT8 *bd);
void app_opp_pull_vcard(UINT8 *bd);
void app_opp_abort();
void app_opp_done();

#endif //_APP_OPP_H_

