#include <stdlib.h> /* for randon*/
#include "bluedroid/bluetooth.h"
#include "btdatatype.h"
#include "att_ui.h" /*FIXME for ATT_UUIDStru */
#include "bt_utils.h"
#include "at_process_gatt_DTTMode.h"
#include "auto_gatt_client.h"


static gatt_data_transport_t s_cb;

void at_process_gatt_DTTMode_init()
{
	memset(&s_cb, 0, sizeof(gatt_data_transport_t));
}

unsigned char at_process_gatt_DTTMode(char *input_cmd)
{
	if ((strncmp(input_cmd, "KE", strlen("KE")) == 0) && (strlen(input_cmd) == 14)) { // KE(2) + bd(12)
		struct ATT_UUIDStru out;
		bt_uuid_t uuid;
		char *p = input_cmd + strlen("KE");

		s_cb.is_DTTMode = 1;
		s_cb.address = parse_address(p);
		p += 12;
		s_cb.uuid16 = random() & 0xFFFF;
		ATT_UUID_SetU2(&out, s_cb.uuid16);
		memcpy(&uuid.uu, &out.u, 16);

		app_gatt_client_register_client(uuid);

		return 0;
	}

	if (s_cb.is_DTTMode) {
		if ((strncmp(input_cmd, "KF", strlen("KF")) == 0) && (strlen(input_cmd) == 2)) {
			app_gatt_client_disconnect(s_cb.client_if, s_cb.address, s_cb.conn_id);

			return 0;
		} else if ((strncmp(input_cmd, "KC", strlen("KC")) == 0) && (strlen(input_cmd) == 4)) { // KC(2) + is_start(2)
			UINT8 is_start;
			char *p = input_cmd + strlen("KC");

			is_start = parse_byte(p);
			p += 2;
			app_gatt_client_scan(is_start);

			return 0;
		}
	}

	return 1;
}

gatt_data_transport_t *get_gatt_DTTMode_data()
{
	return &s_cb;
}

