#ifndef TL_H5_H__
#define TL_H5_H__

#include "bt_vendor_lib.h"
#include "bt_hci_lib.h"

typedef void (*tl_h5_data_ind)(UINT8 *data, UINT32 len);
typedef int (*tl_h5_send_to_hc_cb)(UINT8 *data, UINT32 len);

void tl_h5_init(tl_h5_send_to_hc_cb send_cb, tl_h5_data_ind data_ind);
void tl_h5_done();

void hci_h5_init(void);
void hci_h5_cleanup(void);
uint8_t hci_h5_send_int_cmd(uint16_t opcode, HC_BT_HDR *p_buf, tINT_CMD_CBACK p_cback);
void hci_h5_send_msg(HC_BT_HDR *p_msg);
uint16_t hci_h5_receive_msg(unsigned char *buf, unsigned int len);

#endif /*TL_H5_H__*/
