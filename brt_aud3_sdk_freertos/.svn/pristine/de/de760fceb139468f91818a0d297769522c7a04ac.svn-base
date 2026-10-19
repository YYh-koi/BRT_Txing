#include "btdatatype.h"
#include "autoconf_bt.h"
#include "bta_bip.h"
#include "bip_ui.h"
#include "sdap_uimacro.h"
#include "sdap_uistru.h"
#include "sdap_uifunc.h"
#include "expat.h"
#include <unistd.h>
#include "brt_trace.h"
#include "auto_avrcp.h"

#define TAG "bse_bip"

#ifdef CONFIG_BIP_CLIENT


typedef union {
	void				*param;
	UINT8				*bd;
	UINT16				*result;
	tBIP_AuthenIndStru		*auth_par;
	tBIP_ResponderAttributesStru	*rsp_attr;
	tBIP_ConnectCompleteStru	*conn_cmp;
	tBIP_DisconnectCompleteStru	*disc_cmp;
	tBIP_ObjectDataStru		*obj_data;
	tBIP_GetContinueFlag		*next_op;

	BtUINT64Stru			*obj_length;
	tBIP_ImageListItemStru		*img_item;
	tBIP_ImagePropertyStru		*img_prop;
	tBIP_GetImagesListCompleteStru	*get_img_list_cmp;
} tAppClntEvtParameters;

static UINT8 s_bip_data_done = FALSE;
tBIP_Encoding	s_cover_art_encoding = 0;
bip_user_date_t s_bip_user_data = {0};
static const char *s_img_types[] = {"JPEG", "GIF", "WBMP", "PNG", "JPEG2K", "BMP", "USR-XXX"};

static void hlp_bip_connect_cbk(void *handle, void *context, tBIP_Event_Type event, void *param);
static void hlp_cleanup_image_buffer(bip_user_date_t *user_data);
static void hlp_BIP_Image_Cbk(void *handle, void *context, tBIP_Event_Type event, void *param);
static void hlp_BIP_Properties_Cbk(void *handle, void *context, tBIP_Event_Type event, void *param);
#endif

extern void event_avrcp_indication(UINT8 event, void *param);
static bt_coverart_data_callback s_bip_cbk = {0};
static uint32_t s_bip_psm = 0;
void bta_bip_open(uint8_t remote_bda[6])
{
  bse_log_api(TAG, "%s",__func__);
  event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_READY);
  if (s_bip_user_data.bip_connected == FALSE) {
    //BIP_ClientStartReq(NULL, remote_bda, BIP_FEATURE_AVRCP_COVER_ART, s_bip_psm | OBEX_TL_L2CAP, hlp_bip_connect_cbk);  lkz debug
  }

}

void bta_bip_init(char* pCoverArtPath)
{
    event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_INIT);
}

void bta_bip_close(void)
{

}

void bta_bip_update_cover_art(uint8_t *cover_name)
{
    (void)cover_name;
    s_bip_data_done = FALSE;
	
    //BIP_GetImagePropertiesReq(s_bip_user_data.bip_con_handle, 0, s_bip_user_data.image_req.handle, hlp_BIP_Properties_Cbk);
    BIP_GetLinkedThumbnailReq(s_bip_user_data.bip_con_handle, 0, s_bip_user_data.image_req.handle, hlp_BIP_Image_Cbk); //thrunbnail base 200*200 JPEG
    event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_DOWNLOADING);
}

bool bta_is_cover_art_done(void)
{
  return s_bip_data_done;
}

bool bta_is_cover_art_connected(void)
{
  bse_log_api(TAG, "%s is connected %d", __func__, s_bip_user_data.bip_connected);
  return s_bip_user_data.bip_connected;
}

void bta_bip_register_coverArt_callback(bt_coverart_data_callback cb)
{
  bse_log_api(TAG, "%s",__func__);
  s_bip_cbk = cb;
}

void set_bip_cover_handle(tImageHandle cover_handle)
{
  s_bip_user_data.image_req.handle = cover_handle;
}

void set_bip_l2cap_psm(uint16_t psm)
{
  bse_log_api(TAG, "%s psm = %hu",__func__, psm);
  s_bip_psm=psm;
}

