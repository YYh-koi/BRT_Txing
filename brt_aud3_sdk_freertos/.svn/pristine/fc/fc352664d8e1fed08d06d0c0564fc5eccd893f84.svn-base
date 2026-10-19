#include <string.h>
#include "transport_impl.h"
#include "hci_parser.h"
#include "brt_trace.h"

#define LOG(fmt, ...) bse_log_info("transport_h4", fmt, ## __VA_ARGS__)

static transport_bridge_t *s_bridge = NULL;
static hci_parser_t *parser = NULL;

static void fw_config_callback(int result)
{
	int res;

	if (result == 0) {
		res = TRANSPORT_START_OK;
	} else {
		res = TRANSPORT_START_ERROR;
	}
	LOG("fw_config_callback, res = %d", res);
	s_bridge->callbacks->on_started(res);
}

static void send_internal_command(unsigned char *buf, unsigned short len, internal_command_result_callback cb)
{
	// Nothing to do
}

static transport_layer_callbacks_t tl_callbacks = {
	fw_config_callback,
	send_internal_command,
};

static void transport_h4_init(transport_bridge_t *bridge)
{
	s_bridge = bridge;
	parser = hci_parser_new();
	LOG("transport_h4_init, parser = %08x", parser);
	if (parser && s_bridge) {
		parser->init(parser, s_bridge->callbacks->on_data_received);
		s_bridge->vendor_transport->tl_init_ex(&tl_callbacks);
	}
}

static void transport_h4_done()
{
	LOG("transport_h4_done, parser = %08x", parser);
	if (parser) {
		parser->done(parser);
		hci_parser_free(parser);
		parser = NULL;
	}
}

// FIXME: extract the common code
static int transport_h4_send(UINT8 *buf, UINT32 len)
{
	if (s_bridge && s_bridge->vendor_transport) {
		s_bridge->vendor_transport->tl_send(buf, len);
	}

	return 0;
}

static int transport_h4_process_recv_hci_data(UINT8 *data, UINT32 len)
{
    if (parser) {
		return parser->parse(parser, data, len);
	}

	return 1;
}

void transport_h4_start()
{
	if(s_bridge) {
		s_bridge->vendor_transport->tl_fw_config(NULL);
	}
}

void transport_h4_stop()
{
	if(s_bridge) {
		s_bridge->vendor_transport->tl_power(0);
	}
}

transport_impl_t g_h4_tl_impl = {
	transport_h4_init,
	transport_h4_done,
	transport_h4_start,
	transport_h4_stop,
	transport_h4_send,
	transport_h4_process_recv_hci_data,
};

