#ifndef _AT_INTERNAL_H__
#define _AT_INTERNAL_H__
#include "auto_at.h"

typedef enum
{
	SETTING_ID_ERR = 0,/*!<err id,should not be used*/
	SETTING_ID_EQUALIZER = 1,
	SETTING_ID_REPEAT_MODE = 2,
	SETTING_ID_SHUFFLE = 3,
	SETTING_ID_SCAN = 4
} BTSettingID_e; /*!<the value of enum is based on spec,please do not change.*/

typedef enum
{
	SETTING_VALUE_EQUALIZER_OFF = 0x01,
	SETTING_VALUE_EQUALIZER_ON = 0x02,

	SETTING_VALUE_REPEAT_OFF = 0x01,
	SETTING_VALUE_REPEAT_SINGLE_TRACK = 0x02,
	SETTING_VALUE_REPEAT_ALL_TRACK = 0x03,
	SETTING_VALUE_REPEAT_GROUP = 0x04,

	SETTING_VALUE_SCAN_OFF = 0x01,
	SETTING_VALUE_SCAN_ALL_TRACK = 0x02,
	SETTING_VALUE_SCAN_GROUP = 0x03,

	SETTING_VALUE_SHUFFLE_OFF = 0x01,
	SETTING_VALUE_SHUFFLE_ALL_TRACK  = 0x02,
	SETTING_VALUE_SHUFFLE_GROUP = 0x03,

	SETTING_VALUE_PLAYER_NOT_SUPPORTED = 0xFD,/*!<indicate remote device do not support this setting.*/
	SETTING_VALUE_STATUS_NOT_CHANGED = 0xFE,/*!<indicate this setting status is not changed.*/
	SETTING_VALUE_ERR = 0xFF /*!<err value,should not use*/
} BTSettingValue_e; /*!<the value of enum is based on spec,please do not change.*/

void at_process_internal();

void bt_event_lock_init(void);
void bt_event_lock_done(void);
void at_register_event_callback_internal(at_event_callback cb);
void at_register_event_len_callback_internal(at_event_len_callback cb);
void at_process_interpret(const char *head, const char *data, unsigned short len);
void at_process_interpret_ex(const char *head, const char *data, unsigned short len);
void bt_event_indicate(unsigned int event_code, void *param);
void bt_set_scan_name_buff(char *p, int len);
void bt_cancel_scan_name_buff();

#endif

