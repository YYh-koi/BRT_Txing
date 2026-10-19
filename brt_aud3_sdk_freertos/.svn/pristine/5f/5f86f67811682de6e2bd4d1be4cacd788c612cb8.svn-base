#include "btdatatype.h"
#include <stdlib.h>
#include <string.h>
#include "bip_ui.h"
#include "brtbt_interface.h"

/* State */
enum {
    ST_AVRCP_BIP_INIT,
    ST_AVRCP_BIP_READY,
    ST_AVRCP_BIP_CONNECTING,
    ST_AVRCP_BIP_CONNECTED,
    ST_AVRCP_BIP_DOWNLOADING,
};

#ifdef CONFIG_BIP_CLIENT
#define OBEX_HTTP_OK 0xA0
#define OBEX_HTTP_BAD_REQUEST 0xC0
#define IMG_STORAGE_PATH_MAX_LEN 128
#define IMG_HDL_LEN 16
#define MAX_DEV_NAME_LEN 249

typedef enum {
	IMAGE_REQ_IDLE,
	IMAGE_REQ_PROPERTIES_GET,
	IMAGE_REQ_IMAGE_GET,
} cover_art_request_state_t;

typedef struct tag_get_cover_art_image_request {
	cover_art_request_state_t state;
	tImageHandle handle;
	tBIP_ImageDescriptorStru descriptor;
	struct ListStru prop_list;	/* tBIP_ImagePropertyStru */
} get_cover_art_image_request_t;

typedef struct {
	UINT8 bip_connected;
	UINT8 bip_bd[BD_ADDR_LEN];
	void *bip_con_handle;
	UINT8 img_handle[IMG_HDL_LEN];
	UINT8 img_handle_length;
	UINT16 img_size;
	UINT8 *img_buf;
	short img_path_length;
	unsigned char img_path[IMG_STORAGE_PATH_MAX_LEN];
	get_cover_art_image_request_t image_req;
    UINT8 final_state;
}bip_user_date_t;
#endif

void set_bip_l2cap_psm(uint16_t psm);

void set_bip_cover_handle(tImageHandle cover_handle);

void bta_bip_open(uint8_t remote_bda[6]);

void bta_bip_close(void);

void bta_bip_update_cover_art(uint8_t *cover_name);

//If pCoverArtPath is NULL , default cover path is "A:/cover_art.jpg"
//else if CFG_UPGRADE_LFS_FAT_ON_PARTITION0_AND_1 is opened , path will be CFG_TEMP_DRIVE":/cover_art.jpg"
void bta_bip_init(char* pCoverArtPath);

bool bta_is_cover_art_done(void);

bool bta_is_cover_art_connected(void);

void bta_bip_set_dump_coverArt(bool b_dump, char* pCoverArtPath);

void bta_bip_register_coverArt_callback(bt_coverart_data_callback cb);