#ifdef CONFIG_BIP_CLIENT

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Create a copy of image property. The member of pointer type shall also be copied after
this function returns.
------------------------------------------------------------------------------------*/

static void *hlp_duplicate_property(struct ListStru *list, tBIP_ImagePropertyStru *src_prop)
{
	tBIP_ImagePropertyStru *prop_new = List_NodeNew(sizeof(tBIP_ImagePropertyStru));
	memcpy(prop_new, src_prop, sizeof(tBIP_ImagePropertyStru));
	List_AddTail(list, prop_new);
	return &prop_new->property;
}

static void hlp_duplicate_string(UCHAR **str)
{
	UCHAR *str_new;
	if (*str != NULL) {
		str_new = NEW(strlen((const char *)*str));
		strcpy((char *)str_new, (const char *)*str);
		*str = str_new;
	}
}

static void hlp_display_pixels(UINT16 *pixels)
{
	char str[64];
	memset(str, 0, 64);
	if (pixels[0] || pixels[1] || (pixels[2] && pixels[3])) {
		strcat(str, "\tPixel: ");
		if (pixels[0]) {
			sprintf(str + strlen(str), "%d*", pixels[0]);
		} else {
			strcat(str, "**");
		}
		if (pixels[1]) {
			sprintf(str + strlen(str), "%d", pixels[1]);
		} else {
			strcat(str, "*");
		}
		if (pixels[2] && pixels[3]) {
			sprintf(str + strlen(str), "-%d*%d", pixels[2], pixels[3]);
		}
	}

	bse_log_api(TAG, "%s", str);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Display the pixels: w1, h1, w2, h2
------------------------------------------------------------------------------------*/
static void hlp_display_size(UCHAR *prefix, BtUINT64Stru *size64)
{
	if (size64->high32 || size64->low32) {
		char size_str[24];

		BTUtil_U64toA(size64->high32, size64->low32, size_str, 24);
		bse_log_api(TAG, "%s: %s", prefix, size_str);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Display the transformation
------------------------------------------------------------------------------------*/
static void hlp_display_transform(UINT8 transformation)
{
	UCHAR comma[3] = {0, 0, 0};
	char str[64];
	memset(str, 0, 64);
	if (transformation & (BIP_TRANSFORMATION_STRETCH|BIP_TRANSFORMATION_CROP|BIP_TRANSFORMATION_FILL)) {
		strcat(str, "\tTransformation: ");
		if (transformation & BIP_TRANSFORMATION_STRETCH) {
			strcat(str, "stretch");
			comma[0] = ',';
			comma[1] = ' ';
		}
		if (transformation & BIP_TRANSFORMATION_CROP) {
			sprintf(str + strlen(str), "%scrop", comma);
			comma[0] = ',';
			comma[1] = ' ';
		}
		if (transformation & BIP_TRANSFORMATION_FILL) {
			sprintf(str + strlen(str), "%sfill", comma);
		}
		bse_log_api(TAG, "%s", str);
	}
}
static void hlp_display_image_properties(struct ListStru *prop_list)
{
	tBIP_NativePropertyStru		*native;
	tBIP_VariantPropertyStru	*variant;
	tBIP_AttachmentPropertyStru	*attachment;
	tBIP_ImagePropertyStru *prop = prop_list->head;

	bse_log_api(TAG, "*** Native ***********");
	while (prop != NULL) {
		switch (prop->element_type) {
			case BIP_ELEMENT_NATIVE:
				native = &prop->property.native;
				if ((native->encoding != BIP_ENCODING_USER_TAG) || (native->user_tag == NULL)) {
					bse_log_api(TAG, "\tType: %s", s_img_types[native->encoding]);
          				s_cover_art_encoding = native->encoding;
				} else {
					bse_log_api(TAG, "\tType: %s", native->user_tag);
        				s_cover_art_encoding = native->encoding;
				}
				hlp_display_pixels(&native->pixel_w1);
				hlp_display_size((UCHAR *)"Size", &native->size);
				break;
			case BIP_ELEMENT_VARIANT:
				bse_log_api(TAG, "*** Variant ***********");
				variant = &prop->property.variant;
				if ((variant->encoding != BIP_ENCODING_USER_TAG) || (variant->user_tag == NULL)) {
					bse_log_api(TAG, "\tType: %s", s_img_types[variant->encoding]);
				} else {
					bse_log_api(TAG, "\tType: %s", variant->user_tag);
				}
				hlp_display_pixels(&variant->pixel_w1);
				hlp_display_size((UCHAR *)"Max size", &variant->max_size);
				hlp_display_transform(variant->transformation);
				break;
			case BIP_ELEMENT_ATTACHMENT:
				bse_log_api(TAG, "*** Attachment *******");
				attachment = &prop->property.attachment;
				if (attachment->name != NULL) {
					bse_log_api(TAG, "\tName: %s", attachment->name);
				}
				if (attachment->content_type != NULL) {
					bse_log_api(TAG, "\tContent type: %s", attachment->content_type);
				}
				if (attachment->charset != NULL) {
					bse_log_api(TAG, "\tCharset: %s", attachment->charset);
				}
				if (attachment->created != NULL) {
					bse_log_api(TAG, "\tCreated: %s", attachment->created);
				}
				if (attachment->modified != NULL) {
					bse_log_api(TAG, "\tModified: %s", attachment->modified);
				}
				hlp_display_size((UCHAR *)"Size", &attachment->size);
				break;
			default:
				break;
		}
		prop = LNEXT(prop);
	}
	bse_log_api(TAG, "++++++++++++++++++++++++++++++++++++++");
}

static void hlp_free_image_properties(struct ListStru *prop_list)
{
	tBIP_ImagePropertyStru *prop;

	while ((prop = List_RemoveHead(prop_list)) != NULL) {
		switch (prop->element_type) {
		case BIP_ELEMENT_NATIVE:
		case BIP_ELEMENT_VARIANT:
			if (prop->property.native.user_tag != NULL) {
				FREE(prop->property.native.user_tag);
			}
			break;
		case BIP_ELEMENT_ATTACHMENT:
			if (prop->property.attachment.content_type != NULL) {
				FREE(prop->property.attachment.content_type);
			}
			if (prop->property.attachment.charset != NULL) {
				FREE(prop->property.attachment.charset);
			}
			if (prop->property.attachment.name != NULL) {
				FREE(prop->property.attachment.name);
			}
			if (prop->property.attachment.created != NULL) {
				FREE(prop->property.attachment.created);
			}
			if (prop->property.attachment.modified != NULL) {
				FREE(prop->property.attachment.modified);
			}
			break;
		default:
			break;
		}
		LFREE(prop);
	}

	prop_list->head = prop_list->tail = NULL;
	
}

static int hlp_calculate_image_descriptor(struct ListStru *prop_list, tBIP_ImageDescriptorStru *src_descriptor, tBIP_ImageDescriptorStru *dest_descriptor)
{
	tBIP_NativePropertyStru		*native;
	tBIP_VariantPropertyStru	*variant;
	tBIP_ImagePropertyStru *prop = prop_list->head;
	int same_encoding = 0;

	while (prop != NULL) {
		switch (prop->element_type) {
			case BIP_ELEMENT_NATIVE:
				native = &prop->property.native;
				if ((native->encoding != BIP_ENCODING_USER_TAG) || (native->user_tag == NULL)) {
					if (native->encoding != src_descriptor->encoding) {
						bse_log_api(TAG, "%s request encoding is %d but native is %d.", __func__, src_descriptor->encoding, native->encoding);
					} else {
						same_encoding = 1;
					}
				}

				if (src_descriptor->pixel_w1 <= native->pixel_w1 &&
						src_descriptor->pixel_h1 <= native->pixel_h1) {

					if(src_descriptor->pixel_h1 == 0 && src_descriptor->pixel_w1 == 0) {
						memcpy(dest_descriptor, native, sizeof(tBIP_ImageDescriptorStru));
					} else {
						memcpy(dest_descriptor, src_descriptor, sizeof(tBIP_ImageDescriptorStru));
					}
					if (same_encoding) {
						return 0;
					}
				}
				break;
			case BIP_ELEMENT_VARIANT:
				variant = &prop->property.variant;
				if ((variant->encoding != BIP_ENCODING_USER_TAG) || (variant->user_tag == NULL)) {
					if (variant->encoding != src_descriptor->encoding) {
						bse_log_api(TAG, "%s request encoding is %d but variant is %d.", __func__, src_descriptor->encoding, variant->encoding);
					} else {
						same_encoding = 1;
					}
				}
				if (src_descriptor->pixel_w1 <= variant->pixel_w1 &&
						src_descriptor->pixel_h1 <= variant->pixel_h1) {
					if(src_descriptor->pixel_h1 == 0 && src_descriptor->pixel_w1 == 0) {
						memcpy(dest_descriptor, variant, sizeof(tBIP_ImageDescriptorStru));
					} else {
						memcpy(dest_descriptor, src_descriptor, sizeof(tBIP_ImageDescriptorStru));
					}
					if (same_encoding) {
						return 0;
					}
				}
				break;
			default:
				break;
		}

		prop = LNEXT(prop);
	}

	if (native != NULL) {
		memcpy(dest_descriptor, native, sizeof(tBIP_NativePropertyStru));
	}

	return 0;
}

static void hlp_bip_connect_cbk(void *handle, void *context, tBIP_Event_Type event, void *param)
{
	bt_bdaddr_t address;
	UCHAR name[MAX_DEV_NAME_LEN];
	tBIP_ConnectCompleteStru *connComplStru;
	uint8_t status;

	bse_log_api(TAG, "%s handle=0x%0x, event = 0x%x.", __func__, handle, event);
	switch (event) {
		case BIP_EVT_CLIENT_START_CFM:
			event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_CONNECTING);
			memcpy(s_bip_user_data.bip_bd, param, BD_ADDR_LEN);
			s_bip_user_data.bip_con_handle = handle;
			break;

		case BIP_EVT_CONNECT_COMPLETE:
			connComplStru = param;
			if (connComplStru) {
				memcpy(address.address, s_bip_user_data.bip_bd, BD_ADDR_LEN);
				if (connComplStru->result == ER_BIP_SUCCESS) {
					//status = OBEX_HTTP_OK;
					s_bip_user_data.bip_connected = TRUE;
					event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_CONNECTED);
					bse_log_api(TAG, "Connect to IMGS complete!");
					hlp_avrcp_get_element_attr_req(connComplStru->bd, 1);
					//hlp_tuner_set_bip_descriptor();
					//BIP_GetImagePropertiesReq(s_bip_user_data.bip_con_handle, 0, s_bip_user_data.image_req.handle, hlp_BIP_Properties_Cbk);
				} else {
					//status = OBEX_HTTP_BAD_REQUEST; 
					s_bip_user_data.bip_connected = FALSE;
					s_bip_user_data.bip_con_handle = 0;
					bse_log_api(TAG, "Connect to IMGS failed, reason is %hu!", connComplStru->result);
					event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_READY);
					/*if (bip_reconnect) {
	                    bip_reconnect = 0;
					    bse_log_api(TAG, "Connect to IMGS again!");//For Desay
                        app_avrcp_connect_bip(address, 0);
					}*/
				}
				//memcpy(address.address, s_avrcp_user_data->remote_bd, BD_ADDR_LEN);
				//HAL_CBACK(s_bip_user_data->avrcp_cbks, connect_bip_result_cb, address, status);
			}
			break;

		case BIP_EVT_DISCONNECT_COMPLETE:
			s_bip_user_data.bip_con_handle = 0;
			s_bip_user_data.bip_connected = FALSE;
			break;
		default:
			break;
	}

}
#if 0
static void hlp_BIP_Properties_Cbk(void *handle, void *context, tBIP_Event_Type event, void *param)
{
  bse_log_api(TAG, "%s", __func__);
}
#else
static void hlp_BIP_Properties_Cbk(void *handle, void *context, tBIP_Event_Type event, void *param)
{
	int len;
	tAppClntEvtParameters real_par;
	tBIP_BasicPropertyStru		*basic;
	tBIP_NativePropertyStru		*native;
	tBIP_VariantPropertyStru	*variant;
	tBIP_AttachmentPropertyStru	*attachment;
	tBIP_ImageDescriptorStru 	req_param;
	bse_log_api(TAG, "%s handle=0x%0x, event = 0x%x.", __func__, handle, event);
	real_par.param = param;
	switch (event) {
		case BIP_EVT_IMAGE_PROPERTY_IND:
			if (real_par.img_prop != NULL) {
				bse_log_api(TAG, "%s(%d) element_type:0x%x.", __func__, BIP_EVT_IMAGE_PROPERTY_IND, real_par.img_prop->element_type);
				switch (real_par.img_prop->element_type) {
					case BIP_ELEMENT_IMAGE_PROPERTIES:
						basic = &real_par.img_prop->property.basic;
						if (basic->friendly_name != NULL) {
							bse_log_api(TAG, "%s BIP_ELEMENT_IMAGE_PROPERTIES - friendly_name:%s.", __func__, basic->friendly_name);
						}
						break;
					case BIP_ELEMENT_NATIVE:
						native = hlp_duplicate_property(&s_bip_user_data.image_req.prop_list, real_par.img_prop);
						hlp_duplicate_string(&native->user_tag);
						break;
					case BIP_ELEMENT_VARIANT:
						variant = hlp_duplicate_property(&s_bip_user_data.image_req.prop_list, real_par.img_prop);
						hlp_duplicate_string(&variant->user_tag);
						break;
					case BIP_ELEMENT_ATTACHMENT:
						attachment = hlp_duplicate_property(&s_bip_user_data.image_req.prop_list, real_par.img_prop);
						hlp_duplicate_string(&attachment->charset);
						hlp_duplicate_string(&attachment->content_type);
						hlp_duplicate_string(&attachment->created);
						hlp_duplicate_string(&attachment->modified);
						hlp_duplicate_string(&attachment->name);
						break;
					default:
						break;
				}
			}
			break;
		case BIP_EVT_GET_OBJECT_COMPLETE:
			if (real_par.get_img_list_cmp->result != ER_BIP_SUCCESS) {
				bt_bdaddr_t address;
				memcpy(address.address, s_bip_user_data.bip_bd, BD_ADDR_LEN);
				bse_log_api(TAG, "Failed to get properties of image (%07d) for %04X.", s_bip_user_data.image_req.handle, real_par.get_img_list_cmp->result);
				s_bip_user_data.image_req.state = IMAGE_REQ_IDLE;
				//HAL_CBACK(s_avrcp_user_data->avrcp_cbks, get_image_result_cb, address, BT_STATUS_FAIL);
				event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_CONNECTED);
				break;
			}
			s_bip_user_data.image_req.state = IMAGE_REQ_IMAGE_GET;
			hlp_display_image_properties(&s_bip_user_data.image_req.prop_list);
			hlp_calculate_image_descriptor(&s_bip_user_data.image_req.prop_list, &s_bip_user_data.image_req.descriptor, &req_param);
			req_param.user_tag = NULL; /*FIXME*/
			hlp_free_image_properties(&s_bip_user_data.image_req.prop_list);

			BIP_GetImageReq(s_bip_user_data.bip_con_handle, 0, s_bip_user_data.image_req.handle, &req_param, hlp_BIP_Image_Cbk);
			break;
		default:
			break;
	}
}
#endif

