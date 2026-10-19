LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(CONFIG_FILE)

MY_LIST :=
LOCAL_SRC_FILES := bt_functions.c  \
		auto_gap.c \
		internal_gap.c \
		get_service_list_async.c\
		patch_reconn.c \
		ring_buffer.c \
		profile.c \
		bt_manager.c \
		bt_power_manager.c \
		internal_manager.c \
		auto_commfunc.c \
		bt_utils.c  \
		trace.c    

ifeq ($(CONFIG_BT_CHIP_CSR8350),y)
LOCAL_SRC_FILES += auto_stream.c internal_stream.c internal_csr_dsp_8350.c
endif
ifeq ($(CONFIG_BT_CHIP_CSR),y)
LOCAL_SRC_FILES += vendor/csr_pskey.c \
				   vendor/ring_tone_fsm.c \
					vendor/vendor_csr.c 
else
ifeq ($(CONFIG_BT_CHIP_ISSC),y)
LOCAL_SRC_FILES += vendor/vendor_issc.c
else
ifeq ($(CONFIG_BT_CHIP_BCM),y)
LOCAL_SRC_FILES += vendor/vendor_bcm.c
else
LOCAL_SRC_FILES += vendor/vendor_dummy.c
endif
endif
endif

ifeq ($(CONFIG_A2DP),y)
LOCAL_SRC_FILES += auto_a2dp.c internal_a2dp.c parse_a2dp_data.c
endif

ifeq ($(CONFIG_HFP),y)
LOCAL_SRC_FILES += auto_hf.c internal_hf.c
endif

ifeq ($(CONFIG_AVRCP),y)
LOCAL_SRC_FILES += internal_avrcp.c auto_avrcp.c
endif

ifeq ($(CONFIG_PBAP),y)
LOCAL_SRC_FILES += internal_pbap.c auto_pbap.c
LOCAL_SRC_FILES += vcf_parse.c
LOCAL_SRC_FILES += photo_parser.c base64.c
else 
ifeq ($(CONFIG_MAP),y)
LOCAL_SRC_FILES += vcf_parse.c
endif
endif

ifeq ($(CONFIG_MAP),y)
LOCAL_SRC_FILES += auto_map.c internal_map.c internal_bmessage.c 
endif

ifeq ($(CONFIG_HID),y)
LOCAL_SRC_FILES += auto_hid.c internal_hid.c auto_cb_screen.c internal_cb_screen.c  
endif

ifeq ($(CONFIG_IAP2),y)
LOCAL_SRC_FILES += auto_iap.c internal_iap.c
endif

ifeq ($(CONFIG_SPP),y)
LOCAL_SRC_FILES += auto_spp.c
ifeq ($(CONFIG_SPP_ANDROID), y)
MY_LIST += $(wildcard $(LOCAL_PATH)/rfcomm/*.c)
else
MY_LIST += $(LOCAL_PATH)/rfcomm/auto_rfcomm.c \
		   $(LOCAL_PATH)/rfcomm/auto_rfcomm_sdp.c 
endif
endif

ifeq ($(CONFIG_DUN),y)
MY_LIST += $(wildcard $(LOCAL_PATH)/dun/*.c)
endif

ifeq ($(CONFIG_FTP),y)
LOCAL_SRC_FILES += auto_ftp.c internal_ftp.c
endif

ifeq ($(CONFIG_OPP),y)
LOCAL_SRC_FILES += auto_opp.c
endif

ifeq ($(CONFIG_CHECK_LICENSE),y)
LOCAL_SRC_FILES += auto_license.c 
ifeq ($(LICENSE_BY_NETWORK),y)
LOCAL_STATIC_LIBRARIES := libbse_license \
	libBsPA_Sdk \
	libBsPA_Packet \
	libBsPA_Rsa \
	libBsPA_CommFunc \
	libBsPA_HWValue \
	libBsPA_MD5 \
	libchklic 
endif
ifeq ($(LICENSE_BY_CHIP),y)
LOCAL_SRC_FILES += internal_security.c
endif
endif

ifeq ($(CONFIG_PAN),y)
LOCAL_SRC_FILES += auto_pan.c internal_pan.c
endif

ifeq ($(CONFIG_ANCS_CLIENT),y)
LOCAL_SRC_FILES += auto_ancs.c
endif

ifeq ($(CONFIG_GATT),y)
MY_LIST += $(wildcard $(LOCAL_PATH)/gatt/*.c)
endif

# if not support A2DP PCM output, commit below
MY_LIST += $(wildcard $(LOCAL_PATH)/audio_hw/*.c)

# if  support A2DP PCM output, commit below
#MY_LIST += $(wildcard $(LOCAL_PATH)/audio_hw/pcm/*.c)

ifeq ($(CONFIG_TL_BCSP),y)
MY_LIST += $(wildcard $(LOCAL_PATH)/bcsp/*.c)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/bcsp
else
ifeq ($(CONFIG_TL_H5),y)
MY_LIST += $(wildcard $(LOCAL_PATH)/h5/*.c)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/h5/include
endif
endif

include $(LOCAL_PATH)/transport/Android.mk

LOCAL_SRC_FILES += $(MY_LIST:$(LOCAL_PATH)/%=%)

include $(LOCAL_PATH)/../auto_at/Android.mk

LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/include \
					$(BRLINK_TOP_DIR)/stack/include  \
					$(BRLINK_TOP_DIR)/stack/xmlparse \
					$(PLATFORM_DEP_PATH) \
					$(LOCAL_PATH)/../auto_at \
					$(LOCAL_PATH)/transport \
					$(LOCAL_PATH)/audio_hw \
					$(LOCAL_PATH)/gatt \
					$(LOCAL_PATH)/dun \
					$(LOCAL_PATH)/rfcomm \
					$(LOCAL_PATH)/vendor

LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/stack/pan

ifeq ($(CONFIG_ANCS_CLIENT),y)
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/stack/ancs
else
LOCAL_SRC_FILES := $(filter-out auto_ancs.c, $(LOCAL_SRC_FILES))
endif

LICENSE_BY_NETWORK := n
ifeq ($(LICENSE_BY_NETWORK),y)
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/active/ProductActivation/BsPA_Sdk/ \
	$(BRLINK_TOP_DIR)/active/ProductActivation/BsPA_CommFunc/ \
	$(BRLINK_TOP_DIR)/active/ProductActivation/BsPA_packet/ \
	$(BRLINK_TOP_DIR)/active/ProductActivation/BsPA_Rsa/ \
	$(BRLINK_TOP_DIR)/active/ProductActivation/BsPA_MD5/ \
	$(BRLINK_TOP_DIR)/active/ProductActivation/BsPA_HWValue/ \
	$(BRLINK_TOP_DIR)/active/ProductActivation/curl/ 
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/active/chk_lic

LOCAL_STATIC_LIBRARIES += libBsPA_Sdk libBsPA_Packet libBsPA_CommFunc libBsPA_Rsa libBsPA_MD5 libBsPA_HWValue libchklic
LOCAL_CFLAGS += -DLICENSE_BY_NETWORK
endif
#LOCAL_CFLAGS += -DLICENSE_BY_CHIP

#LOCAL_CFLAGS += -DPATCH_RECONN
LOCAL_ALLOW_UNDEFINED_SYMBOLS:=true
LOCAL_SHARED_LIBRARIES += libcutils libbrlink libdl
LOCAL_STATIC_LIBRARIES += libbrt_vendor
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libbt_platform
#not set LOCAL_PRELINK_MODULE
include $(BUILD_SHARED_LIBRARY)
