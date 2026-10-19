#ifndef TRANSPORT_IMPL_H__
#define TRANSPORT_IMPL_H__
#include "transport.h"
#include "porting.h"

typedef struct tag_transport_bridge {
	transport_callbacks_t *callbacks;
	transport_layer_t  *vendor_transport;
} transport_bridge_t;

typedef struct tag_transport_impl {
	void (*init)(transport_bridge_t *host);
	void (*done)();
	void (*start)();
	void (*stop)();
	int  (*send)(UINT8 *buf, UINT32 len);
	int  (*process_recv_hci_data)(UINT8 *data, UINT32 len);
} transport_impl_t;

#endif /*TRANSPORT_IMPL_H__*/