#if 0
static void hlp_BIP_Image_Cbk(void *handle, void *context, tBIP_Event_Type event, void *param)
{
  bse_log_api(TAG, "s\n", __func__);
}
#else
static void hlp_BIP_Image_Cbk(void *handle, void *context, tBIP_Event_Type event, void *param)
{
	tAppClntEvtParameters real_par;
	bse_log_api(TAG, "%s handle=0x%0x, event = 0x%x.", __func__, handle, event);

	real_par.param = param;
	switch (event) {
		case BIP_EVT_OBJECT_SIZE_IND:
			bse_log_api(TAG, "Image size %d", real_par.obj_length->low32);
			break;
		case BIP_EVT_ASK4_GET_REQUEST_IND:
			*real_par.next_op = BIP_OP_CONTINUE;
			BIP_GetContinueReq(handle);
			break;
		case BIP_EVT_OBJECT_DATA_IND:
			if (real_par.obj_data->data != NULL) {
				int size = s_bip_user_data.img_size;
				bse_log_api(TAG, "%s data ind: is_final:%d.length:%d.", __func__, real_par.obj_data->is_final, real_par.obj_data->length);
				if (real_par.obj_data->is_final == BIP_DATA_COMPLETE || real_par.obj_data->is_final == BIP_DATA_PARTIAL) {
					if (real_par.obj_data->length > 0) {
						size += real_par.obj_data->length;
						s_bip_user_data.img_buf = realloc(s_bip_user_data.img_buf, size);
						if (s_bip_user_data.img_buf)  {
							memcpy(s_bip_user_data.img_buf + s_bip_user_data.img_size, real_par.obj_data->data, real_par.obj_data->length);
							s_bip_user_data.img_size = size;
						}

					}
					if (real_par.obj_data->is_final == BIP_DATA_COMPLETE) {
						/*FILE *fp;
						bt_bdaddr_t address;
						mode_t mode;
						memcpy(address.address, s_bip_user_data.bip_bd, BD_ADDR_LEN);
						mode = umask(0);
						if ((fp = fopen((const char *)s_bip_user_data.img_path, "wb")) != NULL) {
							fwrite(s_bip_user_data.img_buf, 1, s_bip_user_data.img_size, fp);
							fclose(fp);
							s_bip_user_data.image_req.state = IMAGE_REQ_IDLE;
							//HAL_CBACK(s_bip_user_data.avrcp_cbks, get_image_result_cb, address, BT_STATUS_SUCCESS);
						}
						umask(mode);*/
            s_bip_data_done = TRUE;
		bse_log_api(TAG, "s_bip_user_data.img_size:%d, s_img_types:%s", s_bip_user_data.img_size, s_img_types[s_cover_art_encoding]);
		
            if (s_bip_cbk) {
                s_bip_cbk(s_bip_user_data.img_buf, s_bip_user_data.img_size, s_img_types[s_cover_art_encoding]);  //callback to applicant
            }
            
						hlp_cleanup_image_buffer(&s_bip_user_data);
					}
				} else {
				}
			}
			break;
		case BIP_EVT_GET_OBJECT_COMPLETE:
			if (real_par.get_img_list_cmp->result != ER_BIP_SUCCESS) {
				bt_bdaddr_t address;
				memcpy(address.address, s_bip_user_data.bip_bd, BD_ADDR_LEN);
				bse_log_api(TAG, "Failed to get image(%07d) for %04X.", s_bip_user_data.image_req.handle, real_par.get_img_list_cmp->result);
				s_bip_user_data.image_req.state = IMAGE_REQ_IDLE;
				//HAL_CBACK(s_bip_user_data.avrcp_cbks, get_image_result_cb, address, BT_STATUS_FAIL);
				event_avrcp_indication(EV_AVRCP_BIP_STATUS, (void *)ST_AVRCP_BIP_CONNECTED);
			}
			bse_log_api(TAG, "Image is saved as %s", s_bip_user_data.img_path);
			s_bip_user_data.image_req.state = IMAGE_REQ_IDLE;
			
			//HAL_CBACK(s_avrcp_user_data->avrcp_cbks, get_image_result_cb, address, BT_STATUS_SUCCESS);
			char coverart_info[32]={0};
			sprintf(coverart_info, "%ld.%s", s_bip_user_data.image_req.handle,  s_img_types[s_cover_art_encoding]);
			event_avrcp_indication(EV_AVRCP_BIP_COVERART_IMAGE, coverart_info);
			hlp_cleanup_image_buffer(&s_bip_user_data);
			break;
		default:
			break;
	}
}
#endif
static void hlp_cleanup_image_buffer(bip_user_date_t *user_data)
{
	if (user_data->img_buf) {
		free(user_data->img_buf);
		user_data->img_buf = NULL;
	}
	user_data->img_size = 0;
}
#endif