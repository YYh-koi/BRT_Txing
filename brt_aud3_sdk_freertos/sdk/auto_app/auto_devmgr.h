#ifndef _APP_DEV_MGR_H_
#define _APP_DEV_MGR_H_

#define MAX_DEV_NUMBER	4
#define BD_ADDR_LEN		6
#define LINK_KEY_LEN	16

typedef struct {
	UINT8 bd[BD_ADDR_LEN];
	UINT8 link_key[LINK_KEY_LEN];
} DeviceStru;

void app_dev_download_database(void);
void app_dev_upload_database(void);
void app_dev_add_to_database(UINT8 *bd, UINT8 *link_key);
//0xFF:Not Found,other:index of the device in database
UINT8 app_dev_get_link_key(UINT8 *bd,UINT8 *link_key);
UINT8 app_dev_set_link_key(UINT8 *bd,UINT8 *link_key);
UINT8 app_dev_get_address(UINT8 i,UINT8 *bd);


#endif
