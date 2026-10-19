/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_stream.h
*
*	Description:
*
*
*	Create Date:		2014/07/12
***********************************************************************/

#ifndef _APP_STREAM_H_
#define _APP_STREAM_H_

#include "global.h"
#include "btlist.h"
#include "internal_stream.h"

#define CONFIG_STREAM_NAME_LEN			(10)
#define CONFIG_STREAM_REOPERTOR_TIMEOUT	(1000)
#define CONFIG_STREAM_REOPERTOR_NUM		(2)

/* State */
enum {
    APP_ST_STREAM_INIT,
    APP_ST_STREAM_READY,
    APP_ST_STREAM_STARTING,
    APP_ST_STREAM_WORKING,
    APP_ST_STREAM_STOPPING,
    APP_ST_STREAM_END
};

/* Event */
enum {
    APP_EV_STREAM_STARTED,
    APP_EV_STREAM_STOPPED,
    APP_EV_STREAM_START_FAIL,
    APP_EV_STREAM_STOP_FAIL,
    APP_EV_STREAM_PATCH_COMPLETE,
    APP_EV_STREAM_END
};

enum {
    STREAM_CTL_TYPE_START,
    STREAM_CTL_TYPE_STOP
};

typedef struct _stream_list_stru {
	UINT8 stream_type;
	UINT8 name[CONFIG_STREAM_NAME_LEN];
	UINT16 acl_handle;
	UINT16 cid;
} stream_list_t;

typedef struct _stream_oper_stru {
	UINT8 ctl_type;
	UINT8 stream_type;
	UINT16 acl_handle;
	UINT16 cid;
} stream_oper_t;

typedef struct _app_stream_data_stru {
	struct BtList *stream_list;
	struct BtList *oper_queue;
	struct FsmTimer *re_oper;
	stream_oper_t *cur_oper;
	UINT8 cur_stream_type;
} app_stream_data_t;

void app_stream_init(void);
void app_stream_open(UINT8 stream_type, UINT8 *p_name, UINT16 acl_handle, UINT16 cid);
void app_stream_start(UINT8 stream_type);
void app_stream_stop(UINT8 stream_type);
void app_stream_close(UINT8 stream_type, UINT8 *p_name);
void app_stream_test_count_oper(void);
void app_stream_event_cbk(UINT8 event_id, void *param);
UINT8 app_stream_get_type(void);
void  app_stream_set_type(UINT8 stream_type);
UINT8 app_stream_get_status(void);
void  app_stream_set_status(UINT8 status);
void app_stream_close_ringtone(void);
void app_stream_mic_mute(unsigned int flag);
void app_stream_fadeout_ringtone(void);
#endif	/* _APP_STREAM_H_ */

