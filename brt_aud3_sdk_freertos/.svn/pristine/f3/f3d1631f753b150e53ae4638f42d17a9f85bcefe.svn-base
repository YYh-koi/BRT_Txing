#ifndef __APP_LE_SCANNER_H__
#define __APP_LE_SCANNER_H__

bt_status_t app_gatt_scan_periodic_startsync(int sid, bt_bdaddr_t address, int skip, int timeout, int reg_id);
bt_status_t app_gatt_scan_periodic_stopsync(int sync_handle);
bt_status_t app_gatt_scan_register_scanner(bt_uuid_t app_uuid);
bt_status_t app_gatt_scan_unregister_scanner(int scanner_id);
bt_status_t app_gatt_scan_client_scan(unsigned char start);
bt_status_t app_gatt_scan_set_scan_parameters(unsigned char client_if, int scan_interval, int scan_window);

#endif /* __APP_LE_SCANNER_H__ */
