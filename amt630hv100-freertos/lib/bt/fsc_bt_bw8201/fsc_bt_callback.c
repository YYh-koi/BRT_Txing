#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "bt_api.h"
#include "fscbt_interface.h"
#include "fsc_bw8202.h"
#include "fsc_bw8202_cmd.h"
#include "fsc_bt_callback.h"

extern void carlink_set_bt_mac(const char *mac, int mac_len);
extern char * bw8202_bd_addr_to_str(const BTAddr_t *addr);

void BT_StatusCBK(DashBTEvent_t e,void *respond)
{
#if 1
	static uint16 pb_print_count = 0;
	char buf[UART_BT_MSG_MAX_LEN] = {0};
	char **p_image = NULL;
	unsigned int p_len;
	switch(e)
	{
		case FSC_EVENT_DEV_PWR_IND: {
			FSC_EVENT_PWR_STATE_IND_T *t_pwr = (FSC_EVENT_PWR_STATE_IND_T *)respond;
			printf("BT power=%d\n", t_pwr->pwrstate);
			break;
		}
		case FSC_EVENT_VERSION_IND: {
			FSC_EVENT_VERSION_IND_T *t_ver = (FSC_EVENT_VERSION_IND_T *)respond;
			printf("Read version, mode type[%s], mode version[%d],birthday[%s]\n", t_ver->mode_type, 
					t_ver->mode_version, t_ver->mode_birthday);			
			break;
		}
		case FSC_EVENT_BT_NAME_IND: {
			FSC_EVENT_BT_NAME_IND_T *t_name = (FSC_EVENT_BT_NAME_IND_T *)respond;
			printf("BT name=%s\n", t_name->name);
			break;
		}
		case FSC_EVENT_BT_LENAME_IND: {
			FSC_EVENT_BT_NAME_IND_T *t_lename = (FSC_EVENT_BT_NAME_IND_T *)respond;
			printf("BT LE name=%s\n", t_lename->name);
			break;
		}
		case FSC_EVENT_BT_ADDR_IND: {
			FSC_EVENT_BT_ADDR_IND_T *t_addr = (FSC_EVENT_BT_ADDR_IND_T *)respond;
//                        printf("---------------------FSC_EVENT_BT_addr_ZXB--------------------------------------\r\n");
                        //strncpy(g_bt_mac, (const char *)bw8202_bd_addr_to_str(&t_addr->addr), 13);
                        //g_bt_mac_ready = true;
			printf("BT addr=%s\n", bw8202_bd_addr_to_str(&t_addr->addr));
                        carlink_set_bt_mac((const char *)bw8202_bd_addr_to_str(&t_addr->addr), 13);
			break;
		}
		case FSC_EVENT_BT_LEADDR_IND: {
			FSC_EVENT_BT_ADDR_IND_T *t_leaddr = (FSC_EVENT_BT_ADDR_IND_T *)respond;
			printf("BT LE addr=%s\n", bw8202_bd_addr_to_str(&t_leaddr->addr));
			break;
		}
		case FSC_EVENT_BAUD_IND: {
			FSC_EVENT_BAUD_IND_T *t_baud = (FSC_EVENT_BAUD_IND_T *)respond;
			printf("Current baudrate=%d\n", t_baud->cur_baud);
		}
		case FSC_EVENT_SPKVOL_IND: {
			FSC_EVENT_SPKVOL_IND_T *t_vol = (FSC_EVENT_SPKVOL_IND_T *)respond;
			printf("A2dpvol=%d, Hfpvol=%d\n", t_vol->a2dp_volume, t_vol->hfp_volume);
			break;
		}
		case FSC_EVENT_DEV_SSP_IND: {
			FSC_EVENT_DEV_SSP_IND_T *t_ssp = (FSC_EVENT_DEV_SSP_IND_T *)respond;
			printf("BT SSP=%d\n", t_ssp->ssp_state);
			break;
		}
		case FSC_EVENT_BT_PIN_IND: {
			FSC_EVENT_BT_PIN_IND_T *t_pin = (FSC_EVENT_BT_PIN_IND_T *)respond;
			printf("BT Pin=%s\n", t_pin->pin_word);
			break;
		}
		case FSC_EVENT_BT_PAIR_IND: {
			FSC_EVENT_BT_PAIR_IND_T *t_pair = (FSC_EVENT_BT_PAIR_IND_T *)respond;
			printf("BT Pair=%d\n", t_pair->pair_action);
			break;
		}
		case FSC_EVENT_BT_PAGE_IND: {
			FSC_EVENT_BT_PAGE_IND_T *t_page = (FSC_EVENT_BT_PAGE_IND_T *)respond;
			printf("BT Page=%d\n", t_page->page_action);
		}
		case FSC_EVENT_DEV_STATE_IND: {
			FSC_EVENT_DEV_STATE_IND_T *t_dev = (FSC_EVENT_DEV_STATE_IND_T *)respond;
			printf("DEV Stat=%d\n", t_dev->devstate);
			break;
		}
		case FSC_EVENT_INQUIRY_IND: {
			FSC_EVENT_INQUIRY_IND_T *t_inquiry = (FSC_EVENT_INQUIRY_IND_T *)respond;
			if(t_inquiry->scan_end)
				printf("Scan is completed!\n");
			else
				printf("BT Scan num=%d, rssi=%d, type=%d, addr=%s, name=%s, cod=%06X\n", t_inquiry->order, t_inquiry->rssi,
						t_inquiry->dev_type, bw8202_bd_addr_to_str(&t_inquiry->addr), t_inquiry->name, t_inquiry->dev_cod);
			break;
		}
		case FSC_EVENT_PLIST_IND: {
			FSC_EVENT_PLIST_IND_T *t_record = (FSC_EVENT_PLIST_IND_T *)respond;
			if(t_record->list_end)
				printf("list is in end!\n");
			else
				printf("BT paired dev list num=%d, profile=%d, addr=%s, name=%s\n", t_record->order, t_record->profile, 
						bw8202_bd_addr_to_str(&t_record->addr), t_record->name);
			break;
		}
		case FSC_EVENT_PAIRED_RESULT_IND: {
			FSC_EVENT_PAIRED_RESULT_IND_T *t_result = (FSC_EVENT_PAIRED_RESULT_IND_T *)respond;
			printf("BT Paired Result=%d, addr=%s\n", t_result->paired_cause, t_result->addr.address);
			break;
		}
		case FSC_EVENT_DEV_FOCUS_IND: {
			FSC_EVENT_DEV_FOCUS_IND_T *t_focus = (FSC_EVENT_DEV_FOCUS_IND_T *)respond;
			printf("primary addr=%s, ", bw8202_bd_addr_to_str(&t_focus->primary_addr));
			printf("secondary addr=%s\n", bw8202_bd_addr_to_str(&t_focus->secondary_addr));
			break;
		}
		case FSC_EVENT_AUD_ROUTE_IND: {
			FSC_EVENT_AUD_ROUTE_IND_T *t_route = (FSC_EVENT_AUD_ROUTE_IND_T *)respond;
			printf("AUD Route=%d\n", t_route->route);
			break;
		}
		case FSC_EVENT_DEV_ADV_IND: {
			FSC_EVENT_DEV_ADV_IND_T *t_adv = (FSC_EVENT_DEV_ADV_IND_T *)respond;
			printf("ADV name=%s, len=%d, Data=%s\n", t_adv->adv_name, t_adv->adv_len, t_adv->adv);
			break;
		}
		case FSC_EVENT_DEV_COVERART_IND: {
			FSC_EVENT_DEV_COVERART_IND_T *t_coverart = (FSC_EVENT_DEV_COVERART_IND_T *)respond;
			printf("COVERART=%d\r\n", t_coverart->coverart);
			break;
		}
		case FSC_EVENT_COVER_DATA_IND: {
			//char **p_image = NULL;
			//unsigned int p_len;
			FSC_EVENT_COVER_DATA_IND_T *t_cover_data = (FSC_EVENT_COVER_DATA_IND_T *)respond;
			fscbt_get_coverart_data(p_image, &p_len);
			if(p_len == t_cover_data->data_len)
			{
				printf("Binary data for image photo.\r\n");
				//Cover data handle by user   (*p_image)[0 - p_len-1]
			}
			else
			{
				printf("Cover data recv error!\r\n");
			}
			if(*p_image)
				vPortFree(*p_image);	// must be free *p_image 
			else
				printf("Cover data malloc error!\r\n");
			break;
		}
		case FSC_EVENT_BIP_STATE_IND: {
			FSC_EVENT_BIP_STATE_IND_T *t_bip_state = (FSC_EVENT_BIP_STATE_IND_T *)respond;
			switch(t_bip_state->bip_state)
			{
				case BIP_DOWNLOADING:
					printf("Cover data download start!\n");
					break;
				case BIP_CONNECTED:
					printf("Cover connected or cover data download end!\n");
					break;
				default:
					printf("BIP status=%d\n", t_bip_state->bip_state);
					break;
			}
			break;
		}
		case FSC_EVENT_HFP_STATE_IND: {
			FSC_EVENT_HFP_STATE_IND_T *t_hfpstate = (FSC_EVENT_HFP_STATE_IND_T *)respond;
			printf("HFP phone state=%d, HFP primary state=%d, HFP secondary state=%d\n", t_hfpstate->phone_state, 
																t_hfpstate->primary_state, t_hfpstate->secondary_state);
			break;
		}
		case FSC_EVENT_HFP_CID_IND: {
			FSC_EVENT_HFP_CID_IND_T *t_call = (FSC_EVENT_HFP_CID_IND_T *)respond;
			printf("Phone Calling=%s\n", t_call->call_number);
			break;
		}
		case FSC_EVENT_HFP_CIE_IND: {
			FSC_EVENT_HFP_CIE_IND_T *t_call_name = (FSC_EVENT_HFP_CIE_IND_T *)respond;
			printf("Phone Call name=%s\n", t_call_name->call_name);
			break;
		}
		case FSC_EVENT_HFP_PHONE_INFO_IND: {
			FSC_EVENT_HFP_INFO_IND_T *t_hfp_phone_info = (FSC_EVENT_HFP_INFO_IND_T *)respond;
			printf("Remote hfp phone dev addr=%s, name=%s\n", bw8202_bd_addr_to_str((&t_hfp_phone_info->remote_addr)),
															 t_hfp_phone_info->remote_name);
			break;
		}
		case FSC_EVENT_HFP_PRIMARY_INFO_IND: {
			FSC_EVENT_HFP_INFO_IND_T *t_hfp_primary_info = (FSC_EVENT_HFP_INFO_IND_T *)respond;
			printf("Remote hfp primary dev addr=%s, name=%s\n", bw8202_bd_addr_to_str((&t_hfp_primary_info->remote_addr)),
															   t_hfp_primary_info->remote_name);
			break;
		}
		case FSC_EVENT_HFP_SECONDARY_INFO_IND: {
			FSC_EVENT_HFP_INFO_IND_T *t_hfp_secondary_info = (FSC_EVENT_HFP_INFO_IND_T *)respond;
			printf("Remote hfp secondary dev addr=%s, name=%s\n", bw8202_bd_addr_to_str((&t_hfp_secondary_info->remote_addr)),
																 t_hfp_secondary_info->remote_name);
			break;
		}
		case FSC_EVENT_HFP_AUDIO_IND: {
			FSC_EVENT_HFP_AUDIO_IND_T *t_hfp_audio = (FSC_EVENT_HFP_AUDIO_IND_T *)respond;
			printf("HFP Audio phone=%d, primary=%d, secondary=%d\n", t_hfp_audio->phone_audio,t_hfp_audio->primary_audio,
																	t_hfp_audio->secondary_audio);
			break;
		}
		case FSC_EVENT_HFP_SIG_IND: {
			FSC_EVENT_HFP_SIG_IND_T *t_hfp_sig = (FSC_EVENT_HFP_SIG_IND_T *)respond;
			printf("Remote dev signal=%d\n", t_hfp_sig->sig);
			break;
		}
		case FSC_EVENT_HFP_ROAM_IND: {
			FSC_EVENT_HFP_ROAM_IND_T *t_hfp_roam = (FSC_EVENT_HFP_ROAM_IND_T *)respond;
			printf("Remote dev roam=%d\n", t_hfp_roam->roam);
			break;
		}
		case FSC_EVENT_HFP_BATT_IND: {
			FSC_EVENT_HFP_BATT_IND_T *t_hfp_batt = (FSC_EVENT_HFP_BATT_IND_T *)respond;
			printf("HFP phone batt=%d, primary batt=%d, secondary batt=%d\n", t_hfp_batt->phone_batt, 
																t_hfp_batt->primary_batt, t_hfp_batt->secondary_batt);
			break;
		}
		case FSC_EVENT_HFP_MANU_IND: {
			FSC_EVENT_HFP_MANU_IND_T *t_hfp_manu = (FSC_EVENT_HFP_MANU_IND_T *)respond;
			printf("Remote Manu=%s\n", t_hfp_manu->manu_name);
			break;
		}
		case FSC_EVENT_REMOTE_NUM_IND: {
			FSC_EVENT_REMOTE_NUM_IND_T *t_remote_num = (FSC_EVENT_REMOTE_NUM_IND_T *)respond;
			printf("Remote phone number=%s\n", t_remote_num->remote_num);
			break;
		}
		case FSC_EVENT_REMOTE_NET_IND: {
			FSC_EVENT_REMOTE_NET_IND_T *t_remote_net = (FSC_EVENT_REMOTE_NET_IND_T *)respond;
			printf("Remote net=%s\n", t_remote_net->remote_net);
			break;
		}
		case FSC_EVENT_HFP_VR_IND: {
			FSC_EVENT_HFP_VR_IND_T *t_voice_remote = (FSC_EVENT_HFP_VR_IND_T *)respond;
			printf("Phone Voice wakeup=%d\n", t_voice_remote->voice_remote);
			break;
		}
		case FSC_EVENT_A2DP_STATE_IND: {
			FSC_EVENT_A2DP_STATE_IND_T *t_a2dpstate = (FSC_EVENT_A2DP_STATE_IND_T *)respond;
			printf("A2dp phone state=%d, primary state=%d, secondary state=%d\n", t_a2dpstate->phone_state, 
																t_a2dpstate->primary_state, t_a2dpstate->secondary_state);
			break;
		}
		case FSC_EVENT_A2DP_MUTE_IND: {
			FSC_EVENT_A2DP_MUTE_IND_T *t_a2dpmute = (FSC_EVENT_A2DP_MUTE_IND_T *)respond;
			printf("A2dp mute=%d\n", t_a2dpmute->mute_state);
			break;
		}
		case FSC_EVENT_A2DP_PHONE_INFO_IND: {
			FSC_EVENT_A2DP_INFO_IND_T *t_a2dp_phone_info = (FSC_EVENT_A2DP_INFO_IND_T *)respond;
			printf("Remote a2dp dev phone addr=%s, name=%s\n", bw8202_bd_addr_to_str((&t_a2dp_phone_info->remote_addr)),
															  t_a2dp_phone_info->remote_name);
			break;
		}
		case FSC_EVENT_A2DP_PRIMARY_INFO_IND: {
			FSC_EVENT_A2DP_INFO_IND_T *t_a2dp_primary_info = (FSC_EVENT_A2DP_INFO_IND_T *)respond;
			printf("Remote a2dp dev primary addr=%s, name=%s\n", bw8202_bd_addr_to_str((&t_a2dp_primary_info->remote_addr)),
																t_a2dp_primary_info->remote_name);
			break;
		}
		case FSC_EVENT_A2DP_SECONDARY_INFO_IND: {
			FSC_EVENT_A2DP_INFO_IND_T *t_a2dp_secondary_info = (FSC_EVENT_A2DP_INFO_IND_T *)respond;
			printf("Remote a2dp dev secondary addr=%s, name=%s\n", bw8202_bd_addr_to_str((&t_a2dp_secondary_info->remote_addr)),
																  t_a2dp_secondary_info->remote_name);
			break;
		}
		case FSC_EVENT_AVRCP_STATE_IND: {
			FSC_EVENT_AVRCP_STATE_IND_T *t_avrcpstus = (FSC_EVENT_AVRCP_STATE_IND_T *)respond;
			printf("HPhone avrcp status=%d, HP1 avrcp status=%d, HP2 avrcp status=%d\n", t_avrcpstus->phone_state, 
																t_avrcpstus->primary_state, t_avrcpstus->secondary_state);
			break;
		}
		case FSC_EVENT_AVRCP_CFG_IND: {
			FSC_EVENT_AVRCP_CFG_IND_T *t_avrcpcfg = (FSC_EVENT_AVRCP_CFG_IND_T *)respond;
			printf("Avrcp cfg = %d\n", t_avrcpcfg->cfg);
			break;
		}
		case FSC_EVENT_PLAY_STATE_IND: {
			FSC_EVENT_PLAY_STATE_IND_T *t_player_state = (FSC_EVENT_PLAY_STATE_IND_T *)respond;
			printf("Avrcp player status=%d\n", t_player_state->play_state);
			break;
		}
		case FSC_EVENT_TRACK_STATE_IND: {
			FSC_EVENT_TRACK_STATE_IND_T *t_track_info = (FSC_EVENT_TRACK_STATE_IND_T *)respond;
			printf("Player status=%d, track current time=%d, total time=%d\n", t_track_info->play_state, 
																t_track_info->cursenconds, t_track_info->totaltime);
			break;
		}
		case FSC_EVENT_TRACK_ID3_IND: {
			FSC_EVENT_TRACK_ID3_IND_T *t_track_id3 = (FSC_EVENT_TRACK_ID3_IND_T *)respond;
			printf("Track title=%s, track artist=%s, track album=%s\n", t_track_id3->id3_title, 
																t_track_id3->id3_artist, t_track_id3->id3_album);
			break;
		}
		case FSC_EVENT_AVRCP_CTPLAY_IND:
			printf("CT media play\r\n");
			break;
		case FSC_EVENT_AVRCP_CTPAUSE_IND:
			printf("CT media pause\r\n");
			break;
		case FSC_EVENT_AVRCP_CTSTOP_IND:
			printf("CT media stop\r\n");
			break;
		case FSC_EVENT_AVRCP_CTFWD_IND:
			printf("CT media forward\r\n");
			break;
		case FSC_EVENT_AVRCP_CTBACK_IND:
			printf("CT media backward\r\n");
			break;
		case FSC_EVENT_PBAP_STATE_IND: {
			FSC_EVENT_PBAP_STATE_IND_T *t_pbap_state = (FSC_EVENT_PBAP_STATE_IND_T *)respond;
			printf("PBAP status=%d\n", t_pbap_state->pbap_state);
			break;
		}
		case FSC_EVENT_PBAP_CNT_IND: {
			FSC_EVENT_PBAP_CNT_IND_T *t_pbap_cnt = (FSC_EVENT_PBAP_CNT_IND_T *)respond;
			printf("PBAP cnt=%d\n", t_pbap_cnt->pbap_count);
			break;
		}
		case FSC_EVENT_PBAP_DATA_IND: {
			FSC_EVENT_PBAP_DATA_IND_T *t_pbap_book = (FSC_EVENT_PBAP_DATA_IND_T *)respond;
			if(t_pbap_book->type== PBAP_DATA_TYPE_END)
			{
				pb_print_count = 0;
				printf("PBAP download end!\n");
			}
			else
			{
				pb_print_count++;
				printf("PBAP count=%d, type=%d, name=%s, num=%s\n", pb_print_count, t_pbap_book->type,
																   t_pbap_book->name,t_pbap_book->num);
			}
			break;
		}
		case FSC_EVENT_SPP_STATE_IND: {
			FSC_EVENT_SPP_STATE_IND_T *t_spp_state = (FSC_EVENT_SPP_STATE_IND_T *)respond;
			printf("SPP status=%d\n", t_spp_state->spp_state);
			break;
		}
		case FSC_EVENT_SPP_DATA_IND: {
			//char **p_image = NULL;
			//unsigned int p_len;
			FSC_EVENT_SPP_DATA_IND_T *t_spp_recv = (FSC_EVENT_SPP_DATA_IND_T *)respond;
			fscbt_get_coverart_data(p_image, &p_len);
			if(p_len == t_spp_recv->data_len)
			{
				memcpy(buf, *p_image, p_len);
				printf("Spp recv = %s\r\n", buf);
			}
			else
			{
				printf("Spp recv error!\r\n");
			}
			if(*p_image)
				vPortFree(*p_image);	// must be free *p_image 
			else
				printf("Spp data malloc error!\r\n");
			break;
		}
		case FSC_EVENT_GATT_STATE_IND: {
			FSC_EVENT_GATT_STATE_IND_T *t_gatt_state = (FSC_EVENT_GATT_STATE_IND_T *)respond;
			printf("GATT status=%d\n", t_gatt_state->gatt_state);
			break;
		}
		case FSC_EVENT_GATT_DATA_IND: {
			//char **p_image = NULL;
			//unsigned int p_len;
			FSC_EVENT_GATT_DATA_IND_T *t_gatt_recv = (FSC_EVENT_GATT_DATA_IND_T *)respond;
			fscbt_get_coverart_data(p_image, &p_len);
			if(p_len == t_gatt_recv->data_len)
			{
				memcpy(buf, *p_image, p_len);
				printf("Gatt recv = %s\r\n", buf);
			}
			else
			{
				printf("Gatt recv error!\r\n");
			}
			if(*p_image)
				vPortFree(*p_image);	// must be free *p_image 
			else
				printf("Gatt data malloc error!\r\n");
			break;
		}
		case FSC_EVENT_GATT_UUID_IND: {
			FSC_EVENT_GATT_UUID_IND_T *t_gatt_uuid = (FSC_EVENT_GATT_UUID_IND_T *)respond;
			printf("GATT UUID=%d\n", t_gatt_uuid->uuid_state);
			break;
		}
		default:
			break;
	}
#endif

}

