#include "btdatatype.h"
#include "tl_h5.h"
#include "bt_hci_lib.h"
#include "sysdep.h"

#define HCI_TLPKT_COMMAND 1
#define HCI_TLPKT_ACLDATA 2
#define HCI_TLPKT_SCODATA 3
#define HCI_TLPKT_EVENT   4

extern void bse_print_va(const char *tag, const char *fmt, ...);

#define BRT_DBG(fmt, args...)  bse_print_va("uartif", " %s: " fmt, __FUNCTION__ , ##args)

bt_hc_callbacks_t *bt_hc_cbacks = NULL;
static tl_h5_send_to_hc_cb tl_send_cb = NULL;
static tl_h5_data_ind s_data_received_cb = NULL;

static int send_hci_data(int msgid, unsigned char *buf, unsigned int len)
{
	if (tl_send_cb) {
		return tl_send_cb(buf, len);
	}

	return 0;
}

static void preload_cb(TRANSAC transac, bt_hc_preload_result_t result)
{
	BRT_DBG("HC preload_cb %d [0:SUCCESS 1:FAIL]", result);

	if (result == BT_HC_PRELOAD_SUCCESS) {
		// TODO
	}
}

static void postload_cb(TRANSAC transac, bt_hc_postload_result_t result)
{
	BRT_DBG("HC postload_cb %d", result);
}

static void lpm_cb(bt_hc_lpm_request_result_t result)
{
}

static void hostwake_ind(bt_hc_low_power_event_t event)
{
}

static char *alloc(int size, char *file, int line)
{
	return H5_NEW_(size, file, line);
}

static int dealloc(TRANSAC transac, char *p_buf, char *file, int line)
{
	H5_FREE_((char *) transac, file, line);

	return BT_HC_STATUS_SUCCESS;
}

static int data_ind(TRANSAC transac, char *p_buf, int len)
{
	HC_BT_HDR *p_msg = (HC_BT_HDR *) transac;
	uint8_t *buf;

	if ((len + 1 - (int) sizeof(HC_BT_HDR)) < 0) {
		BRT_DBG("Error: data_ind length too small!!!!! len=%d. sizeof(HC_BT_HDR)=%d", len, sizeof(HC_BT_HDR));
		return 0;
	}

	buf = (uint8_t *) H5_NEW_(len + 1 - sizeof(HC_BT_HDR), __FILE__, __LINE__);

	if (buf) {
		switch (p_msg->event) {
		case MSG_HC_TO_STACK_HCI_ACL:
			*buf = HCI_TLPKT_ACLDATA;
			break;

		case MSG_HC_TO_STACK_HCI_EVT:
			*buf = HCI_TLPKT_EVENT;
			break;

		case MSG_HC_TO_STACK_HCI_SCO:
			*buf = HCI_TLPKT_SCODATA;
			break;

		default:
			BRT_DBG("ERROR: the hci header is wrong!! event:%d.", p_msg->event);
			break;
		}

		memcpy(buf + 1, p_msg + 1, len - sizeof(HC_BT_HDR));

		if (s_data_received_cb) {
			s_data_received_cb(buf, len + 1 - sizeof(HC_BT_HDR));
		}

		H5_FREE_(buf, __FILE__, __LINE__);
	}

	bt_hc_cbacks->dealloc(transac, p_buf, __FILE__, __LINE__);

	return BT_HC_STATUS_SUCCESS;
}

static int tx_result(TRANSAC transac, char *p_buf, \
                     bt_hc_transmit_result_t result)
{
	dealloc(transac, p_buf, __FILE__, __LINE__);

	return BT_HC_STATUS_SUCCESS;
}

bt_hc_callbacks_t hc_cbacks = {
	sizeof(bt_hc_callbacks_t),
	send_hci_data,
	preload_cb,
	postload_cb,
	lpm_cb,
	hostwake_ind,
	alloc,
	dealloc,
	data_ind,
	tx_result
};

void tl_h5_init(tl_h5_send_to_hc_cb send_cb, tl_h5_data_ind data_ind)
{
	bt_hc_cbacks = &hc_cbacks;
	s_data_received_cb = data_ind;
	tl_send_cb = send_cb;

	hci_h5_init();
}

void tl_h5_done()
{
	hci_h5_cleanup();
}

