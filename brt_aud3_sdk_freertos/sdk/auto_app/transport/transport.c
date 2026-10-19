#include "porting.h"
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "brt_trace.h"
#include "transport.h"
#include "transport_impl.h"
#include "brlinkd.h"

#ifdef CONFIG_BT_TL_LOCAL_BUFFER
#include "tl_buffer.h"
#endif

#define LOG(fmt, ...) bse_log_info("transport", fmt, ## __VA_ARGS__)

typedef struct tag_transport_adapter {
	transport_t        transport;
	transport_bridge_t *bridge;
	transport_impl_t   *impl;
	void (*constructor)(struct tag_transport_adapter *self);
	void (*destructor)(struct tag_transport_adapter *self);
} transport_adapter_t;

#define CAST_ADAPTER_TO_TRANSPORT(adapter)  ((transport_t *) adapter)
#define CAST_TRANSPORT_TO_ADAPTER(transport)  ((transport_adapter_t *) transport)

// FIXME
static transport_adapter_t *s_transport = NULL;

static int process_vendor_hci_data(unsigned char *data, unsigned short len)
{
	if (s_transport && s_transport->impl && s_transport->impl->process_recv_hci_data) {
		s_transport->impl->process_recv_hci_data((UINT8 *) data, (UINT32) len);
	}

	return 1; /* TODO ? */
}

#ifdef CONFIG_BT_TL_LOCAL_BUFFER
static void init()
{
	tl_buffer_init();
}

static int tl_looper_execute(void)
{
	UINT32 read;
	UINT8 data[128];

	while (1) {
		memset(data, 0, sizeof(data));
		read = tl_buffer_read(data, sizeof(data));

		if (read == 0) {
			return 0;
		}

		process_vendor_hci_data(data, read);
	}

	return 1;
}

static void vendor_transport_on_data_ind(unsigned char *data, unsigned short len)
{
	tl_buffer_write(data, len);
}

#else
static void init()
{
}

static int tl_looper_execute(void)
{
	return 0;
}

static void vendor_transport_on_data_ind(unsigned char *data, unsigned short len)
{
	process_vendor_hci_data(data, len);
}
#endif

static void transport_init(transport_t *transport, transport_callbacks_t *callback)
{
	transport_adapter_t *adapter = CAST_TRANSPORT_TO_ADAPTER(transport);
	transport_bridge_t *bridge;

	if (adapter->bridge == NULL) {
		return;
	}

	bridge = adapter->bridge;
	adapter->bridge->callbacks = callback;

	if (!adapter->bridge->callbacks) {
		LOG("Fatal Error: callbacks is NULL.");
		return;
	}

	if (!bridge->vendor_transport) {
		LOG("Fatal Error: Can NOT get the OS-dependent porting functions.");
		return;
	}

	/* FIXME vendor_transport's init should be called in impl.*/
	if (bridge->vendor_transport->tl_init != NULL) {
	bridge->vendor_transport->tl_init(vendor_transport_on_data_ind);
	}

	init();

	if (adapter->impl) {
		adapter->impl->init(adapter->bridge);
	}
}

static void transport_done(transport_t *transport)
{
	transport_adapter_t *adapter = CAST_TRANSPORT_TO_ADAPTER(transport);
	transport_bridge_t *bridge;

	if (adapter == NULL) {
		return;
	}

	if (adapter->impl) {
		adapter->impl->done();
	}

	bridge = adapter->bridge;

	if (bridge && bridge->vendor_transport && bridge->vendor_transport->tl_done) {
		bridge->vendor_transport->tl_done();
	}
}

static void transport_start(transport_t *transport)
{
	transport_adapter_t *adapter = CAST_TRANSPORT_TO_ADAPTER(transport);

	if (adapter && adapter->impl) {
		adapter->impl->start();
	}
}

static void transport_stop(transport_t *transport)
{
	transport_adapter_t *adapter = CAST_TRANSPORT_TO_ADAPTER(transport);

	if (adapter && adapter->impl) {
		adapter->impl->stop();
	}
}

static int transport_send(transport_t *transport, UINT8 *buf, UINT32 len)
{
	transport_adapter_t *adapter = CAST_TRANSPORT_TO_ADAPTER(transport);

	if (adapter && adapter->impl) {
		return adapter->impl->send(buf, len);
	}

	return 0;
}

static void transport_restart(transport_t *self)
{
}

static transport_looper_callback transport_get_looper_func(transport_t *self)
{
	return tl_looper_execute;
}

static void transport_destructor(transport_adapter_t *adapter)
{
	if (adapter && adapter->bridge) {
		FREE(adapter->bridge);
	}
}

static int transport_constructor(transport_adapter_t *adapter)
{
	transport_t *transport = CAST_ADAPTER_TO_TRANSPORT(adapter);
	transport_bridge_t *bridge;

	transport->init            = transport_init;
	transport->done            = transport_done;
	transport->start           = transport_start;
	transport->stop            = transport_stop;
	transport->restart         = transport_restart;
	transport->send            = transport_send;
	transport->get_looper_func = transport_get_looper_func;

	bridge = (transport_bridge_t *) NEW(sizeof(transport_bridge_t));
	bridge->vendor_transport = get_porting_transport_layer(); /*from sysdep */

	adapter->bridge           = bridge;
	adapter->constructor      = transport_constructor;
	adapter->destructor       = transport_destructor;
#ifdef CONFIG_TL_BCSP
	extern transport_impl_t g_bcsp_tl_impl;
	adapter->impl             = &g_bcsp_tl_impl;
#else
    if((hw_get_bt_module() == BRT_MODULE_CHIP_RTL) || (hw_get_bt_module() == BRT_MODULE_CHIP_BRT)) {
        extern transport_impl_t g_h5_tl_impl;
        adapter->impl             = &g_h5_tl_impl;
    } else {
        extern transport_impl_t g_h4_tl_impl;
        adapter->impl             = &g_h4_tl_impl;
    }
#endif

	return 1;
}

transport_t *transport_new()
{
	transport_adapter_t *adapter = (transport_adapter_t *) NEW(sizeof(transport_adapter_t));

	if (adapter == NULL) {
		return NULL;//adapter;
	}

	if (!transport_constructor(adapter)) {
		transport_destructor(adapter);
		FREE(adapter);
		return 0;
	}

	s_transport = adapter;

	return CAST_ADAPTER_TO_TRANSPORT(adapter);
}

void transport_free(transport_t *transport)
{
	transport_adapter_t *adapter = CAST_TRANSPORT_TO_ADAPTER(transport);

	if (adapter) {
		transport_destructor(adapter);
		FREE(adapter);
	}

	s_transport = NULL;
}

