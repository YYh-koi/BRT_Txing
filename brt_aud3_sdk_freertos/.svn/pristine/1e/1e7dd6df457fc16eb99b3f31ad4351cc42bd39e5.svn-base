#include <string.h>
#include "transport_impl.h"
#include "bcsptl.h"
#include "bcspui.h"

static BCSPClientCallbackStru s_bcsp_callbacks;
static transport_bridge_t *s_bridge = NULL;

static void bcsp_data_ind(unsigned char *data, unsigned long len)
{
	if (s_bridge && s_bridge->callbacks) {
		s_bridge->callbacks->on_data_received(data, len);
	}
}

static void bcsp_link_result(int result)
{
	int res;

	if (s_bridge == NULL || s_bridge->callbacks == NULL) {
		return;
	}

	if (result == LINK_RESULT_OK) {
		res = TRANSPORT_START_OK;
	} else {
		res = TRANSPORT_START_ERROR;
	}

	s_bridge->callbacks->on_started(res);
}

static void bcsp_lower_layer_send(unsigned char *data, unsigned long len)
{
	if (s_bridge && s_bridge->vendor_transport) {
		return s_bridge->vendor_transport->tl_send(data, len);
	}
}

static void transport_bcsp_init(transport_bridge_t *bridge)
{
	s_bridge = bridge;
	memset(&s_bcsp_callbacks, 0, sizeof(BCSPClientCallbackStru));
	s_bcsp_callbacks.Func_DataInd = bcsp_data_ind;
	s_bcsp_callbacks.Func_LinkResult = bcsp_link_result;
	s_bcsp_callbacks.Func_LowerSendCallback = bcsp_lower_layer_send;
	BcspStartup(/*FIXME:tl_hdl*/0, &s_bcsp_callbacks, /*FIXME:host_hdl*/0);
}

static void transport_bcsp_done()
{
}

static int transport_bcsp_send(UINT8 *buf, UINT32 len)
{
	BcspSend(/*tl_hdl FIXME*/ 0, buf, len);
	return 0;
}

static int transport_bcsp_process_recv_hci_data(UINT8 *data, UINT32 len)
{
	BcspRcvBytesFromUart(data, len);
	return 1;
}

void transport_bcsp_start()
{
	s_bridge->vendor_transport->tl_fw_config(NULL);
	InitBcspTL();
}

void transport_bcsp_stop()
{
	DoneBcspTL();
	// TODO on_stopped
}

transport_impl_t g_bcsp_tl_impl = {
	transport_bcsp_init,
	transport_bcsp_done,
	transport_bcsp_start,
	transport_bcsp_stop,
	transport_bcsp_send,
	transport_bcsp_process_recv_hci_data,
};

