/**
  * This file process the logic associated with GATT Data Transport Mode.
  */

#ifndef _AT_DATA_TRANSPARENT_TRANSMISSION_MODE_H__
#define _AT_DATA_TRANSPARENT_TRANSMISSION_MODE_H__
#include "btdatatype.h"
#include "bluedroid/bluetooth.h"

typedef struct tag_gatt_data_transport_cb {
	UINT8       is_DTTMode; /* whether is in Data Transparent Transmission Mode.*/
	UINT16      uuid16;
	bt_bdaddr_t address;
	UINT16      client_if;
	UINT16      conn_id;
} gatt_data_transport_t;

void at_process_gatt_DTTMode_init();
unsigned char at_process_gatt_DTTMode(char *input_cmd);

#endif  /*_AT_DATA_TRANSPARENT_TRANSMISSION_MODE_H__*/
