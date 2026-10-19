#ifndef EVT_COMPOSITION_H__
#define EVT_COMPOSITION_H__

#ifndef CONFIG_CARLIFE_CERTIFICATE
#define PABP_CALLOG_SIZE 30
#else 
#define PABP_CALLOG_SIZE 50
#endif

int evt_hfp_disconnected(char *buffer, unsigned short len,UINT8 *bd);
int evt_hfp_connected(char *buffer, unsigned short len, UINT8 *bd);
int evt_hfp_outgoing_call(char *buffer, unsigned short len, char *number);
int evt_hfp_incoming_call(char *buffer, unsigned short len, char *number);
int evt_hfp_call_rejected(char *buffer, unsigned short len);
int evt_hfp_call_accepted(char *buffer, unsigned short len, char *name);
int evt_gap_pair_mode_entered(char *buffer, unsigned short len);
int evt_gap_pair_mode_leaved(char *buffer, unsigned short len, int result);
int evt_gap_remote_dev_found(char *buffer, unsigned short len, const UINT8 *bd, UINT8 status, const char *name);
int evt_discovery_finished(char *buffer, unsigned short len);

int evt_hfp_incoming_call_name(char *buffer, unsigned short len, char *name);
int evt_hfp_current_call_number(char *buffer, unsigned short len, char *number);
int evt_bt_initialized(char *buffer, unsigned short len);
int evt_hfp_connecting(char *buffer, unsigned short len,UINT8 *bd);
int evt_a2dp_paused(char *buffer, unsigned short len);
int evt_a2dp_playing(char *buffer, unsigned short len);
int evt_hfp_sco_established(char *buffer, unsigned short len);
int evt_hfp_sco_released(char *buffer, unsigned short len);
int evt_bt_config_ind(char *buffer, unsigned short len, unsigned char accept_call, unsigned char auto_connect);
int evt_hfp_status_ind(char *buffer, unsigned short len, unsigned char *bd, unsigned char status);
int evt_hf_3_ip_ind(char *buffer, unsigned short len,UINT8 index, UINT8 state,UINT8 number[32] );
int evt_hf_3_it_ind(char *buffer, unsigned short len,UINT8 index, UINT8 state,UINT8 number[32] );
int evt_hf_3_iq_ind(char *buffer, unsigned short len,UINT8 index, UINT8 state,UINT8 number[32] );
int evt_income_ringtone(char *buffer, unsigned short len,UINT8 state);
int evt_hfp_ongoing_call_number_ind(char *buffer, unsigned short len, const char *number);
int evt_hfp_voice_dial_begin(char *buffer, unsigned short len);
int evt_hfp_voice_dial_end(char *buffer, unsigned short len);

int evt_avrcp_nowplay_status_ind(char *buffer, unsigned short len, void *param);
int evt_avrcp_change_ind(char *buffer, unsigned short len, int status);
int evt_avrcp_pos_changed(char *buffer, unsigned short len, int time);

int evt_avrcp_status_ind(char *buffer, unsigned short len, unsigned char *bd, unsigned char status);
int evt_gap_local_name(char *buffer, unsigned short len, const char *name);
int evt_gap_pincode_ind(char *buffer, unsigned short len, const char *pin);
int evt_a2dp_status_ind(char *buffer, unsigned short len, unsigned char *bd, unsigned char status);
int evt_bt_version_ind(char *buffer, unsigned short len, const char *version);
int evt_gap_paired_device_ind(char *buffer, unsigned short len, unsigned short index, UINT8 *bd, const char *name);
int evt_a2dp_connected(char *buffer, unsigned short len,char *bd);
int evt_a2dp_disconnected(char *buffer, unsigned short len,char *bd);
int evt_avrcp_id3_ind(char *buffer, unsigned short len, const char *name, const char *artist, const char *album, const char *time);
int evt_pbap_status_ind(char *buffer, unsigned short len, unsigned char status);
int evt_pbap_contact_ind(char *buffer, unsigned short len, char *param);
int evt_pbap_sync_completed(char *buffer, unsigned short len);
int evt_pbap_supported_repos_ind(char *buffer, unsigned short len, unsigned char repos);



// helper
void evt_composition_init();
int evt_discovery_started(char *buffer, unsigned short len);
int evt_discovery_get_device(int index, UINT8 *bd);
unsigned char *get_connected_device_bd();
char * get_connected_device_name();
int evt_local_scan_mode_ind(char *buffer, unsigned short len,UINT8 mode);
int evt_get_connected_device_bd(char *buffer, unsigned short len, UINT32 handle);
int evt_get_connected_device_name(char *buffer, unsigned short len, UINT32 handle);
int do_get_scan_mode_ex();
#endif // EVT_COMPOSITION_H__

