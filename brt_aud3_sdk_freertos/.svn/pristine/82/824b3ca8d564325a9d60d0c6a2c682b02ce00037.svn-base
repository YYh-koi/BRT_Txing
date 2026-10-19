#ifndef BSE_PORTING_H__
#define BSE_PORTING_H__
#include "btdatatype.h"
#include "task.h"

// define the debug level mask
#define BSE_DEBUG_LEVEL_API     0x00000001
#define BSE_DEBUG_LEVEL_INFO    0x00000002
#define BSE_DEBUG_LEVEL_WARNING 0x00000004
#define BSE_DEBUG_LEVEL_ERROR   0x00000008
#define BSE_DEBUG_LEVEL_VERBOSE 0x00000010

#define BSE_DEBUG_LEVEL_BTSNOOP 0x00010000
#define BSE_DEBUG_LEVEL_STACK   0x00020000
#define BSE_DEBUG_LEVEL_HCI     0x00040000

#define BSE_DEBUG_BTDEBUG_LOG_BACK   0x00100000
#define BSE_DEBUG_BTSNOOP_A2DP_AUDIO 0x00200000
#define BSE_DEBUG_BTSNOOP_SCO_AUDIO  0x00400000
#define BSE_DEBUG_A2DP_SCO_WAV       0x00800000

#define BSE_DEBUG_LEVEL_OUTPUTFILE 0x10000000

#define CONFIGURATION_TYPE_READONLY		0x00
#define CONFIGURATION_TYPE_READWRITE	0x01
#define CONFIGURATION_TYPE_BINARY		0x02

typedef void (*tl_data_ind)(unsigned char *data, unsigned int len);
typedef void (*porting_thread_func)();

/**
  * optional type/struct definition for extending the functionary of transport layer.
  */
typedef void (*internal_command_result_callback)(void *param);

typedef struct tag_transport_layer_callbacks {
	/**
	  * Notify upper layer that the firmware configuration procedure has finished.
	  * @param result The result of firmware-configuration. 0 for success. other for failed.
	  */
	void (*fw_config_callback)(int result);

	/**
	  * Callback provided by upper layer. It is called when vendor want to send internal HCI command.
	  * @param buf The buffer of the command
	  * @param len The length of command in bytes.
	  * @param cb Callback which will be called when the command has sent
	  *
	  * @return 1(true) if send successfully. 0(false) otherwise.
	  */
	int (*send_internal_command)(unsigned char *buf, unsigned short len, internal_command_result_callback cb);
} transport_layer_callbacks_t;


typedef struct tag_transport_layer_stru {
	void (*tl_init)(tl_data_ind cb);
	void (*tl_send)(unsigned char *data, unsigned short len);
	void (*tl_done)(void);
	/**
	  * [optional]
	  */
	void (*tl_init_ex)(transport_layer_callbacks_t *cbs);
	/**
	  * power on/off the BT chip
	  * [optional]
	  */
	void (*tl_power)(unsigned char on);
	/**
	  * config/download the firmware into the BT chip.
	  * [optional]
	  */
	void (*tl_fw_config)(void *param);
} transport_layer_t;

typedef struct tag_porting_stru {
	/* callbacks of transport layer. SHOULD be filled. */
	transport_layer_t *transport;

	/*porting function to get elapsed time of the platform machine (milliseconds) since boot. SHOULD be filled*/
	unsigned long (*porting_get_current_time)();

	/* All below callbacks is optional. It software runs in multiple-task system,
	   it is recommanded to implement all of them. */
	/* porting functions of protection of critical area. */
	SYSTEM_LOCK (*porting_create_critical)(const char *name);
	void (*porting_enter_critical)(SYSTEM_LOCK lock);
	void (*porting_leave_critical)(SYSTEM_LOCK lock);
	void (*porting_free_critical)(SYSTEM_LOCK lock);

	/*porting functions of wait-signal model*/
	void* (*porting_create_event)(const char *name);
	void (*porting_wait_event)(void *event);
	void (*porting_wait_timeout_event)(void *event, unsigned int milliseconds);
	void (*porting_signal_event)(void *event);
	void (*porting_delete_event)(void *event);

	/* thread-related porting functions */
	void *(*porting_create_thread)(porting_thread_func func);
	void (*porting_join_thread)(void *pthread_hdl);

	void (*porting_print)(const char *tag, const int level, const char *newline_break, const char *content);
	void (*porting_hci_log_init)();
	void (*porting_hci_log_destroy)();
	void (*porting_hci_log_capture)(unsigned char direction, unsigned int length, unsigned char *data);
	int (*porting_get_debug_level)();
	/*porting functions to read/write configuration which is commonly stored in non-volatile memory. */
	int (*porting_read_configure)(unsigned char flag, unsigned short offset, int len, unsigned char *data);
	int (*porting_write_configure)(unsigned char flag, unsigned short offset, int len, unsigned char *data);
	int (*porting_get_local_property)(const char* sec, const char *key, char *value, int *value_len, char* file_path);
	int (*porting_set_local_property)(const char *key, char *value);
	int (*porting_write_data_to_cp)(unsigned int register_addr, unsigned char *p_buff, unsigned int length);
	int (*porting_read_data_from_cp)(unsigned int register_addr, unsigned char *p_buff, unsigned int length);
	void (*porting_sleep)(unsigned long milli_sec);
	void (*porting_set_debug_level)(int level);
	/* porting function to get the Bluetooth config path.
	 * type: 0 -- readonly path. for TL.INI 
	 * type: 1 -- read/write path. for saving paired device list and so on. 
	 * type: 2 -- binary file path. such as firmware file. */
	const char * (*porting_get_configure_path)(unsigned char type);
	void (*porting_set_configure_path)(unsigned char type, const char *path);
} porting_functions_t;

void porting_init(porting_functions_t *cb);
int porting_get_debug_level();
porting_functions_t *get_porting_functions();
#endif

