#include "btdatatype.h"

void ey_set_ap_ssid(UINT8 *uuid_p, UCHAR *str);
void ey_set_ap_passwd(UINT8 *uuid_p, UCHAR *str);
void ey_set_ap_connect_status(UINT8 *uuid_p, UCHAR *str);
void ey_set_ip_addr(UINT8 *uuid_p, UCHAR *str);
void ey_set_p2p_go_data(UINT8 *uuid_p, UCHAR *str);

typedef struct {
	UCHAR wifi_ssid[64];
	UCHAR wifi_passwd[64];
	UCHAR wifi_status[64];
	UCHAR wifi_ip_addr[32];
	UCHAR wifi_go_data[256];
} ey_wifi_info_t;

ey_wifi_info_t ey_wifi_info;