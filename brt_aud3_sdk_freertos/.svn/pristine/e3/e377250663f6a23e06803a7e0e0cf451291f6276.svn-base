/***
**  See stack/include/ble_advertiser.h in bluedroid
***/

#include "btdatatype.h"
#include "bt_common_types.h"
#include "bt_gatt_types.h"
//#include "../../include/Types.h"
/* advertising filter policy */
#define AP_SCAN_CONN_ALL 0x00 /* default */
#define AP_SCAN_WL_CONN_ALL 0x01
#define AP_SCAN_ALL_CONN_WL 0x02
#define AP_SCAN_CONN_WL 0x03
#define AP_SCAN_CONN_POLICY_MAX 0x04
typedef uint8_t tBTM_BLE_AFP;

/* default advertising filter policy */
#ifndef BTM_BLE_DEFAULT_AFP
#define BTM_BLE_DEFAULT_AFP AP_SCAN_CONN_ALL
#endif

/* advertising channel map */
#define BTM_BLE_ADV_CHNL_37 (0x01 << 0)
#define BTM_BLE_ADV_CHNL_38 (0x01 << 1)
#define BTM_BLE_ADV_CHNL_39 (0x01 << 2)
typedef uint8_t tBTM_BLE_ADV_CHNL_MAP;

/*d efault advertising channel map */
#ifndef BTM_BLE_DEFAULT_ADV_CHNL_MAP
#define BTM_BLE_DEFAULT_ADV_CHNL_MAP \
  (BTM_BLE_ADV_CHNL_37 | BTM_BLE_ADV_CHNL_38 | BTM_BLE_ADV_CHNL_39)
#endif

typedef struct {
	uint16_t advertising_event_properties;
  	uint32_t adv_int_min;
  	uint32_t adv_int_max;
  	tBTM_BLE_ADV_CHNL_MAP channel_map;                                                                                                  
  	//tBTM_BLE_AFP adv_filter_policy;//FIXME
  	int8_t tx_power;
  	uint8_t primary_advertising_phy;
  	uint8_t secondary_advertising_phy;
  	uint8_t scan_request_notification_enable;
} __attribute__((packed))tBTM_BLE_ADV_PARAMS;

typedef struct {
	uint8_t enable;
  	uint16_t min_interval;
  	uint16_t max_interval;
  	uint16_t periodic_advertising_properties;
} __attribute__((packed))tBLE_PERIODIC_ADV_PARAMS;
