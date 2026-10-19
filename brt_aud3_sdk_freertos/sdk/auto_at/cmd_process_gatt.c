#include "btdatatype.h"

#ifdef CONFIG_GATT
#define LOG_TAG "AT_GATT"
#include "auto_at.h"
#include "at_process_gatt_DTTMode.h"
#include "at_process_gatt_client.h"
#include "at_process_gatt_server.h"

void at_process_gatt_init()
{
	at_process_gatt_DTTMode_init();
}

unsigned char at_process_gatt(char * input_cmd)
{
	unsigned char err;

	/* FIXME Check whether in Data_Transparent_Transmission_Mode. */
	err = at_process_gatt_DTTMode(input_cmd);
	if (!err) return err;

	err = at_process_gatt_client(input_cmd);
	if (!err) return err;

	return at_process_gatt_server(input_cmd);
}

void at_process_gatt_data(UINT8 *data, UINT16 len)
{
	// TODO
}

#endif


