CUR_DIR := ../auto_at
AT_FILES := ./auto_at.c \
    ./evt_parse.c \
    ./cmd_process.c \
	./cmd_process_ex.c \
	./cmd_composition.c \
	./cmd_operations.c

ifeq ($(CONFIG_GATT),y)
AT_FILES += ./evt_parse_gatt.c
AT_FILES += ./evt_parse_gatt_DTTMode.c
AT_FILES += ./cmd_process_gatt.c
AT_FILES += ./cmd_process_gatt_client.c
AT_FILES += ./cmd_process_gatt_server.c
AT_FILES += ./cmd_process_gatt_DTTMode.c
endif

LOCAL_SRC_FILES += $(AT_FILES:%=$(CUR_DIR)/%)
