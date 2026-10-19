#include "auto_gatt_fsm.h"
#include "auto_le_advertiser.h"
#include "sysdep.h"

bt_status_t app_gatt_advm_start_advertising_set(int duration, int max_ext_adv_events, int reg_id, short param_len, unsigned char * param, short adv_data_len, unsigned char * adv_data, short scan_resp_data_len, unsigned char * scan_resp_data, short periodic_adv_data_len, unsigned char * period_adv_data, short periodic_data_len, unsigned char * periodic_data)
{
	fsm_msg_gatt_advm_start_advertising_set_t *msg = (fsm_msg_gatt_advm_start_advertising_set_t *) NEW(sizeof(fsm_msg_gatt_advm_start_advertising_set_t));
	msg->param_len = param_len;
	if (msg->param_len > 0) {
		msg->param = (unsigned char *) NEW(msg->param_len);

		if (msg->param) {
			memcpy(msg->param, param, msg->param_len);
		}
	}
	msg->adv_data_len = adv_data_len;
	if (msg->adv_data_len > 0) {
		msg->adv_data = (unsigned char *) NEW(msg->adv_data_len);

		if (msg->adv_data) {
			memcpy(msg->adv_data, adv_data, msg->adv_data_len);
		}
	}
	msg->scan_resp_data_len = scan_resp_data_len;
	if (msg->scan_resp_data_len > 0) {
		msg->scan_resp_data = (unsigned char *) NEW(msg->scan_resp_data_len);

		if (msg->scan_resp_data) {
			memcpy(msg->scan_resp_data, scan_resp_data, msg->scan_resp_data_len);
		}
	}
	msg->periodic_adv_data_len = periodic_adv_data_len;
	if (msg->periodic_adv_data_len > 0) {
		msg->period_adv_data = (unsigned char *) NEW(msg->periodic_adv_data_len);

		if (msg->period_adv_data) {
			memcpy(msg->period_adv_data, period_adv_data, msg->periodic_adv_data_len);
		}
	}
	msg->periodic_data_len = periodic_data_len;
	if (msg->periodic_data_len > 0) {
		msg->periodic_data = (unsigned char *) NEW(msg->periodic_data_len);

		if (msg->periodic_data) {
			memcpy(msg->periodic_data, periodic_data, msg->periodic_data_len);
		}
	}
	msg->duration = duration;
	msg->max_ext_adv_events = max_ext_adv_events;
	msg->reg_id = reg_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_START_ADVERTISING_SET, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_getownaddress(int advertiser_id)
{
	fsm_msg_gatt_advm_getownaddress_t *msg = (fsm_msg_gatt_advm_getownaddress_t *) NEW(sizeof(fsm_msg_gatt_advm_getownaddress_t));
	msg->advertiser_id = advertiser_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_GETOWNADDRESS, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_stopadvertisingset(int advertiser_id)
{
	fsm_msg_gatt_advm_stopadvertisingset_t *msg = (fsm_msg_gatt_advm_stopadvertisingset_t *) NEW(sizeof(fsm_msg_gatt_advm_stopadvertisingset_t));
	msg->advertiser_id = advertiser_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_STOPADVERTISINGSET, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_enableadvertisingset(int advertiser_id, unsigned char enable, int duration, int max_ext_adv_events)
{
	fsm_msg_gatt_advm_enableadvertisingset_t *msg = (fsm_msg_gatt_advm_enableadvertisingset_t *) NEW(sizeof(fsm_msg_gatt_advm_enableadvertisingset_t));
	msg->advertiser_id = advertiser_id;
	msg->enable = enable;
	msg->duration = duration;
	msg->max_ext_adv_events = max_ext_adv_events;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_ENABLEADVERTISINGSET, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_set_advertising_data(int advertiser_id, short data_len, unsigned char * data)
{
	fsm_msg_gatt_advm_set_advertising_data_t *msg = (fsm_msg_gatt_advm_set_advertising_data_t *) NEW(sizeof(fsm_msg_gatt_advm_set_advertising_data_t));
	msg->advertiser_id = advertiser_id;
	msg->data_len = data_len;
	if (msg->data_len > 0) {
		msg->data = (unsigned char *) NEW(msg->data_len);

		if (msg->data) {
			memcpy(msg->data, data, msg->data_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_SET_ADVERTISING_DATA, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_set_scan_response_data(int advertiser_id, short data_len, unsigned char * data)
{
	fsm_msg_gatt_advm_set_scan_response_data_t *msg = (fsm_msg_gatt_advm_set_scan_response_data_t *) NEW(sizeof(fsm_msg_gatt_advm_set_scan_response_data_t));
	msg->advertiser_id = advertiser_id;
	msg->data_len = data_len;
	if (msg->data_len > 0) {
		msg->data = (unsigned char *) NEW(msg->data_len);

		if (msg->data) {
			memcpy(msg->data, data, msg->data_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_SET_SCAN_RESPONSE_DATA, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_set_advertising_parameters(int advertiser_id, short param_len, unsigned char * param)
{
	fsm_msg_gatt_advm_set_advertising_parameters_t *msg = (fsm_msg_gatt_advm_set_advertising_parameters_t *) NEW(sizeof(fsm_msg_gatt_advm_set_advertising_parameters_t));
	msg->advertiser_id = advertiser_id;
	msg->param_len = param_len;
	if (msg->param_len > 0) {
		msg->param = (unsigned char *) NEW(msg->param_len);

		if (msg->param) {
			memcpy(msg->param, param, msg->param_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_SET_ADVERTISING_PARAMETERS, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_set_periodic_advertising_parameters(int advertiser_id, short param_len, unsigned char * param)
{
	fsm_msg_gatt_advm_set_periodic_advertising_parameters_t *msg = (fsm_msg_gatt_advm_set_periodic_advertising_parameters_t *) NEW(sizeof(fsm_msg_gatt_advm_set_periodic_advertising_parameters_t));
	msg->advertiser_id = advertiser_id;
	msg->param_len = param_len;
	if (msg->param_len > 0) {
		msg->param = (unsigned char *) NEW(msg->param_len);

		if (msg->param) {
			memcpy(msg->param, param, msg->param_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_PARAMETERS, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_set_periodic_advertising_data(int advertiser_id, short data_len, unsigned char * data)
{
	fsm_msg_gatt_advm_set_periodic_advertising_data_t *msg = (fsm_msg_gatt_advm_set_periodic_advertising_data_t *) NEW(sizeof(fsm_msg_gatt_advm_set_periodic_advertising_data_t));
	msg->advertiser_id = advertiser_id;
	msg->data_len = data_len;
	if (msg->data_len > 0) {
		msg->data = (unsigned char *) NEW(msg->data_len);

		if (msg->data) {
			memcpy(msg->data, data, msg->data_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_DATA, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_advm_set_periodic_advertising_enable(int advertiser_id, unsigned char enable)
{
	fsm_msg_gatt_advm_set_periodic_advertising_enable_t *msg = (fsm_msg_gatt_advm_set_periodic_advertising_enable_t *) NEW(sizeof(fsm_msg_gatt_advm_set_periodic_advertising_enable_t));
	msg->advertiser_id = advertiser_id;
	msg->enable = enable;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_ENABLE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

