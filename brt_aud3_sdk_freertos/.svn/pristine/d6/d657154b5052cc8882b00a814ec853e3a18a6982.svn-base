LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(CONFIG_FILE)

LOCAL_SRC_FILES := porting_utils.c \
		bt_snoop.c \
		portings.c \
		bt_debug.c \
		fileio.c

ifeq ($(CONFIG_PAN),y)
LOCAL_SRC_FILES += pan_inter.c
endif
ifeq ($(CONFIG_DUN),y)
LOCAL_SRC_FILES += bt_dhcp.c
LOCAL_SHARED_LIBRARIES := libnetutils
endif

include $(LOCAL_PATH)/tl/Android.mk

LOCAL_SRC_FILES += $(PORTINGS_FILES)
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/tl
LOCAL_C_INCLUDES += $(PORTINGS_INCLUDES:%=$(LOCAL_PATH)/%)

LOCAL_SHARED_LIBRARIES += libcutils 
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libbrt_vendor

#not set LOCAL_PRELINK_MODULE
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)
