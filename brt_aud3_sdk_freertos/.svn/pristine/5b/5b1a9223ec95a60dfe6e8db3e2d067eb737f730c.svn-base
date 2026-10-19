LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(CONFIG_FILE)

LOCAL_SRC_FILES := \
		at_command.c \
		brlinkd.c

ifeq ($(CONFIG_OS_ANDROID),y)
AUDIO_UTILS_DIR := audio_utils
ifeq ($(CONFIG_SCO_OVER_UART),y)
    LOCAL_SRC_FILES += audio_sco_hw.c
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/../$(AUDIO_UTILS_DIR)/include
    LOCAL_WHOLE_STATIC_LIBRARIES := libbse_audio_io libbse_audio_utils libbse_resampler
	LOCAL_STATIC_LIBRARIES += libbse_audio_io 
endif
endif
ifeq ($(CONFIG_SCO_OVER_PCM),y)
LOCAL_SRC_FILES += \
			audio_sco_pcm_hw.c \
			pcm_player.c \
			pcm_record.c \
			pcm/pcm.c
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/linux/audio_utils/include \
		$(BRLINK_TOP_DIR)/linux/sink_hw/include \
		$(BRLINK_TOP_DIR)/stack/include \
		$(LOCAL_PATH)/pcm/include
LOCAL_WHOLE_STATIC_LIBRARIES := libbse_audio_io libbse_audio_utils libbse_resampler
LOCAL_STATIC_LIBRARIES += libbse_audio_io
#LOCAL_SHARED_LIBRARIES += libtinyalsa
endif #CONFIG_SCO_OVER_PCM

ifeq ($(CONFIG_OS_LINUX),y)
ifeq ($(CONFIG_SCO_OVER_UART),y)
LOCAL_SRC_FILES += audio_sco_linux.c wav_file.c
LDLIBS += -lasound
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/linux/audio_utils/include \
		$(BRLINK_TOP_DIR)/linux/sink_hw/include \
		$(BRLINK_TOP_DIR)/stack/include \
		$(LOCAL_PATH)/pcm/include
endif
endif

ifeq ($(CONFIG_BRT_AEC_PROCESSING),y)
LOCAL_C_INCLUDES += $(BRLINK_TOP_DIR)/linux/preprocessing_vcp8/
LOCAL_SHARED_LIBRARIES += libbrt_aec
endif

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := brlinkd

LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/../sink_hw/ \
		$(BRLINK_TOP_DIR)/sdk/auto_at/ \
		$(BRLINK_TOP_DIR)/sdk/auto_app \
		$(BRLINK_TOP_DIR)/sdk/auto_app/audio_hw \
		$(BRLINK_TOP_DIR)/sdk/auto_app/audio_hw/pcm/include \
		$(BRLINK_TOP_DIR)/linux/portings \
		$(BRLINK_TOP_DIR)/include \

LOCAL_CFLAGS := -DCONFIG_DUMP_CORE

ifeq ($(SERVER_PSEUDO_TTY),y)
LOCAL_CFLAGS += -DSERVER_PSEUDO_TTY
ifeq ($(USER_DEFINED_PSEUDO_TTY ),y)
LOCAL_CFLAGS += -DUSER_DEFINED_PSEUDO_TTY
endif
LOCAL_SRC_FILES += bt_pty.c
endif

LOCAL_CFLAGS += -DAT_EVENT_NULL_TERMINATED
LOCAL_SHARED_LIBRARIES += libbt_platform
#LOCAL_SHARED_LIBRARIES = libbt_platform
ifeq ($(CONFIG_OS_ANDROID),y)
LOCAL_SHARED_LIBRARIES += libbrlink libcutils libhardware libmedia liblog libutils
endif
ifeq ($(CONFIG_SCO_OVER_UART),y)
#SIMPLE_AEC_BY_VAD := y
ifeq ($(SIMPLE_AEC_BY_VAD),y)
LOCAL_SRC_FILES += vad.c
LOCAL_CFLAGS += -DSIMPLE_AEC_BY_VAD
endif
endif

ifeq ($(CONFIG_OS_LINUX),y)
ifneq ($(CONFIG_OS_ANDROID),y)
LOCAL_SRC_FILES += hw_linux.c
endif #CONFIG_OS_ANDROID
endif #CONFIG_OS_LINUX


ifeq ($(CONFIG_A2DP_SNK),y)
LOCAL_SRC_FILES += audio_sink_device.c 

ifeq ($(CONFIG_AUDIO_SINK_DEVICE_BY_HAL),y)
LOCAL_CFLAGS += -DAUDIO_SINK_DEVICE_BY_HAL
else
LOCAL_WHOLE_STATIC_LIBRARIES := libbse_audio_io libbse_audio_utils libbse_resampler
LOCAL_STATIC_LIBRARIES += a2dp_sink.default
	ifeq ($(CONFIG_OS_LINUX),y)
		ifneq ($(CONFIG_OS_ANDROID),y)
			ifeq ($(CONFIG_AUDIO_HW_ALSA),y)
				LOCAL_SHARED_LIBRARIES += asound
			endif
		endif #CONFIG_OS_ANDROID
	endif #CONFIG_OS_LINUX
endif #CONFIG_AUDIO_SINK_DEVICE_BY_HAL

endif #CONFIG_A2DP_SNK

#not set LOCAL_PRELINK_MODULE

ifeq ($(CONFIG_WRITE_AUDIO_IN_PERIODIC_PTHREAD),y)
LOCAL_CFLAGS += -DWRITE_AUDIO_IN_PERIODIC_PTHREAD
LOCAL_SRC_FILES += bse_audio_queue.c bse_timer.c
endif

include $(BUILD_EXECUTABLE)
