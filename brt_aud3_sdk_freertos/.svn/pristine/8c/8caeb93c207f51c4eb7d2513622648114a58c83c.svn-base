#ifndef BTSDK_AT_SOCK_
#define BTSDK_AT_SOCK_
#include "btdatatype.h"
#include "brtbt_interface.h"

#ifndef MODULE_TAG
#define MODULE_TAG "brlinkd"
#endif

#define MAX_SAVED_DEVICE_COUNT 8
#define ADDR_LEN 13  // "001583AABBCC\0"
#define NAME_LEN 21

// status bits
#define A2DP_CONNECTION_MASK        0x01
#define A2DP_PLAYING_MASK           0x02
#define HFP_CONNECTION_MASK         0x04
#define HFP_INCOMING_CALL_MASK      0x08
#define HFP_OUTGOING_CALL_MASK      0x10
#define HFP_ONGOING_MASK            0x20

typedef struct tag_bt_device {
	char address[ADDR_LEN];
	char name[NAME_LEN];
} bt_device_t;

typedef struct tag_brlink_data {
	int status; // HFP/A2DP status
	int battery_value;
	int signal_strength;
	int saved_device_num;
	bt_device_t saved_device_list[MAX_SAVED_DEVICE_COUNT];
	int current_dev_index;
	char pincode[4];
} brlink_data_t;

void interrupt_handler(int signal);
unsigned int hw_get_bt_en_pin();
unsigned int hw_get_uartport();
unsigned int hw_get_bt_module();
int sw_get_a2dp_resampler();
int sw_get_debug_mode();
int sw_get_ble_connection_type();
void sw_play_state_register_cb(bt_play_state_callback play_state_cb);
void sw_a2dp_data_register_cb(bt_pcm_data_callback a2dp_data_cb);
void sw_hfp_spk_register_cb(bt_pcm_data_callback hfp_spk_cb);
void sw_bip_coverart_register_cb(bt_coverart_data_callback coverart_cb);
int sw_get_avrcp_bip_permission();
#endif //BTSDK_AT_SOCK_

