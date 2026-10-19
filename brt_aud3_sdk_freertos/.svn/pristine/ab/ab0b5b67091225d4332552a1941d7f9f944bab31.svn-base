#ifndef BT_SNOOP_H__
#define BT_SNOOP_H__
#include "bluedroid/bluetooth.h"
#include "autoconf_bt.h"

//#define BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD

#if 0
void btsnoop_init(void);
void btsnoop_open(char *p_path);
void btsnoop_close(void);
void btsnoop_cleanup(void);
void btsnoop_capture(unsigned char *p_buf, uint32_t len, uint8_t is_rcvd);
#else
void btsnoop_init(void){}
void btsnoop_open(char *p_path){}
void btsnoop_close(void){}
void btsnoop_cleanup(void){}
void btsnoop_capture(unsigned char *p_buf, uint32_t len, uint8_t is_rcvd){}
#endif

#ifdef BTSNOOP_CAPTURE_IN_PERIODIC_PTHREAD
void btsnoop_capture_loop(void);
#endif

#ifdef CONFIG_OPEN_FILTER_DATA
void brt_debug_save_avdtp_cid(uint16_t cid, long stream_handle);
void brt_debug_clean_avdtp_cid(long stream_handle);
void brt_debug_cleanup_all_avdtp();
int brt_debug_check_avdtp_media(const uint8_t *ori_p);

int brt_debug_check_avdtp_src_media(const uint8_t *ori_p);

void brt_debug_save_pbap_tl_hdl(uint16_t tl_handle, uint8_t over_l2cap);
void brt_debug_clean_pbap_tl_hdl();
int brt_debug_check_pbap_tl_hdl(const uint8_t *p);
#endif

#endif /*BT_SNOOP_H__*/
