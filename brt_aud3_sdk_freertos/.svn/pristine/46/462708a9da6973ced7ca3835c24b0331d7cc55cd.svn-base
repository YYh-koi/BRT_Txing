#include <string.h>
#include "sysdep.h"
#include "bt_hci_lib.h"
#include "transport_impl.h"
#include "brt_trace.h"
#include "brlinkd.h"

#define LOG(fmt, ...) bse_log_info("transport_h5", fmt, ## __VA_ARGS__)

#define HCI_TLPKT_COMMAND 1
#define HCI_TLPKT_ACLDATA 2
#define HCI_TLPKT_SCODATA 3
#define HCI_TLPKT_EVENT   4

#define BT_EVT_TO_LM_HCI_CMD        0x2000      /* HCI Command                      */
#define BT_EVT_TO_LM_HCI_ACL        0x2100      /* HCI ACL Data                     */
#define BT_EVT_TO_LM_HCI_SCO        0x2200      /* HCI SCO Data                     */
#define BT_EVT_TO_LM_DIAG           0x2c00      /* LM Diagnostics commands          */


static transport_bridge_t *s_bridge = NULL;

static void fw_config_callback(int result)
{
	int res;

	if (result == 0) {
		res = TRANSPORT_START_OK;
	} else {
		res = TRANSPORT_START_ERROR;
	}
	s_bridge->callbacks->on_started(res);
}

static int send_internal_command(unsigned char *buf, unsigned short len, internal_command_result_callback cb)
{
	uint16_t opcode = len;
	HC_BT_HDR *p_buf = (HC_BT_HDR *) buf;

	return hci_h5_send_int_cmd(opcode, p_buf, cb);
}

static transport_layer_callbacks_t tl_callbacks = {
	fw_config_callback,
	send_internal_command,
};

static void transport_h5_init(transport_bridge_t *bridge)
{
	s_bridge = bridge;

	if (s_bridge && s_bridge->vendor_transport) {
		s_bridge->vendor_transport->tl_init_ex(&tl_callbacks);
	}
}

static void transport_h5_done()
{
}

static void send_tl_data(UINT8 *buf, UINT32 len)
{
	if (s_bridge && s_bridge->vendor_transport) {
		s_bridge->vendor_transport->tl_send(buf, len);
	}
    return;
}

static void h5_data_ind(unsigned char *data, unsigned long len)
{
	if (s_bridge && s_bridge->callbacks) {
		s_bridge->callbacks->on_data_received(data, len);
	}
}

static int transport_h5_send(UINT8 *buf, UINT32 len)
{
	HC_BT_HDR *p;
	uint8_t *pp;
	uint16_t event = 0;

	switch (*buf) {
	case HCI_TLPKT_COMMAND:
		event = BT_EVT_TO_LM_HCI_CMD;
		break;
	case HCI_TLPKT_ACLDATA:
		event = BT_EVT_TO_LM_HCI_ACL;
		break;
	case HCI_TLPKT_SCODATA:
		event = BT_EVT_TO_LM_HCI_SCO;
		break;
	case HCI_TLPKT_EVENT:
		event = BT_EVT_TO_LM_DIAG; // TODO ??
		break;
	}

	// FIXME: LE data????
	p = NEW(sizeof(HC_BT_HDR) + len - 1);
	pp = (uint8_t *)(p + 1);

	p->len = len - 1;
	p->offset = 0;

	memcpy(pp, buf + 1, len - 1);
	p->event = event;

	hci_h5_send_msg(p);

	return 0;
}

static int transport_h5_process_recv_hci_data(UINT8 *data, UINT32 len)
{
	return hci_h5_receive_msg(data, len);
}

void transport_h5_start()
{
#ifdef BT_POWER_CONFIG
	static int h5_start_flag = 0;
	//h5_start_flag++;
	if(!h5_start_flag)
	{

		h5_start_flag = 1;
#endif
		tl_h5_init(send_tl_data, h5_data_ind);

		if (s_bridge && s_bridge->vendor_transport) {

			 if(hw_get_bt_module() == BRT_MODULE_CHIP_BRT) {
			 	LOG("begin power off....");
				s_bridge->vendor_transport->tl_power(1);
				LOG("begin power on....");
				usleep(20 * 1000);
				s_bridge->vendor_transport->tl_power(0);
				LOG("begin fw config....");
			} else { 
				LOG("begin power off....");
				s_bridge->vendor_transport->tl_power(0);
				LOG("begin power on....");
				usleep(20 * 1000);
				s_bridge->vendor_transport->tl_power(1);
				LOG("begin fw config....");
			}
			
			s_bridge->vendor_transport->tl_fw_config(NULL);
			LOG("after fw config....");
		}
#ifdef BT_POWER_CONFIG
	}
#endif
}

void transport_h5_stop()
{
#ifndef BT_POWER_CONFIG
	tl_h5_done();

	if (s_bridge && s_bridge->vendor_transport) {
		LOG("power off in h5_stop....");
		s_bridge->vendor_transport->tl_power(0);
	}
#endif
}

transport_impl_t g_h5_tl_impl = {
	transport_h5_init,
	transport_h5_done,
	transport_h5_start,
	transport_h5_stop,
	transport_h5_send,
	transport_h5_process_recv_hci_data,
};

