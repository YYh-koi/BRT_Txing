#ifndef __APP_LE_ADVERTISER_H__
#define __APP_LE_ADVERTISER_H__

bt_status_t app_gatt_advm_start_advertising_set(int duration, int max_ext_adv_events, int reg_id, short param_len, unsigned char * param, short adv_data_len, unsigned char * adv_data, short scan_resp_data_len, unsigned char * scan_resp_data, short periodic_adv_data_len, unsigned char * period_adv_data, short periodic_data_len, unsigned char * periodic_data);
bt_status_t app_gatt_advm_getownaddress(int advertiser_id);
bt_status_t app_gatt_advm_stopadvertisingset(int advertiser_id);
bt_status_t app_gatt_advm_enableadvertisingset(int advertiser_id, unsigned char enable, int duration, int max_ext_adv_events);
bt_status_t app_gatt_advm_set_advertising_data(int advertiser_id, short data_len, unsigned char * data);
bt_status_t app_gatt_advm_set_scan_response_data(int advertiser_id, short data_len, unsigned char * data);
bt_status_t app_gatt_advm_set_advertising_parameters(int advertiser_id, short param_len, unsigned char * param);
bt_status_t app_gatt_advm_set_periodic_advertising_parameters(int advertiser_id, short param_len, unsigned char * param);
bt_status_t app_gatt_advm_set_periodic_advertising_data(int advertiser_id, short data_len, unsigned char * data);
bt_status_t app_gatt_advm_set_periodic_advertising_enable(int advertiser_id, unsigned char enable);

#endif /* __APP_LE_ADVERTISER_H__ */
