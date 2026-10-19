#include "bt_gatt.h"
#include "auto_gatt_fsm.h"
#include "auto_le_scanner.h"
#include "sysdep.h"

bt_status_t app_gatt_scan_periodic_startsync(int sid, bt_bdaddr_t address, int skip, int timeout, int reg_id)
{
	fsm_msg_gatt_scan_periodic_startsync_t *msg = (fsm_msg_gatt_scan_periodic_startsync_t *) NEW(sizeof(fsm_msg_gatt_scan_periodic_startsync_t));
	msg->sid = sid;
	msg->address = address;
	msg->skip = skip;
	msg->timeout = timeout;
	msg->reg_id = reg_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SCAN_PERIODIC_STARTSYNC, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_scan_periodic_stopsync(int sync_handle)
{
	fsm_msg_gatt_scan_periodic_stopsync_t *msg = (fsm_msg_gatt_scan_periodic_stopsync_t *) NEW(sizeof(fsm_msg_gatt_scan_periodic_stopsync_t));
	msg->sync_handle = sync_handle;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SCAN_PERIODIC_STOPSYNC, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_scan_register_scanner(bt_uuid_t app_uuid)
{
	fsm_msg_gatt_scan_register_scanner_t *msg = (fsm_msg_gatt_scan_register_scanner_t *) NEW(sizeof(fsm_msg_gatt_scan_register_scanner_t));
	msg->app_uuid = app_uuid;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SCAN_REGISTER_SCANNER, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_scan_unregister_scanner(int scanner_id)
{
	fsm_msg_gatt_scan_unregister_scanner_t *msg = (fsm_msg_gatt_scan_unregister_scanner_t *) NEW(sizeof(fsm_msg_gatt_scan_unregister_scanner_t));
	msg->scanner_id = scanner_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SCAN_UNREGISTER_SCANNER, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_scan_client_scan(unsigned char start)
{
	fsm_msg_gatt_scan_client_scan_t *msg = (fsm_msg_gatt_scan_client_scan_t *) NEW(sizeof(fsm_msg_gatt_scan_client_scan_t));
	msg->start = start;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SCAN_CLIENT_SCAN, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_scan_set_scan_parameters(unsigned char client_if, int scan_interval, int scan_window)
{
	fsm_msg_gatt_scan_set_scan_parameters_t *msg = (fsm_msg_gatt_scan_set_scan_parameters_t *) NEW(sizeof(fsm_msg_gatt_scan_set_scan_parameters_t));
	msg->client_if = client_if;
	msg->scan_interval = scan_interval;
	msg->scan_window = scan_window;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SCAN_SET_SCAN_PARAMETERS, (void *)msg);

	return BT_STATUS_SUCCESS;
}

