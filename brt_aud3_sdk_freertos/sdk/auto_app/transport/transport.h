#ifndef BT_TRANSPORT_H__
#define BT_TRANSPORT_H__

#include "btdatatype.h"

enum {
    TRANSPORT_START_OK,
    TRANSPORT_START_ERROR,
};

enum {
    TRANSPORT_STOP_OK,
    TRANSPORT_STOP_ERROR,
};

enum {
    TRANSPORT_RESET_OK,
    TRANSPORT_RESET_ERROR,
};

typedef int (*transport_looper_callback)();

typedef struct tag_transport_callbacks {
	void (*on_started)(int result);
	void (*on_stopped)(int result);
	void (*on_restarted)(int result);
	void (*on_data_received)(UINT8 *buf, UINT32 len);
	void (*on_error_occurred)(int err_code);
} transport_callbacks_t;

typedef struct tag_transport {
	void (*init)(struct tag_transport *self, transport_callbacks_t *callback);
	void (*done)(struct tag_transport *self);
	void (*start)(struct tag_transport *self);
	void (*stop)(struct tag_transport *self);
	void (*restart)(struct tag_transport *self);
	int (*send)(struct tag_transport *self, UINT8 *buf, UINT32 len);
	/**
	  * return the executer which will run in the upper layer's main_loop repeatedly.
	  * For embedded system, UART port uses interrupt to deliver the UART data, so this transport layer
	  * should periodly read the data from the buffer.
	  * <b>Note:</b> It may return NULL on systems with OS(No need to buffer the UART data).
	  */
	transport_looper_callback (*get_looper_func)(struct tag_transport *self);
} transport_t;

transport_t *transport_new();
void transport_free(transport_t *obj);

#endif /*BT_TRANSPORT_H__*/

